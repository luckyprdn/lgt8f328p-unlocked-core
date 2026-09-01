# LGT8F328P Unlocked RC2 — Recovery-Safe Hardware Test Quickstart

Candidate: `0.9.0-source-rc2-safe-hdr-hwtest`

This is the package intended for first hardware tests when no ISP/SWD recovery programmer is available.

## Safety profile

Enabled: normal GPIO, Serial, Wire/TWI, SPI, EEPROM inside the fixed default partition, ADC/PGA, DAC, comparators, uDSC, Timer0/1/2/3, PWM, capture, PCINT, reversible pin mux, normal sleep, and HDR/high-drive GPIO.

Locked: Flash IAP erase/write, SWD disable/takeover, IVBASE relocation, main-clock source switching, RC calibration writes, DPS2, LVD mutation, protected configuration reload, runtime EEPROM partition resizing, and the legacy PMU library.

HDR is enabled deliberately. Treat it as a stronger signal driver, not as an 80 mA power output. Do not short HDR pins to GND/VCC or another driven output.

## Install

Extract the contents of this package into a dedicated Arduino hardware vendor path, for example:

`~/Arduino/hardware/lgt8funlocked/avr/`

The directory must directly contain `boards.txt`, `platform.txt`, `cores/`, `variants/`, `bootloaders/`, and `libraries/`.

Keep the old LGT core backed up. Do not overwrite it without a backup.

## Board selection

For the 48-pin MiniEVB-style board, select:

- Board: `LGT8F328`
- Variant: `328P-LQFP48 MiniEVB`
- RAM profile: `Default / compatibility` for first boot
- Clock source/divider/upload speed: use the same known-working values as the board's baseline/vendor core first. Do not experiment with clock settings during the first test session.

## First test sequence

1. Upload a normal Blink using the baseline/vendor core. Confirm upload/reboot works.
2. Switch to this RC2 package and run `File -> Examples -> LGT8Unlocked -> HardwareSmokeSafe`.
3. Expected Serial milestones: banner, GUID/reset cause, `uDSC 30000*2=60000`, safe-lock statuses, and `SMOKE_DONE`.
4. Confirm D13 LED toggles about every 500 ms.
5. Then run `SystemInfo`, `uDSC`, `ADCMonitor`, `PGA`, `DAC`, `Timer3PWM`, `Timer2Async`, `ComparatorFault`, `PinMuxAndHDR`, `SpiDual`, `Usart9`, and `PowerModes` as applicable.
6. Do not add manual raw-register code that disables SWD, changes the EEPROM partition, changes the main clock source, or writes program Flash while operating without recovery hardware.

## Status values

`0=Ok`, `1=InvalidArgument`, `2=OutOfRange`, `3=NotReady`, `4=Unsupported`, `5=Locked`.

In `HardwareSmokeSafe`, the four recovery-sensitive probes should report `5` (`Locked`).

## What to send back after testing

For every test, record board variant, selected clock/divider/upload speed, sketch name, PASS/FAIL, Serial output, and any logic-analyzer/scope measurement. A photo or capture is useful for PWM/dead-time/DAC/comparator tests.
