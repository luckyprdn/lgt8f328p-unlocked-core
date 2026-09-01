# LGT8F328P Unlocked Coverage Matrix

Status meanings: **FULL-SRC** = application-facing source implementation present; **PARTIAL** = legacy support exists but some silicon behavior remains outside the high-level API; **RAW** = documented register access only; **VERIFY-HW** = implementation requires board measurement before release qualification.

| Subsystem | Feature | Baseline | Unlocked source | Verification |
|---|---|---:|---:|---|
| Core | Arduino compatibility | yes | preserved | AVR-Clang/core-codegen PASS; vendor avr-gcc + hardware pending |
| GPIO | bounds-safe pin tables | bugged/partial | FULL-SRC | static test |
| GPIO | constant-pin fast write/read/toggle | separate fast API | FULL-SRC | AVR codegen fast-path PASS; cycle benchmark pending hardware/vendor toolchain |
| GPIO | high drive HDR0..5 | raw only | FULL-SRC | VERIFY-HW/current limits |
| GPIO | PCINT groups 0..4 | low-level | FULL-SRC | VERIFY-HW |
| PinMux | PMX0/1/2 routes | partial | FULL-SRC for documented public routes | VERIFY-HW |
| Timer0 | basic Arduino PWM/timebase | yes | preserved | regression pending |
| Timer0 | dead-time | missing | FULL-SRC | VERIFY-HW/scope |
| Timer0 | automatic fault shutdown | missing | FULL-SRC | VERIFY-HW |
| Timer0 | 64 MHz high-speed source | missing | FULL-SRC | VERIFY-HW/scope |
| Timer1 | capture | raw | FULL-SRC | VERIFY-HW |
| Timer1 | dead-time/fault | missing | FULL-SRC | VERIFY-HW/scope |
| Timer1 | independent/high-speed prescaler | missing | FULL-SRC | VERIFY-HW |
| Timer2 | async 32.768 kHz | partial/raw | FULL-SRC helper | VERIFY-HW |
| Timer3 | A/B/C PWM mapping QFP48 | incomplete | FULL-SRC | VERIFY-HW |
| Timer3 | capture | raw | FULL-SRC | VERIFY-HW |
| Timer3 | complementary/dead-time/fault | missing | FULL-SRC | VERIFY-HW/scope |
| ADC | 12-bit compensated `analogRead` | yes | preserved | existing behavior |
| ADC | single-conversion `analogReadFast` | missing | FULL-SRC | benchmark pending |
| ADC | event trigger | raw | FULL-SRC | VERIFY-HW |
| ADC | threshold auto-monitor | missing | FULL-SRC | VERIFY-HW |
| ADC | fixed offset calibration | partial/manual | FULL-SRC routine | VERIFY-HW/accuracy |
| ADC | differential PGA | existing separate library | typed FULL-SRC API | VERIFY-HW |
| DAC | 8-bit DAC0 | yes via ANALOG/analogWrite | typed FULL-SRC API | VERIFY-HW |
| AC0 | input/filter/hysteresis/IRQ/output/capture route | power-only baseline | FULL-SRC | VERIFY-HW |
| AC1 | input/filter/hysteresis/IRQ/output/capture route | power-only baseline | FULL-SRC | VERIFY-HW |
| uDSC | DX/DY/DA + all documented opcode families | register defs only | FULL-SRC | AVR-Clang codegen PASS + VERIFY-HW |
| uDSC | 16-bit SRAM alias/load/store | missing | FULL-SRC | AVR-Clang codegen PASS + VERIFY-HW |
| uDSC | algorithms | missing | dot product reference only | benchmark-gated by PRD |
| SPI | normal/FIFO | yes | preserved | regression pending |
| SPI | dual receive | missing | FULL-SRC | VERIFY-HW |
| USART | async Arduino Serial | yes | preserved | regression pending |
| USART | 9-bit/MPCM/synchronous | missing | FULL-SRC | VERIFY-HW |
| USART | USART-SPI master/slave | missing | FULL-SRC | VERIFY-HW |
| TWI | master/slave/arbitration/general-call status | yes/partial | preserved | regression pending |
| TWI | zero-length/buffer hardening | bugged | FULL-SRC fix | static test |
| TWI | address mask/general-call config/flush | missing/TODO | FULL-SRC | VERIFY-HW |
| EEPROM | byte/block emulation | yes | hardened logical/real bounds | static + VERIFY-HW |
| EEPROM | 32-bit/SWM | partial | hardened/page-safe | VERIFY-HW |
| Flash | IAP erase/write | boot/tool only | guarded FULL-SRC API | destructive VERIFY-HW |
| Clock | source/prescaler | partial | FULL-SRC typed API | VERIFY-HW |
| Clock | HFRC/LFRC calibration | raw | FULL-SRC | VERIFY-HW/frequency counter |
| Clock | clock outputs | raw | FULL-SRC protected writes | VERIFY-HW |
| Power | PRR/PRR1 gating | partial | FULL-SRC | VERIFY-HW |
| Power | IDLE/ADC noise/SAVE/DPS0/DPS1/DPS2 | existing LowPower | unified FULL-SRC helpers retained alongside legacy library | VERIFY-HW/current |
| Reset | reset cause/LVD/software reset | partial | FULL-SRC | VERIFY-HW |
| Interrupt | IVBASE relocation | raw | guarded FULL-SRC | VERIFY-HW/boot test |
| Device | GUID | raw | FULL-SRC | VERIFY-HW |
| SWD/OCD | SWD ISP | existing | explicit status/disable + recovery docs | debugger integration remains tooling-dependent |
| RAM | Wire buffer size | fixed 32 | configurable | profile compile PASS; final size benchmark pending vendor toolchain |
| RAM | Serial RX/TX | RX board option / defaults | optional Standard/Lean/Tiny profiles | profile compile PASS; final size benchmark pending vendor toolchain |

## PRD completion interpretation

All application-facing workstreams have a source implementation or are explicitly benchmark/tooling-gated. The project is **not v1.0-qualified** until the vendor LogicGreen avr-gcc build matrix, real-board regression, electrical tests and performance measurements are completed. Generic AVR-backend compilation/disassembly gates already pass. That is deliberate: the PRD defines hardware verification and benchmarking as release gates, and those cannot be truthfully fabricated in a source-only environment.
