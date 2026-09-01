# LGT8F328P Unlocked Core

**LGT8F328P Arduino core** — datasheet-verified, recovery-safe, aggressively hardened.

Fork of [`dbuezas/lgt8fx`](https://github.com/dbuezas/lgt8fx) with extended LGT8Unlocked library,
register-level datasheet accuracy fixes, memory safety hardening, and a recovery-safe mode
that prevents bricking the chip without an ISP/SWD programmer.

## Features

- **Recovery-Safe** — SWD lock, Flash IAP lock, clock switch lock, LVD lock, EEPROM partition clamp.
  Safe to flash without a programmer. Dangerous paths return `Locked` at runtime.
- **Datasheet-Accurate** — 50 register addresses, 156 bit positions — **100% match** vs LGT8FX8P v1.0.5.
  17 datasheet ambiguities documented in [`docs/datasheet-errata.md`](docs/datasheet-errata.md).
- **LGT8Unlocked Library** — uDSC (22 opcodes, 16-bit alias LD/ST), ADC with PGA + threshold monitor,
  Timer advanced (dead-time, capture, fault), DAC, Comparators, PinMux, GPIO (HDR high-drive, PCINT),
  Clock/Power/LVD system API. All parameter-validated, all recovery-safe-gated.
- **32-bit EEPROM Native** — `lgt_eeprom_read32/write32/readSWM/writeSWM` with automatic hole-skipping,
  4-byte alignment enforcement, bounds checks, and EERIE preservation.
- **Hardened Core** — baud=0 guard, 64-bit UBRR, tone OCR overflow → noTone, attachInterrupt null/mode guard,
  pin bounds everywhere, WString overflow + startsWith/indexOf memory-safety fixes.
- **Performance** — Default 32MHz, Fast IO (compile-time SBI codegen), `analogReadFast` (single conversion),
  ADC prescaler 16 (2MHz, optimal per datasheet).
- **Test Coverage** — 51/56 sketches compile, 14 LGT8Unlocked examples × 4+ variants, static regression suite,
  host-side memory safety tests.

## Quick Install

```sh
# Backup existing core if any
cp -r ~/Arduino/hardware/lgt8funlocked/avr ~/Arduino/hardware/lgt8funlocked/avr.bak

# Clone into Arduino hardware directory
git clone https://github.com/luckyprdn/lgt8f328p-unlocked-core ~/Arduino/hardware/lgt8funlocked/avr

# Open Arduino IDE → Board → LGT8F328 → select variant
# Or compile via CLI:
arduino-cli compile --fqbn "lgt8funlocked:avr:328:clock_source=internal,clock_div=1,variant=modelP48,upload_speed=115200,arduino_isp=disable,ram_profile=default" sketch/sketch.ino
```

## Variants

| Board | Package | Timer3 PWM |
|---|---|---|
| `variant=modelP` | LQFP32 | D1/D2 (via PMX) |
| `variant=modelP48` | QFP48 | PF1/PF2/PF3 |
| `variant=modelP_SSOP20` | SSOP20 | D1/D2 (via PMX) |
| `variant=modelP-wemos-TTGO-XI` | Wemos TTGO | PF1/PF2/PF3 |
| `variant=modelD` | LGT8F328D/E | N/A |

## Clock Menu

- Internal 32MHz RC (default, ±1% calibrated)
- External 32/16/12 MHz crystal
- Divider 1, 2, 4, 8, 16, 32

## RAM Profiles

| Profile | Serial RX | Serial TX | Wire Buffer | SRAM Saved |
|---|---|---|---|---|
| Default | 64 | 64 | 32 | — |
| Standard | 64 | 64 | 32 | — |
| Lean | 32 | 32 | 16 | ~100 B |
| Tiny | 16 | 16 | 16 | ~150 B |

## EEPROM Safety

Default partition: **1KB logical (1020 bytes usable, 2KB physical at 0x7800–0x7FFF)**.

- Bootloader lives at **0x7400–0x77FF** — safe at default 1KB
- Expanding to 2KB+ overwrites the bootloader
- Recovery-safe clamps the partition to 1KB — `change_size()` is a no-op

**NEVER disable `LGT8_UNLOCKED_RECOVERY_SAFE` unless you have an ISP/SWD programmer.**

## Caveats

- V-USB examples (Vkeyboard, VUsbMouse, CircleMouse) require **external 12–20 MHz clock**
  — not compatible with internal 32 MHz
- `LGT8Unlocked` library targets **LGT8F328P silicon only** (modelD returns `#error`)
- `sysclock` example switches to external crystal — don't run without one

## Credits

- [dbuezas/lgt8fx](https://github.com/dbuezas/lgt8fx) — base Arduino platform
- [SuperUserNameMan](https://github.com/SuperUserNameMan) — EEPROM library + hardware validation
- [LogicGreen Technologies](http://www.lgtic.com/) — LGT8FX8P datasheet v1.0.5
- RC2 bug-hunt team — 27 bug classes hardened

## Errata

See [`docs/datasheet-errata.md`](docs/datasheet-errata.md) for 17 documented datasheet ambiguities,
naming contradictions, and implementation decisions.

## Hardware Smoke Test

Flash `HardwareSmokeSafePro` for a comprehensive verification:

```sh
arduino-cli compile --fqbn "lgt8funlocked:avr:328:clock_source=internal,clock_div=1,variant=modelP48,upload_speed=115200,arduino_isp=disable,ram_profile=default" \
  ~/Arduino/hardware/lgt8funlocked/avr/libraries/LGT8Unlocked/examples/HardwareSmokeSafePro
```

Covers: uDSC, EEPROM native 32-bit + SWM + hole-skip, Timer3 D1/D2 PWM, analogReadFast,
WString edge cases, HDR high-drive GPIO, safe-lock asserts, watchdog sanity.