#!/usr/bin/env python3
"""Source-level regression tests for LGT8F328P Unlocked.

These tests deliberately require only Python so they can run even when an AVR
compiler is unavailable. They protect the invariants behind the P0/P1 fixes;
they do not replace AVR compilation or hardware verification.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def text(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8", errors="replace")


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise AssertionError(msg)


def section(src: str, start: str, end: str | None = None) -> str:
    i = src.find(start)
    require(i >= 0, f"missing anchor: {start}")
    if end is None:
        return src[i:]
    j = src.find(end, i + len(start))
    require(j >= 0, f"missing end anchor: {end}")
    return src[i:j]


def continuous_to_real(address: int, page_user_bytes: int = 1020) -> int:
    if address >= page_user_bytes:
        return 1024 * (address // page_user_bytes) + address % page_user_bytes
    return address


def test_eeprom_mapping_and_native_words() -> None:
    h = text("libraries/E2PROM/EEPROM.h")
    cpp = text("libraries/E2PROM/EEPROM.cpp")
    require("#define lgt_eeprom_free_space_per_1KB_page() ((uint16_t)1020)" in h,
            "EEPROM must conservatively hide the final 32-bit flash cell")
    expected = {0: 0, 1019: 1019, 1020: 1024, 2039: 2043, 2040: 2048}
    for logical, physical in expected.items():
        require(continuous_to_real(logical) == physical,
                f"logical EEPROM mapping {logical}->{physical} failed")
    valid = section(cpp, "static bool lgt_eeprom_valid_word_range", "uint32_t lgt_eeprom_read32")
    require("(address & 0x03u) != 0" in valid, "native 32-bit EEPROM access must require word alignment")
    require("bytes <= (uint32_t)(size - address)" in valid, "native word range must be overflow-safe")
    swm = section(cpp, "void lgt_eeprom_writeSWM", "void lgt_eeprom_readSWM")
    require("if (length == 0 || pData == 0) return;" in swm, "SWM write must reject zero/null")
    require("wordReal = (uint16_t)(real + (uint16_t)(i * 4u))" in swm,
            "SWM write must explicitly advance EEAR instead of assuming auto-increment")
    require("wordsHere = (pageBytes - inPage) / 4u" in swm,
            "SWM must chunk on emulation-page boundaries")
    # With the deliberately exposed 1020 bytes/page, all native word chunks are
    # word aligned and never touch physical offsets 1020..1023.
    for logical in range(0, 4 * 1020, 4):
        real = continuous_to_real(logical)
        require(real % 4 == 0, f"native word physical alignment lost at logical {logical}")
        require((real % 1024) <= 1016, f"native word enters reserved flash cell at {real}")


def test_eeprom_controller_state() -> None:
    h = text("libraries/E2PROM/EEPROM.h")
    cpp = text("libraries/E2PROM/EEPROM.cpp")
    for name in ("lgt_eeprom_SWM_ON", "lgt_eeprom_SWM_OFF", "lgt_eeprom_reset"):
        require(name in h, f"missing {name}")
    require("uint8_t _lgt_eccr = (uint8_t)(ECCR & 0x7Fu)" in h,
            "EEPROM protected writes must preserve partition/config bits")
    require("lgt_eeprom_init( number_of_1KB_pages );" in h,
            "EEPROM change_size must call lgt_eeprom_init")
    require("lgt_eeprom_write_eccr" in cpp and "ECCR = (uint8_t)(value & 0x7Fu);" in cpp,
            "ECCR helper must clear write-enable bit in final protected write")


def test_wire_guards() -> None:
    cpp = text("libraries/Wire/Wire.cpp")
    twi = text("libraries/Wire/utility/twi.c")
    req = section(twi, "uint8_t twi_readFrom", "uint8_t twi_writeTo")
    require("if (length == 0) return 0;" in req, "low-level TWI zero-length guard missing")
    require("if (data == 0) return 0;" in req, "low-level TWI null guard missing")
    require(req.find("if (length == 0) return 0;") < req.find("twi_masterBufferLength = length-1"),
            "zero-length guard must precede length-1")
    isr = section(twi, "case TW_MR_DATA_ACK", "case TW_MR_SLA_NACK")
    require("twi_masterBufferIndex >= TWI_BUFFER_LENGTH" in isr,
            "master receive ISR capacity guard missing")
    require("twi_masterBufferIndex < TWI_BUFFER_LENGTH" in isr,
            "final-byte receive capacity guard missing")
    require("if ((length != 0) && (data == 0)) return 1;" in twi,
            "TWI transmit/write null guard missing")
    require("void TwoWire::flush(void)" in cpp and "twi_flush();" in cpp,
            "Wire.flush must be implemented")
    require("quantity <= 0" in cpp and "address < 0 || address > 0x7F" in cpp,
            "int requestFrom overload must reject invalid quantities/addresses")
    require(("if (quantity > 0 && data == 0)" in cpp) or ("if (quantity != 0 && data == 0)" in cpp),
            "Wire.write(buffer) null guard missing")
    require("txAddress = (address <= 0x7Fu) ? address : 0x80u" in cpp and "if (txAddress > 0x7Fu)" in cpp,
            "beginTransmission must fail closed for invalid 7-bit addresses")
    require("size_t accepted = 0" in cpp and "return accepted;" in cpp,
            "Wire.write(buffer) must return the actual accepted byte count")
    require("if (address > 0x7Fu) return 0;" in twi and "if (address > 0x7Fu) return 4;" in twi,
            "low-level TWI master operations must reject invalid 7-bit addresses")


def _extract_qfp48_timer_entries(src: str) -> list[str]:
    arr = section(src, "const uint8_t PROGMEM digital_pin_to_timer_PGM[]", "#if defined(ARDUINO_MAIN)")
    q = section(arr, "#if defined(__LGT8FX8P48__)", "#else")
    entries = []
    for line in q.splitlines()[1:]:
        m = re.match(r"\s*([A-Z0-9_]+)\s*,?\s*/\*\s*(\d+)", line)
        if m:
            entries.append(f"{m.group(2)}:{m.group(1)}")
    return entries


def test_qfp48_tables_and_analog_map() -> None:
    p = text("variants/standard/pins_arduino.h")
    q = text("variants/lgt8fx8p48/pins_arduino.h")
    require("#define NUM_PIN_TABLE_ENTRIES        40" in p, "QFP48 lookup bound must be 40")
    require("#define\tNUM_DIGITAL_PINS\t40" in q or "#define NUM_DIGITAL_PINS\t40" in q,
            "QFP48 must expose 40 digital pins")
    require("(((p) < 8) ? (p) + 14 : (((p) < 12) ? (p) + 15 : -1))" in p,
            "QFP48 analog A8..A11 gap mapping missing")
    entries = _extract_qfp48_timer_entries(p)
    require(len(entries) == 18, f"QFP48 timer tail must contain D22..D39 (got {len(entries)})")
    mapping = dict(e.split(":", 1) for e in entries)
    require(mapping.get("33") == "TIMER3A", "D33/PF1 must map Timer3A")
    require(mapping.get("34") == "TIMER3B", "D34/PF2 must map Timer3B")
    require(mapping.get("35") == "TIMER3C", "D35/PF3 must map Timer3C")
    for d in ("36", "37", "38", "39"):
        require(mapping.get(d) == "NOT_ON_TIMER", f"D{d} alternate PWM must require explicit PMX")
    require(p.count("table size mismatch") >= 3, "variant table compile-time assertions missing")


def test_digital_and_analog_pin_guards() -> None:
    a = text("cores/lgt8f/Arduino.h")
    d = text("cores/lgt8f/wiring_digital.c")
    wa = text("cores/lgt8f/wiring_analog.c")
    require("< NUM_PIN_TABLE_ENTRIES" in section(a, "#define digitalPinToPort", "#define analogInPinToBit"),
            "Arduino pin lookup macros must be bounds-safe")
    for fn in ("void pinMode", "void digitalWrite", "int digitalRead", "void digitalToggle"):
        body = section(d, fn)
        first = body[:500]
        require("NUM_PIN_TABLE_ENTRIES" in first, f"{fn} must guard invalid pins before lookup")
    aw = section(wa, "void analogWrite", None)[:500]
    require("if (pin >= NUM_PIN_TABLE_ENTRIES) return;" in aw,
            "analogWrite invalid-pin guard missing")


def test_udsc_opcode_builders() -> None:
    u = text("libraries/LGT8Unlocked/src/lgt/udsc.h")
    required = {
        "OP_DIV": "0xB0", "OP_DIVMOD": "0xB1", "OP_CLEAR": "0x80",
        "OP_LOAD_Y": "0x1D", "OP_SQUARE_X_U": "0x88", "OP_SQUARE_Y_U": "0x8A",
    }
    for name, value in required.items():
        require(re.search(rf"\b{name}\s*=\s*{re.escape(value)}", u) is not None,
                f"uDSC opcode {name} mismatch")
    # Exhaustively mirror the documented 01 S1 S0 ... builder encoding.
    def mul(xs: bool, ys: bool, neg: bool = False, half: bool = False) -> int:
        return 0x40 | (0x20 if xs else 0) | (0x10 if ys else 0) | (0x08 if half else 0) | (0 if neg else 0x04)
    def mac(xs: bool, ys: bool, sub: bool = False, half: bool = False, signed_acc: bool = False) -> int:
        return 0x40 | (0x20 if xs else 0) | (0x10 if ys else 0) | (0x08 if half else 0) | (0 if sub else 0x04) | 0x02 | (1 if signed_acc else 0)
    require(mul(False, False) == 0x44 and mul(True, True, True, True) == 0x78,
            "uDSC multiply opcode reference invariant failed")
    require(mac(False, False) == 0x46 and mac(True, True, True, True, True) == 0x7B,
            "uDSC MAC opcode reference invariant failed")
    require("if(a==0||b==0)return0;" in u.replace(" ", ""), "uDSC dotProduct null guard missing")
    require("0x2100u" in u or "+0x2000u" in u, "uDSC SRAM alias helper missing")


def test_protected_writes_and_system_guards() -> None:
    c = text("libraries/LGT8Unlocked/src/lgt/common.h")
    s = text("libraries/LGT8Unlocked/src/lgt/system.h")
    us = text("libraries/LGT8Unlocked/src/lgt/usart.h")
    adc = text("libraries/LGT8Unlocked/src/lgt/adc.h")
    for helper in ("pmx0Write", "pmcrWrite", "vdtcrWrite"):
        body = section(c, f"void {helper}", "}")
        require("& 0x7Fu" in body, f"{helper} final write must clear WCE")
    body = section(c, "void pmx2Write", "}")
    require("& 0x67u" in body, "pmx2Write must clear WCE and reserved bits 4:3")
    body = section(c, "void mcucrWrite", "}")
    require("~_BV(0)" in body, "MCUCR protected write must clear WCE final bit")
    require("PeripheralADC" in s and "enum Peripheral:uint8_t{ADC=" not in s,
            "Power peripheral enum must not collide with AVR ADC macro")
    lvd = section(s, "struct LVD", "struct System")
    require("uint8_t v=_BV(SWR);" in lvd, "normal LVD configuration must keep active-low SWR high")
    sleep = section(s, "static inline Status sleepMode", "static inline void idle")
    require('"sei\\n\\tsleep\\n\\tcli"' in sleep, "sleep helper must use atomic SEI/SLEEP sequence")
    uspi = section(us, "struct USARTSPI0", None)
    end = section(uspi, "static inline void end()", "private:")
    restore = section(uspi, "static inline void modeAsync()", "}")
    require("modeAsync();" in end and "_BV(UCSZ01)|_BV(UCSZ00)" in restore,
            "USART-SPI end must restore asynchronous 8N1 frame size")
    trig = section(adc, "static inline void trigger", "static inline void disableTrigger")
    require("detail::adcUpdate(_BV(ADATE),0);" in trig, "ADC continuous/event trigger must enable ADATE without clearing ADIF")
    require("if(periodCode>3u)return InvalidArgument" in s, "DPS2 timer must reject invalid period codes")
    require("(uint16_t)(limit-byteAddress)<512u" in s, "vector relocation must stay inside active program partition")
    require("switchTo(ClockSource src" in s and "delayMicroseconds" in s, "clock source switch helper missing safe enable/delay ordering")


def test_timer2_and_spi_dual_hardening() -> None:
    t = text("libraries/LGT8Unlocked/src/lgt/timer.h")
    sp = text("libraries/LGT8Unlocked/src/lgt/spi_ext.h")
    block = section(t, "struct Timer2Async", None)
    for token in ("beginExternal32768Hz", "beginInternal32KHz", "TIMSK2", "TIFR2", "waitSync(uint32_t maxSpins=0)"):
        require(token in block, f"Timer2 async safe transition missing {token}")
    require("if(source==Internal32KHz)a|=_BV(INTCK)" in block, "Timer2 INTCK source selection missing")
    require("SPFR=(uint8_t)(_BV(RDEMPT)|_BV(WREMPT));" in sp, "SPI dual must start from a clean FIFO")
    receive = section(sp, "static inline Status receive2", "static inline SPIDualResult receive2")
    require(receive.count("SPDR") >= 3, "SPI dual transaction must write once and read two received bytes")
    require("SPFR=" not in receive, "SPI dual must not reset FIFO after every transaction")
    require("!(SPCR&_BV(SPE))" in sp, "SPI dual must require enabled master SPI")


def test_docs_and_version() -> None:
    platform = text("platform.txt")
    require("version=2.0.7" in platform, "platform version must match 2.0.7 package")
    for rel in ("docs/coverage.md", "docs/datasheet-errata.md", "docs/hardware-verification.md", "CHANGELOG-unlocked.md"):
        require((ROOT / rel).exists(), f"missing project documentation {rel}")


def main() -> int:
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_") and callable(v)]
    failed = 0
    for fn in tests:
        try:
            fn()
            print(f"PASS {fn.__name__}")
        except Exception as exc:
            failed += 1
            print(f"FAIL {fn.__name__}: {exc}")
    print(f"\n{len(tests)-failed}/{len(tests)} static regression groups passed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
