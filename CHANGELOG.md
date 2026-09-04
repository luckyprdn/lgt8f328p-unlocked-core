# Changelog

All notable changes to this core. Hardware-verified items cite the errata
log (`docs/datasheet-errata.md`, DOC-001..031).

## [v1.0.3] - 2026-09-04

- boards: upload speed fixed at 57600 (vendor bootloader is 57600-only;
  the selectable 115200/230400 menu was removed after sync failures)
- docs: English-only professional pass (README, API reference), licensing
  and third-party notices added

## [v1.0.2] - 2026-09-04

- Certified silicon run on LQFP48: `final_sweep` pass=11 fail=0 (T14 WDT
  real-reset proof clean)
- EEPROM: DOC-028 skip-write reverted (reset the chip mid-sweep on this
  die); all program paths now serialize on the EEPE busy flag
- New: `Rtc` (1 Hz async-Timer2 clock, survives power-save), `Pwr.peripheral()`
  (PRR domain gating), `Dsp.convolve()`
- Memory safety: Serial buffers >256 are a compile error (atomicity);
  PCINT attach re-arms edge state on callback replacement
- Measured guidance DOC-031: standalone uDSC mul not worth it;
  `dotProductFast` matches (not beats) the I/O path

## [v1.0.1] - 2026-09-04

- EEPROM skip-identical-byte writes (later reverted, see v1.0.2)
- `dsp::mul` byte fast-path for operands < 0x100 (~6 vs ~63 cycles)
- WDT library compiles on 328D/E (PMCR.WCLKS guard)

## [v1.0.0] - 2026-09-04

- First release documenting hardware-verified behavior on LQFP48
  (DOC-001..027)
- uDSC register-pair access (DOC-021), DSP16 saturating contract
  (DOC-025/027), WDT reset-mode only (DOC-023), EEPROM byte engine
  (DOC-024), extended-op SW composition (DOC-026)
- 34 examples including the silicon verification kit

## v1.0.4 - 2026-09-04

- perf: `Print::printNumber` (all Serial/print number output) now formats
  through the uDSC 32/16 DIVMOD on 328P - roughly 5-10x fewer CPU cycles
  per printed number. 328D/E/88A keep the software path.
