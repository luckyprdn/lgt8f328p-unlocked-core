#ifndef LGT8_UNLOCKED_ARDUINO_EXT_H
#define LGT8_UNLOCKED_ARDUINO_EXT_H

// High-level, Arduino-style convenience wrappers over the LGT8Unlocked
// low-level API.  Everything here is cross-silicon: on LGT8F328P the DSP
// helpers transparently use the uDSC coprocessor; on LGT8F328D/E they fall
// back to native AVR arithmetic and the op-amp / Timer3 helpers return
// Unsupported.  One-line calls, no register knowledge required.

#include "common.h"
#include "adc.h"
#include "dac.h"
#include "udsc.h"
#include "timer.h"
#include "gpio.h"
#if LGT8_UNLOCKED_HAS_OPA
#include "opa.h"
#endif

namespace lgt {

// ---------------------------------------------------------------------------
// ADC
// ---------------------------------------------------------------------------

// Average N analogRead() samples (noise reduction).  Default 8 samples.
inline uint16_t analogReadAvg(uint8_t pin, uint8_t n = 8) {
  if (n == 0) n = 1;
  uint32_t s = 0;
  for (uint8_t i = 0; i < n; ++i) s += analogRead(pin);
  return (uint16_t)(s / n);
}

// Watchdog-style ADC window: returns true once the ADC value is inside
// [low, high] for `consecutive` consecutive conversions.  P only; D -> false.
inline bool analogReadMonitor(uint8_t pin, uint16_t low, uint16_t high, uint8_t consecutive = 1) {
  (void)pin;
  return ADCAdvanced::monitor(low, high, consecutive) == Ok;
}

// Program the PGA differential gain (x1/x8/x16/x32).  P only; D -> Unsupported.
inline Status analogSetGain(ADCPGAGain gain) {
  return ADCAdvanced::configurePGA(PGAPosADCMux, PGANegADCMux, gain, true);
}

// Run the LogicGreen fixed-offset calibration.  P only; D -> Unsupported.
inline Status analogCalibrate(uint8_t samples = 8) {
  return ADCAdvanced::calibrateOffset(samples);
}

// ---------------------------------------------------------------------------
// DSP (uDSC on 328P, native on 328D/E)
// ---------------------------------------------------------------------------

inline int16_t dspMul(int16_t a, int16_t b) { dsp::DSP16 r = dsp::DSP16(a) * dsp::DSP16(b); return r.v; }
inline int16_t dspDiv(int16_t a, int16_t b) { if (b == 0) return 0; dsp::DSP16 r = dsp::DSP16(a) / dsp::DSP16(b); return r.v; }
inline int16_t dspMod(int16_t a, int16_t b) { if (b == 0) return 0; dsp::DSP16 r = dsp::DSP16(a) % dsp::DSP16(b); return r.v; }

// Arduino map() but the multiply/divide go through the uDSC when present.
inline int32_t dspMap(int32_t x, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax) {
  if (inMax == inMin) return outMin;
  dsp::DSP16 t = (int16_t)(x - inMin);
  t = t * (int16_t)(outMax - outMin) / (int16_t)(inMax - inMin);
  return (int32_t)t.v + outMin;
}

// Dot product (FIR filter tap): y = sum(a[i]*b[i]).  Uses uDSC MAC loop on P.
inline int16_t dspDot(const int16_t *a, const int16_t *b, uint16_t n) {
  if (!a || !b) return 0;
#if LGT8_UNLOCKED_HAS_UDSC
  return (int16_t)dsp::dotProduct(a, b, n);
#else
  int32_t s = 0; for (uint16_t i = 0; i < n; ++i) s += (int32_t)a[i] * (int32_t)b[i];
  return (int16_t)s;
#endif
}

// FIR filter convenience: y = sum(x[k] * h[k]).  Same as dspDot.
inline int16_t dspFIRFilter(const int16_t *x, const int16_t *h, uint16_t n) { return dspDot(x, h, n); }

// Integer average of an array via uDSC MAC accumulation (P) or native (D).
inline int16_t dspAverage(const int16_t *a, uint16_t n) {
  if (n == 0) return 0; if (!a) return 0;
#if LGT8_UNLOCKED_HAS_UDSC
  dsp::DSP16 acc = 0;
  for (uint16_t i = 0; i < n; ++i) acc.mac(a[i], 1);
  return (int16_t)(acc.v / n);
#else
  int32_t s = 0; for (uint16_t i = 0; i < n; ++i) s += a[i];
  return (int16_t)(s / n);
#endif
}

// ---------------------------------------------------------------------------
// DAC — millivolt-friendly (0..255 auto-converted)
// ---------------------------------------------------------------------------
inline void dacWriteMillivolts(uint8_t ch, uint16_t millivolts, uint16_t refMillivolts = 3300) {
  uint8_t v = (uint8_t)(((uint32_t)millivolts * 256u) / refMillivolts);
  if (v > 255) v = 255;
#if LGT8_UNLOCKED_HAS_DAC1
  if (ch == 1) { DAC1Advanced::write(v); return; }
#endif
  DAC0Advanced::write(v);
}

// ---------------------------------------------------------------------------
// PWM frequency helpers
// ---------------------------------------------------------------------------

// Timer1 fast PWM at a requested frequency on pin 9 (PB1/OC1A).
// Returns the prescaler code actually used (0 on error).
inline uint8_t pwmFreqT1(uint32_t hz) {
  if (hz == 0) return 0;
  uint32_t base = F_CPU / hz;
  uint32_t presc = 1; uint8_t cs = 1;
  if (base / presc > 65535) { presc = 8;  cs = 2; }
  if (base / presc > 65535) { presc = 64; cs = 3; }
  if (base / presc > 65535) { presc = 256; cs = 4; }
  if (base / presc > 65535) { presc = 1024; cs = 5; }
  uint32_t top = base / presc; if (top < 1) top = 1;
  pinMode(9, OUTPUT);
  TCCR1A = (uint8_t)((1 << WGM11) | (1 << COM1A1));
  TCCR1B = (uint8_t)((1 << WGM13) | (1 << WGM12) | cs);
  ICR1 = (uint16_t)top;
  return cs;
}

// Timer3 fast PWM (PF1/OC3A on QFP48) at a requested frequency.  P only.
inline Status pwmFreqT3(uint32_t hz, uint8_t cs = 1) {
#if LGT8_UNLOCKED_HAS_TIMER3
  if (hz == 0) return InvalidArgument;
  uint32_t top = F_CPU / ((uint32_t)cs * hz);
  if (top < 1) top = 1;
  if (top > 65535) return OutOfRange;
  Timer3::fastPwmICR((uint16_t)top, cs);
  return Ok;
#else
  (void)hz; (void)cs; return Unsupported;
#endif
}

// Timer3 dead-time insertion (ticks between complementary outputs).  P only.
inline Status pwmDeadTime3(uint8_t aTicks, uint8_t bTicks) {
#if LGT8_UNLOCKED_HAS_TIMER3
  Timer3::deadTime(aTicks, bTicks); return Ok;
#else
  (void)aTicks; (void)bTicks; return Unsupported;
#endif
}

// ---------------------------------------------------------------------------
// Op-amps (LGT8F328D/E only)
// ---------------------------------------------------------------------------
inline Status opamp0Enable(bool yes = true) {
#if LGT8_UNLOCKED_HAS_OPA
  OPA0::enable(yes); return Ok;
#else
  (void)yes; return Unsupported;
#endif
}
// Dual-channel auto-switch at a given period (counts).  D only.
inline Status opamp0AutoSwitch(uint8_t period) {
#if LGT8_UNLOCKED_HAS_OPA
  OPA0::channel(OPABoth);
  OPA0::setPeriod(period);
  OPA0::enableTimer(true);
  return Ok;
#else
  (void)period; return Unsupported;
#endif
}

// ---------------------------------------------------------------------------
// Pin-change interrupt (PCINT) with a callback — Arduino attachInterrupt style.
// mode: 0 = CHANGE, 1 = RISING, 2 = FALLING.  Implemented in arduino_ext.cpp.
// ---------------------------------------------------------------------------
Status pinChangeAttach(uint8_t pin, void (*callback)(void), uint8_t mode);
void   pinChangeDetach(uint8_t pin);

}

#endif