# Verification coverage

Module-level coverage of the silicon/behavior claims in this repository.
"Code" = exercised by the test matrix or host tests; "Hardware" = exercised
on LQFP48 (2026-09-04); "Open" = needs instrument (see
`hardware-verification.md` and the errata status column).

| Module | Code | Hardware | Notes |
|--------|------|----------|-------|
| uDSC register access | x | x | pair + nop (DOC-021); 20k stress (DOC-018) |
| uDSC divmod | x | x | 80-case matrix incl. 0x80000000+ (DOC-025) |
| DSP16 (+, -, *, /, %, mac) | x | x | 81-combo saturating matrix (DOC-025/027) |
| dsp::mul byte fast-path | x | x | 256-pair matrix (DOC-029) |
| Extended op variants | x | x | SW-composed (DOC-026) |
| dotProductFast (SRAM window) | x | x | equal to I/O path (DOC-031) |
| convolve | x | x | T4 assert |
| WDT reset mode | x | x | arm/feed/disarm + real-reset proof (DOC-023) |
| WDT interrupt mode | - | x | conclusively absent on this die (DOC-023) |
| EEPROM byte engine | x | x | 1020B sweep + SWM + OOB (DOC-024) |
| EEPROM wide modes | - | x | broken+slow (DOC-024/030) |
| EEPROM serialization | x | x | EEPE busy-wait (DOC-028) |
| RTC (async Timer2) | x | x | 1 Hz, survives power-save |
| PRR peripheral domains | x | x | TWI roundtrip |
| ADC 12-bit / averaging | x | x* | *floating pin only - needs tie |
| ADC INL/PGA gain accuracy | x | - | open (voltage source) |
| SPI (standard) | x | - | open (loopback jumper) |
| SPI dual | x | - | open (two data lines + device) |
| DAC | x | - | open (voltmeter) |
| Timer3 | x | x | counts verified; PWM scope open |
| Comparator | x | - | polling verified by code path |
| USART 9-bit / USART-SPI | x | - | needs peer |
| PCINT | x | - | needs button |
| HDR | x | x* | *state readback; brightness open |
| PinMux | x | - | reversible calls compile-verified |
| Low-power modes | x | - | current open (ammeter) |
| LVD | x | - | reset behavior needs supply sweep |
| WString / String edges | x | - | host + smoke |
| Wire / TWI | x | - | needs I2C peer |
| Recovery-safe locks | x | x | smoke lock checks |
