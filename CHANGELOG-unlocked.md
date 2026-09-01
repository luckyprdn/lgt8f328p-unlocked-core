# LGT8F328P Unlocked changelog

## 0.9.0-source-rc2-safe-hdr-hwtest — 2026-08-31

- merged the recovery-safe/HDR/EEPROM-lock profile with the current bug-hunt hardening wave
- hardened HardwareSerial, SoftwareSerial, tone(), pulseIn(), attachInterrupt(), Wire/TWI recovery and slave-TX bounds
- made ADC/comparator configuration W1C-safe and tightened ADC calibration/monitor/PGA validation
- corrected uDSC 16-bit SRAM alias access to use the documented R0<->DX fast-map semantics
- tightened PMX reserved-bit writes, clock/power/sleep enum validation, PCINT cleanup, Timer0 dead-time and Timer1 high-speed capture reads
- made USART MPCM writes W1C-safe and USART-SPI end release XCK
- made Flash IAP fail closed when its required HFRC/controller clock is unavailable
- hardened WDT protected-write timing
- dangerous recovery-sensitive operations remain locked; HDR stays explicitly enabled; EEPROM partition stays fixed at default 1KB / 1020 logical bytes
- added `HardwareSmokeSafe` and a 27-class RC2 hardening regression gate

## 0.9.0-source-rc1 — 2026-08-31

Source implementation of the Unlocked PRD:

- pin lookup hardening and QFP48 Timer3 mapping
- constant-pin GPIO fast path
- `analogReadFast`
- Wire zero-length/ISR bounds hardening, configurable buffers, address mask, general call and working `flush`
- EEPROM logical/physical bounds fixes, 32-bit/SWM hardening and `change_size` correction
- Standard/Lean/Tiny optional SRAM profiles
- new `LGT8Unlocked` extension library for GPIO/HDR/PCINT, PMX, timers, ADC/PGA/DAC/comparators, uDSC, SPI dual mode, advanced USART, clock/power/reset/GUID/IVBASE/SWD and guarded Flash IAP
- static audit suite, host runtime/syntax gates, AVR-Clang and actual-core AVR codegen gates, coverage matrix, datasheet ambiguity log and hardware verification plan

This is deliberately not tagged v1.0: generic AVR-backend codegen passes, but the vendor LogicGreen avr-gcc build matrix, hardware regression and performance/electrical benchmarks remain release gates.

## 0.9.0-source-rc1-safe1

- Recovery-safe mode enabled by default (`LGT8_UNLOCKED_RECOVERY_SAFE=1`).
- Flash IAP removed from aggregate header/examples and internally locked if manually included.
- SWD disable, interrupt-vector relocation, runtime configuration reload, main clock switching, RC calibration writes, DPS2, LVD writes, and 80 mA HDR are locked.
- Safe/reversible read paths remain available.
- Added `RecoverySafeMode` example and `RECOVERY-SAFE-MODE.md`.
- Recovery-safe macro is injected globally by `platform.txt`, so core and legacy libraries see the same interlock.
- Core `pinMode(E0/E2, ...)` no longer auto-disables SWD.
- Legacy `LowPower.deepSleep2()` is hidden and its DPS2 example removed.
- Legacy `PMU` library is omitted from this temporary safe package.
