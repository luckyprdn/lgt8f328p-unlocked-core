# LGT8Unlocked Library — API Reference

Header: `#include <LGT8Unlocked.h>`

Targets the LGT8F328P (`__LGT8FX8P__`) and LGT8F328D/E (`__LGT8FX8E__`).
Features the silicon lacks are not hidden: the method returns `Unsupported`
(or a stub value) so the API stays identical across chips.

Every behavioral note marked silicon-verified comes from hardware tests on
LGT8F328P-LQFP48; the full list is `docs/datasheet-errata.md` (DOC-001..031).

## Status enum

`lgt::Ok = 0, InvalidArgument, OutOfRange, NotReady, Unsupported, Locked`

Methods returning `Status` should be checked for `Unsupported` (feature not on
this chip) and `Locked` (recovery-safe gate — expected in the default build).

## Objects

### 1. AdcExt
- `readAverage(pin, n = 8)` — averaged 12-bit read
- `monitor(low, high, consecutive = 1)` — window monitor (P)
- `setGain(ADCPGAGain)` — PGA x1/x8/x16/x32 (P)
- `calibrate(samples = 8)` — offset calibration (P)
- `differentialViaOPA[1](invert)` — differential ADC via OPA (D)

### 2. Dsp
- `multiply / divide / modulo(a, b)` — saturating 16-bit (uDSC on P)
- `map(x, inMin, inMax, outMin, outMax)`
- `dot(a, b, n)` — dot product (uDSC)
- `dotFast(a, b, n)` — dot product via direct-SRAM window (P; equal speed)
- `fir(x, h, n)` / `firFast` — filter tap
- `average(a, n)` — MAC-based array average
- `convolve(x, nx, h, nh, out)` — full convolution, SW reference

Note: DSP16 arithmetic saturates (DOC-025/027): `30000*2 == 32767`,
`-32768/-1 == 32767`. Use uDSC for divmod and dot/FIR batches; standalone
16-bit multiplies are faster in plain C (DOC-031).

### 3. Dac
- `writeMillivolt(ch, mv, ref = 3300)` — DAC0 always; DAC1 on D only

### 4. HdDrive  (P only)
- `enable / disable(HighDrivePin)` — high-drive GPIO
- `enabled(pin)`

### 5. Pwm
- `timer1Frequency(hz)` — fast PWM on pin 9, returns prescaler
- `timer3Frequency(hz, cs)` — QFP48 (PF1/OC3A)
- `deadTime0/1/3(aTicks, bTicks)` — anti-shoot-through (P)
- Also `Timer3::duty/output` raw helpers

### 6. Timer
- `captureEdge(rising, noiseCancel)` — Timer1 input capture
- `asyncInt32K(maxSync)` — async T2 on internal 32 kHz (P)
- `asyncExt32768(maxSync)` — async T2 on external crystal
- Note: async T2 is exclusive with `tone()`.

### 7. Comp
- `enable(ch, yes)` / `output(ch)` / `setEdge(ch, edge)` /
  `setFilter(ch, filter)` / `enableInterrupt(ch, yes)` / `clearFlag(ch)`

### 8. Usart
- `enable9Bit / write9 / read9 / available9` — 9-bit frames
- `spiBegin / spiTransfer / spiEnd` — USART as SPI master
- Note: enabling 9-bit changes the framing of the same UART used by `Serial`.

### 9. SpiDual  (P only)
- `begin / receive2 / receive(buf, n) / end` — two data lines in

### 10. PinMux
- `ssToPB1 / txdToPD6 / rxdFromPD5 / pc6AsGPIO / pe6AsGPIO / extOscPins`
  — all reversible with `false`

### 11. Sys
- `chipId()` — 32-bit GUID
- `lastResetCause() / clearResetCause()`
- `programLimit()` — recovery-safe sketch ceiling
- `swdDisabled()`

### 12. Pwr
- `idle / adcNoiseReduction / powerSave / powerDown0 (dps0) / powerDown1 (dps1)`
- `peripheral(Peripheral, on)` — PRR clock-domain gating (safe in any build)
- DPS2 deep sleep exists on P but is `Locked` in recovery-safe builds
  (raw API: `lgt::Power::dps2/dps2Timer/dps2WakePins`)

### 13. Opa  (D only)
- `enable(ch, yes)` — OPA0/OPA1; returns `Unsupported` on P

### 14. Pcint
- `attach(pin, cb, mode)` / `detach(pin)` — any pin; one callback per pin
  (re-attaching replaces the callback and re-arms its edge state)

### 15. Lvd
- `setThreshold(threshold, resetEnable, enable)` — returns `Locked` in
  recovery-safe builds (expected)

### 16. Rtc  (328P internal 32 kHz; Unsupported on D)
- `begin()` — 1 Hz seconds counter on async Timer2; survives power-save
- `seconds() / set(s) / end()` — 32-bit counter, cli-guarded read
- Drift follows the internal RC (~+/-1%); use an external 32.768 kHz crystal
  with `Timer.asyncExt32768` for precision. Exclusive with `tone()`.

## Free functions

`analogReadAverage(pin, n)`, `pwmFrequency(hz)`, `dacWriteMillivolt(ch, mv, ref)`
— plain-Arduino wrappers over the objects above.

## Notes on silicon behavior

- The uDSC must never be enabled outside the library transactions (scoped +
  interrupt-masked, DOC-018); do not drive the DSU registers directly.
- EEPROM: use the library API. It performs byte-mode writes (native wide
  program modes are broken on this die) and serializes on the busy flag
  (DOC-024/028/030).
- WDT is reset-mode only on this silicon (DOC-023) — `Lgtwdt` from `<WDT.h>`.
