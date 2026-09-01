# Actual core AVR codegen probe

This gate includes the **real** `cores/lgt8f/Arduino.h` and the QFP48 variant,
then compiles a C probe with Clang's AVR backend. Lightweight standard-library
and avr-libc headers under `stubs/` exist only because avr-libc is unavailable
in the execution environment.

The test proves the new compatibility API fast path at machine-code level:

- `digitalWrite(13, HIGH)` emits `SBI PORTB,5`.
- `digitalToggle(13)` emits a single write to `PINB`.
- a PWM constant (`D9`) still calls normal `digitalWrite()` so PWM-disconnect
  semantics are preserved.
- a runtime pin also calls normal `digitalWrite()`.

It does not replace compiling the full core with the LogicGreen avr-gcc package.
