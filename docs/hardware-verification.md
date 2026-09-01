# Hardware Verification Plan

No source-only run may mark `[H]` or `[B]`. Use an LGT8F328P board, logic analyzer/oscilloscope, a known I2C/SPI/UART peer, and a current-limited supply.

## Gate H0 — smoke and recovery

1. Flash baseline Blink and Serial echo.
2. Verify SWD/ISP recovery before testing `disableSWD`, PC6 reset repurpose, clock switching, IAP or DPS2.
3. Record exact package/board, oscillator, VCC, core commit and compiler version.

## Gate H1 — digital/timers

- exercise every exposed digital pin; invalid pin calls must have no observable side effect
- QFP48 OC3A/B/C on PF1/PF2/PF3
- Timer1/3 input capture against signal generator
- Timer0/1/3 dead-time measured on scope
- each hardware fault source tested one at a time; verify PWM disconnect is hardware-latency behavior
- Timer0/1 high-speed mode measured; verify shared F2XEN handling

## Gate H2 — analog

- compare compensated `analogRead` and `analogReadFast` throughput/noise
- run fixed offset calibration at multiple VCC/temperatures and record OFR0/OFR1
- verify threshold monitor lower/upper flag and consecutive-sample filter
- sweep PGA gain/input combinations
- DAC transfer curve
- AC0/AC1 input selection, hysteresis, digital filters, interrupt/output/capture/fault routing

## Gate H3 — communications

- Wire 0/1/max/max+1 requests; slave general call; TWAMR masks; arbitration recovery; timeout/flush
- SPI FIFO regression and dual receive: one SPDR write must yield two bytes
- USART 9-bit/MPCM, synchronous mode and USART-SPI mode loopback/peer tests

## Gate H4 — nonvolatile/system

- EEPROM logical addresses around 1019/1020 and multi-page SWM without touching reserved page metadata
- IAP only in a sacrificial, known-unused page; power-fail tests are required before production use
- clock source/prescaler/output/calibration frequency measurements
- IDLE/SAVE/DPS0/DPS1/DPS2 wake sources and current measurements
- LVD thresholds and reset-cause flags
- IVBASE relocation with a sacrificial test image
- GUID stability across resets

## Gate H5 — uDSC

For every opcode family compare against a software reference across edge values (`0`, `1`, max, signed min/max, divide by zero). Inspect generated assembly for the special 16-bit IN/OUT and remapped SRAM paths. Measure cycles for multiply, MAC, division, dot product and any future vector/FIR helper. Only merge algorithm-level acceleration when it wins the PRD benchmark gate.
