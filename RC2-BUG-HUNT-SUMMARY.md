# RC2 Bug-Hunt Merge Summary

This candidate merges the recovery-safe/HDR/EEPROM-lock profile with the current source-level bug-hunt wave. It remains hardware-pending.

## High-impact fixes included

- Wire zero-length/address/buffer hardening plus internal-address preamble failure propagation
- TWI timeout recovery preserves `TWAMR`; slave-TX ISR has defensive index bounds
- HardwareSerial zero-baud and UBRR arithmetic hardening
- SoftwareSerial invalid-pin and invalid-speed guards
- `tone()` zero/out-of-range arithmetic hardening and duration-counter saturation
- `pulseIn()`/`pulseInLong()` invalid-pin and conversion-overflow hardening
- `attachInterrupt()` rejects null callbacks and invalid modes
- ADC `ADIF` W1C-safe configuration, monitor validation, PGA enum validation, calibration busy/pending guards
- AC0/AC1 `C0I/C1I` W1C-safe configuration
- uDSC 16-bit alias load/store corrected to the documented fast-map `R0 <-> DX` semantics
- PMX1/PMX2 reserved-bit masks tightened
- Clock, power-peripheral and sleep-mode enum/range validation
- reset-cause clear preserves SWD state
- PCINT group cleanup when the last mask bit is disabled
- Timer0 dead-time rejects values above 15; Timer1 high-speed capture reads low/wait/high
- USART MPCM configuration avoids clearing TXC; USART-SPI end releases XCK
- Flash IAP fails closed when required HFRC/controller clock is unavailable
- WDT protected-write window keeps interrupts masked through the final write
- recovery-sensitive features remain locked; HDR remains enabled; EEPROM partition remains fixed

`tests/static/test_rc2_hardening.py` enforces 27 of these bug classes/interlocks as source invariants in addition to the older regression suite.
