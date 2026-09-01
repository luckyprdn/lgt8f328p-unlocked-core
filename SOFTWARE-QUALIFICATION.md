# LGT8F328P Unlocked — Software Qualification Report

**Candidate:** 0.9.0-source-rc2-safe-hdr-hwtest
**Date:** 2026-08-31
**Baseline commit:** `823dcc6`
**Target:** LGT8F328P (`__LGT8FX8P__`)
**Language level:** GNU C11 / GNU C++11 compatible extension layer

## Verdict

**Software-side qualification available in this environment: PASS.**

This candidate is intentionally **not v1.0 hardware-qualified**. The remaining release gates require a real LGT8F328P and, for timing/electrical claims, an oscilloscope/logic analyzer/current measurement setup.

## Implemented scope

- core pin/bounds hardening and QFP48 Timer3 mapping
- constant-pin Arduino GPIO fast path while retaining runtime/PWM compatibility semantics
- `analogReadFast()` single-conversion path
- Wire zero-length and buffer hardening, timeout-aware flush, address mask/general-call configuration, configurable buffer length
- EEPROM logical/physical mapping hardening, native 32-bit/SWM guards and size-change fix
- Serial/Wire Standard/Lean/Tiny SRAM profiles
- `LGT8Unlocked` extension library covering GPIO/HDR/PCINT, PMX, Timer0/1/2/3 extensions, capture, dead-time/fault control, ADC/PGA/DAC/comparators, uDSC/16-bit SRAM access, SPI dual receive, advanced USART/USART-SPI, clock/power/LVD/reset/GUID/IVBASE/SWD and guarded Flash IAP
- examples, static regression tests, host runtime tests, AVR-backend codegen tests, coverage matrix, datasheet errata register and hardware handoff plan

## Executed gates

| Gate | Result | What it proves |
|---|---|---|
| `python3 tests/static/test_unlocked.py` | **PASS 9/9** | source invariants, QFP48 tables, EEPROM mapping/controller state, Wire guards, uDSC opcode builders, protected-write/system guards |
| `python3 tests/static/test_rc2_hardening.py` | **PASS** | 27 bug-class/interlock regression checks for the RC2 merge |
| `tests/host_syntax/run.sh` | **PASS** | aggregate extension syntax, fake-register runtime behavior, all 14 extension examples parse/compile in the host harness |
| `tests/host_syntax/legacy_run.sh` | **PASS** | modified `Wire.cpp`, `EEPROM.cpp`, `twi.c` plus Wire boundary runtime behavior |
| `tests/avr_clang/run.sh` | **PASS** | Clang AVR backend emits AVR code for aggregate extension + 14 examples; uDSC path emits `IN`/`OUT` instructions |
| `tests/avr_core_codegen/run.sh` | **PASS** | actual core/variant sources compile through AVR backend; D13 constant write becomes `SBI`; runtime/PWM paths preserve normal `digitalWrite`; Wire/TWI/EEPROM and RAM profiles compile; QFP48 table assertions fire at compile time |
| `git diff --check` | **PASS** | no whitespace-error patch defects |

## Machine-code check

The actual-core AVR probe verifies:

- constant `digitalWrite(13, HIGH)` lowers to `SBI PORTB,5`
- constant `digitalToggle(13)` lowers to a direct `PINB` write
- PWM pin constants keep the compatibility call path so PWM disconnect semantics are not silently bypassed
- runtime pins keep the normal generic lookup path

This validates the intended **Arduino ergonomics + compile-time fast path** design without changing dynamic-pin behavior.

## Important qualification boundary

The container does not include the LogicGreen-patched `avr-gcc`/avr-libc package. AVR codegen tests therefore use Clang's real AVR backend plus minimal compile stubs where avr-libc headers are unavailable. This catches register-width, C/C++ syntax and ordinary AVR instruction/codegen problems, but it cannot prove LogicGreen-specific assembler semantics or exact compatibility with the vendor toolchain.

The uDSC 16-bit `IN/OUT` behavior is especially LogicGreen-specific: the test proves AVR `IN`/`OUT` is emitted, **not** that stock AVR semantics understand the LGT8XM 16-bit extension. That must be verified using the real LogicGreen toolchain and silicon.

## Remaining hardware/vendor-toolchain gates

- full Arduino IDE/CLI build matrix with the actual LogicGreen AVR-GCC package
- flash/upload smoke tests on every target package/variant used in practice
- Timer0/1/3 PWM frequency, complementary outputs, dead-time and hardware-fault latency on scope
- Timer1/3 capture and Timer2 asynchronous-clock behavior
- ADC throughput/noise/calibration/window monitor and PGA accuracy
- DAC transfer curve and AC0/AC1 routing/filter/hysteresis/fault behavior
- SPI dual receive; USART 9-bit/synchronous/USART-SPI; TWI masks/general call/arbitration/timeout against real peers
- uDSC opcode edge cases, divide-by-zero flags, 16-bit SRAM alias and cycle benchmarks
- EEPROM boundary/power-loss behavior and destructive Flash/IAP tests on sacrificial pages
- clock calibration/output/source switching and LVD thresholds
- sleep/DPS modes and current measurements
- IVBASE and SWD recovery tests

See `docs/hardware-verification.md` for the executable handoff plan.

## Release policy

Tag **v1.0** only after the hardware matrix is complete and no P0/P1 correctness defect remains. Until then this package should be described as **0.9.0-source-rc2-safe-hdr-hwtest** or **software-qualified / hardware-pending**.

## Recovery-safe overlay

The RC2 hardware-test profile defaults `LGT8_UNLOCKED_RECOVERY_SAFE=1`. High-risk mutating APIs are locked until a verified hardware recovery path exists. The lockout itself is covered by `tests/static/test_recovery_safe.py` and host runtime assertions.
