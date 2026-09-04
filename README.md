# LGT8F328P Unlocked Core

An Arduino-compatible core for the **LGT8F328P** and **LGT8F328D/E** — the
LogicGreen AVR-based MCUs that are pin-compatible with the ATmega328P but add
a 32 MHz clock, a 12-bit ADC, the uDSC 16-bit DSP coprocessor, and flash-emulated
EEPROM.

Every silicon-dependent claim in this repository is backed by measurements on
real hardware (LGT8F328P-LQFP48, 2026-09-04) rather than datasheet assumptions.
Findings are tracked as errata entries in `docs/datasheet-errata.md` (DOC-001..031).

## Feature matrix

| Feature | LGT8F328P | LGT8F328D/E |
|---------|-----------|-------------|
| uDSC 16-bit DSP coprocessor | Yes (silicon-verified) | Not present (stub) |
| 12-bit ADC, PGA x1/x8/x16/x32 | Yes | ADC only |
| Timer3 PWM + dead-time | Yes (QFP48) | No |
| HDR high-drive GPIO | Yes (6 pins) | No |
| SPI dual (2 lines in) | Yes | No |
| OPA0/OPA1 op-amps | No | Yes |
| DAC0 / DAC1 | DAC0 | DAC0 + DAC1 |
| Async Timer2 + RTC | Yes | Yes (no internal 32 kHz) |
| LGT8Unlocked library | 33 examples | 31 examples |

## Board support

- **328P-LQFP32** — Nano-style modules (WAVGAT and similar)
- **328P-LQFP48 MiniEVB** — QFP48 modules
- **328P-LQFP32 wemos-TTGO-XI** — ESP32-like form factor
- **328D** — LGT8F328D/E modules

## Installation

```
Arduino/
  └── hardware/
      └── lgt8funlocked/      ← extract the release zip here
          └── avr/
```

or clone directly:

```bash
cd ~/Arduino/hardware
git clone https://github.com/luckyprdn/lgt8f328p-unlocked-core.git lgt8funlocked
```

Restart the IDE, then select Board → **LGT8F328** and the matching Variant.
Upload speed is fixed at 57600 (matches the vendor bootloader).

## Library

The `LGT8Unlocked` library exposes 15 high-level objects plus the raw
low-level API. Everything follows the usual Arduino style:

```cpp
#include <LGT8Unlocked.h>

void setup() {
  AdcExt.readAverage(A0, 8);          // averaged 12-bit ADC read
  Dsp.multiply(30000, 2);             // saturating 16-bit multiply (uDSC on 328P)
  HdDrive.enable(lgt::HighDrive_PD5); // high-drive GPIO
  Dac.writeMillivolt(0, 1650);        // DAC in millivolts
  Pwm.timer1Frequency(1000);          // PWM frequency
  Pcint.attach(2, onButton, CHANGE);  // pin-change interrupt on any pin
}
```

Start with **`FeatureTour`** — a guided walkthrough of every module — then the
per-module `Example_*` sketches, the advanced examples (PGA, Timer3PWM, uDSC,
SPI dual, ...), and the silicon verification kit:

- `silicon_verify` — the 6 core silicon gates
- `final_sweep` — full byte-accurate sweep: arithmetic matrices, EEPROM 1020B,
  WDT real-reset proof, RTC, PRR domains, and cycle benchmarks (one upload)
- `danger_probe` — re-checks every feature flagged unreliable on this silicon
- `HardwareSmokeSafePro` — recovery-safe production smoke test

## Silicon notes (summary)

These are hardware behaviors verified on the LQFP48 die; full detail in the
errata document:

- WDT interrupt mode does not fire on this die — use reset mode (DOC-023).
- Native wide EEPROM program modes (16/32-bit) are broken and slow; the byte
  engine is the only correct path and is used by the library (DOC-024/030).
- DSP16 is saturating by contract (30000*2 = 32767), and `INT16_MIN / -1`
  saturates instead of wrapping (DOC-025/027).
- The DSSD register is not usable (DOC-022); extended opcode variants are
  SW-composed from verified primitives (DOC-026).
- `dotProductFast` (direct-SRAM window) matches, but does not beat, the I/O
  path at any measured length — kept as an API, with no performance claim
  (DOC-031).

## Measured performance (cycles/op @ 32 MHz, LQFP48)

| Operation | SW | uDSC | Notes |
|-----------|----|----|-------|
| div 32/16 | 648 | 158 | 4.1x - the headline win |
| dot16 | 722 | 310 | 2.3x for dot/FIR workloads |
| mul 16x16 | 77 | 102 | use SW for standalone multiplies |
| ADC read | 773 | 417 | `analogReadFast` 1.85x |
| EEPROM byte write | - | ~3 us | wide modes 65x slower (broken) |

## Safety

Recovery-safe mode is on by default: dangerous operations (flash IAP, clock
switches, EEPROM resizing, LVD configuration, deep-sleep DPS2) return `Locked`
unless the firmware is built with the recovery-safe gate disabled. Sketch
uploads through the bootloader can never touch those regions.

## Documentation

- `docs/datasheet-errata.md` — DOC-001..031: measured silicon behavior vs. datasheet
- `libraries/LGT8Unlocked/docs/api-reference.md` — full API reference
- Each example is self-documenting (connect/watch/silicon notes in the header)

## License

GNU Lesser General Public License v2.1
