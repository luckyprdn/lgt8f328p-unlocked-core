# Static regression tests

Run from the repository root:

```sh
python3 tests/static/test_unlocked.py
```

These checks protect source-level invariants for the Unlocked hardening work: EEPROM page mapping, Wire bounds, QFP48 pin/timer tables, invalid-pin guards, uDSC opcode scaffolding, protected-write sequencing, ADC trigger semantics and system/USART safety fixes.

They intentionally do **not** claim AVR instruction validity, cycle counts, peripheral behavior or electrical correctness. Those require the AVR toolchain and the hardware gates in `docs/hardware-verification.md`.
