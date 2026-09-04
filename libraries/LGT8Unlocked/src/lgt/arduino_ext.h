#ifndef LGT8_UNLOCKED_ARDUINO_EXT_H
#define LGT8_UNLOCKED_ARDUINO_EXT_H

// High-level, Arduino-style extensions for the LGT8F328P / LGT8F328D/E.
// Use them like Wire / Serial / SPI:
//   AdcExt.readAverage(A0, 8);
//   Dsp.multiply(30000, 2);
//   HdDrive.enable(lgt::HighDrive_PD5);
//   Pcint.attach(2, cb, PCINT_CHANGE);
//
// All methods are stateless and cross-silicon: on the 328P the DSP calls run
// on the uDSC coprocessor; on the 328D/E they fall back to native AVR math
// and features the silicon lacks (Timer3/OPA/HDR) return Unsupported.

#include "common.h"
#include "adc.h"
#include "dac.h"
#include "udsc.h"
#include "timer.h"
#include "gpio.h"
#include "comparator.h"
#include "pinmux.h"
#include "spi_ext.h"
#include "usart.h"
#include "system.h"
#if LGT8_UNLOCKED_HAS_OPA
#include "opa.h"
#endif

// PCINT mode constants as an enum so they do not clash with macros in lgtx8p.h
enum PCINTMode : uint8_t { PCINT_CHANGE = 0, PCINT_RISING = 1, PCINT_FALLING = 2 };

// ===========================================================================
// 1. ADC
// ===========================================================================
struct AdcObj {
  // Average of N analogRead samples (noise reduction). Default 8.
  inline uint16_t readAverage(uint8_t pin, uint8_t n = 8) const {
    if (n == 0) n = 1;
    uint32_t s = 0;
    for (uint8_t i = 0; i < n; ++i) s += analogRead(pin);
    return (uint16_t)(s / n);
  }
  // Window monitor: true jika ADC di [low, high] selama `consecutive`x. P only.
  inline bool monitor(uint8_t pin, uint16_t low, uint16_t high, uint8_t consecutive = 1) const {
    (void)pin;
    return lgt::ADCAdvanced::monitor(low, high, consecutive) == lgt::Ok;
  }
  // PGA gain (x1/x8/x16/x32). P only; D -> Unsupported.
  inline lgt::Status setGain(lgt::ADCPGAGain gain) const {
    return lgt::ADCAdvanced::configurePGA(lgt::PGAPosADCMux, lgt::PGANegADCMux, gain, true);
  }
  // Kalibrasi offset LogicGreen. P only; D -> Unsupported.
  inline lgt::Status calibrate(uint8_t samples = 8) const {
    return lgt::ADCAdvanced::calibrateOffset(samples);
  }
  // D only: OPA0/OPA1 front-end ke ADC_CH9 (differential). P -> Unsupported.
  inline lgt::Status differentialViaOPA(bool invertCh1 = true) const {
    return lgt::ADCAdvanced::differentialViaOPA(invertCh1);
  }
  inline lgt::Status differentialViaOPA1(bool invertCh1 = true) const {
    return lgt::ADCAdvanced::differentialViaOPA1(invertCh1);
  }
};
constexpr AdcObj AdcExt{};

// ===========================================================================
// 2. DSP — uDSC di P, native di D
// ===========================================================================
struct DspObj {
  inline int16_t multiply(int16_t a, int16_t b) const { return (lgt::dsp::DSP16(a) * lgt::dsp::DSP16(b)); }
  inline int16_t divide(int16_t a, int16_t b) const { if (b == 0) return 0; return (lgt::dsp::DSP16(a) / lgt::dsp::DSP16(b)); }
  inline int16_t modulo(int16_t a, int16_t b) const { if (b == 0) return 0; return (lgt::dsp::DSP16(a) % lgt::dsp::DSP16(b)); }
  // map(), scaled on the uDSC when available.
  inline int32_t map(int32_t x, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax) const {
    if (inMax == inMin) return outMin;
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
  }
  // Dot product / FIR filter.
  inline int32_t dot(const int16_t *a, const int16_t *b, uint16_t n) const {
    return lgt::dsp::dotProduct(a, b, n);
  }
  // Same dot product but X loaded via the DSC direct-SRAM window (P only;
  // D returns 0).  Requires SRAM arrays.
  inline int32_t dotFast(const int16_t *a, const int16_t *b, uint16_t n) const {
    return lgt::dsp::dotProductFast(a, b, n);
  }
  inline int16_t fir(const int16_t *x, const int16_t *h, uint16_t n) const {
    return (int16_t)lgt::dsp::dotProduct(x, h, n);
  }
  // FIR via the fast SRAM path (P only; D returns 0).
  inline int16_t firFast(const int16_t *x, const int16_t *h, uint16_t n) const {
    return (int16_t)lgt::dsp::firFast(x, h, n);
  }
  // Average of a 16-bit array via MAC.
  inline int16_t average(const int16_t *a, uint16_t n) const {
    if (n == 0 || !a) return 0;
    lgt::dsp::DSP16 acc = 0;
    for (uint16_t i = 0; i < n; ++i) acc.mac(lgt::dsp::DSP16(a[i]), 1);
    return (int16_t)((int32_t)acc.v / n);
  }
  // Full convolution out[i] = sum_k x[k]*h[i-k], length nx+nh-1.
  // Byte-exact reference implementation (SW). For heavy workloads prefer
  // chaining dotProduct() (uDSC) - the final_sweep benchmark compares both.
  inline void convolve(const int16_t *x, uint16_t nx, const int16_t *h,
                       uint16_t nh, int32_t *out) const {
    if (!x || !h || !out || nx == 0 || nh == 0) return;
    uint16_t L = (uint16_t)(nx + nh - 1);
    for (uint16_t i = 0; i < L; ++i) {
      int32_t acc = 0;
      int16_t k0 = (int16_t)i - (int16_t)(nh - 1); if (k0 < 0) k0 = 0;
      int16_t k1 = (int16_t)i < (int16_t)nx - 1 ? (int16_t)i : (int16_t)(nx - 1);
      for (int16_t k = k0; k <= k1; ++k)
        acc += (int32_t)x[k] * h[(int16_t)i - k];
      out[i] = acc;
    }
  }
};
constexpr DspObj Dsp{};

// ===========================================================================
// 3. DAC — millivolt
// ===========================================================================
struct DacObj {
  // ch 0 = DAC0; ch 1 = DAC1 (hanya D). ref = referensi penuh (mV), default 3300.
  inline void writeMillivolt(uint8_t ch, uint16_t mv, uint16_t ref = 3300) const {
    if (ref == 0) ref = 3300;
    uint8_t v = (uint8_t)(((uint32_t)mv * 256u) / ref);
#if LGT8_UNLOCKED_HAS_DAC1
    if (ch == 1) { lgt::DAC1Advanced::write(v); return; }
#else
    (void)ch;
#endif
    lgt::DAC0Advanced::write(v);
  }
};
constexpr DacObj Dac{};

// ===========================================================================
// 4. HDR — high-drive GPIO (P only; D -> Unsupported)
// ===========================================================================
struct HdDriveObj {
  inline lgt::Status enable(lgt::HighDrivePin p) const { return lgt::DriveStrength::high(p); }
  inline lgt::Status disable(lgt::HighDrivePin p) const { return lgt::DriveStrength::normal(p); }
  inline bool enabled(lgt::HighDrivePin p) const { return lgt::DriveStrength::isHigh(p); }
};
constexpr HdDriveObj HdDrive{};

// ===========================================================================
// 5. PWM
// ===========================================================================
struct PwmObj {
  // Timer1 fast PWM on pin 9 (PB1/OC1A). Returns the prescaler code used.
  inline uint8_t timer1Frequency(uint32_t hz) const {
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
  // Timer3 fast PWM di PF1 (QFP48 pin 33). P only; D -> Unsupported.
  inline lgt::Status timer3Frequency(uint32_t hz, uint8_t cs = 1) const {
#if LGT8_UNLOCKED_HAS_TIMER3
    if (hz == 0) return lgt::InvalidArgument;
    uint32_t top = F_CPU / ((uint32_t)cs * hz);
    if (top < 1) top = 1;
    if (top > 65535) return lgt::OutOfRange;
    lgt::Timer3::fastPwmICR((uint16_t)top, cs);
    return lgt::Ok;
#else
    (void)hz; (void)cs; return lgt::Unsupported;
#endif
  }
  // Dead-time antara channel A/B. T0 max 15 tick, T1/T3 max 255.
  inline lgt::Status deadTime0(uint8_t aTicks, uint8_t bTicks) const {
    if (aTicks > 15 || bTicks > 15) return lgt::InvalidArgument;
    lgt::Timer0Advanced::deadTime(aTicks, bTicks); return lgt::Ok;
  }
  inline lgt::Status deadTime1(uint8_t aTicks, uint8_t bTicks) const {
    lgt::Timer1Advanced::deadTime(aTicks, bTicks); return lgt::Ok;
  }
  inline lgt::Status deadTime3(uint8_t aTicks, uint8_t bTicks) const {
#if LGT8_UNLOCKED_HAS_TIMER3
    lgt::Timer3::deadTime(aTicks, bTicks); return lgt::Ok;
#else
    (void)aTicks; (void)bTicks; return lgt::Unsupported;
#endif
  }
};
constexpr PwmObj Pwm{};

// ===========================================================================
// 6. TIMER — capture + async
// ===========================================================================
struct TimerObj {
  inline void captureEdge(bool rising, bool noiseCancel = false) const {
    lgt::Timer1Advanced::captureEdge(rising, noiseCancel);
  }
  inline lgt::Status asyncExt32768(uint32_t maxSync = 0) const {
    return lgt::Timer2Async::beginExternal32768Hz(0, 0, 0, 0xff, 0xff, maxSync);
  }
  inline lgt::Status asyncInt32K(uint32_t maxSync = 0) const {
    return lgt::Timer2Async::beginInternal32KHz(0, 0, 0, 0xff, 0xff, maxSync);
  }
};
constexpr TimerObj Timer{};

// ===========================================================================
// 7. COMP — analog comparator
// ===========================================================================
struct CompObj {
  inline void enableAll() const { lgt::Comparator0::enable(true); lgt::Comparator1::enable(true); }
  inline void enable(uint8_t ch, bool yes = true) const {
    if (ch == 0) lgt::Comparator0::enable(yes); else lgt::Comparator1::enable(yes);
  }
  inline bool output(uint8_t ch) const {
    return (ch == 0) ? lgt::Comparator0::output() : lgt::Comparator1::output();
  }
  // e: 0=toggle, 1=rising, 2=falling
  inline void setEdge(uint8_t ch, uint8_t e) const {
    lgt::ComparatorEdge ce = (e == 2) ? lgt::CompareFalling : (e == 1) ? lgt::CompareRising : lgt::CompareToggle;
    if (ch == 0) lgt::Comparator0::edge(ce); else lgt::Comparator1::edge(ce);
  }
  inline void setFilter(uint8_t ch, lgt::ComparatorFilter f) const {
    if (ch == 0) lgt::Comparator0::filter(f); else lgt::Comparator1::filter(f);
  }
  inline void enableInterrupt(uint8_t ch, bool yes = true) const {
    if (ch == 0) lgt::Comparator0::interrupt(yes); else lgt::Comparator1::interrupt(yes);
  }
  inline void clearFlag(uint8_t ch) const {
    if (ch == 0) lgt::Comparator0::clearFlag(); else lgt::Comparator1::clearFlag();
  }
};
constexpr CompObj Comp{};

// ===========================================================================
// 8. USART — 9-bit + USART-SPI
// ===========================================================================
struct UsartObj {
  inline void enable9Bit(bool yes = true) const { lgt::USART0Advanced::nineBit(yes); }
  inline void write9(uint16_t v) const { lgt::USART0Advanced::write9(v); }
  inline uint16_t read9() const { return lgt::USART0Advanced::read9(); }
  inline bool available9() const { return lgt::USART0Advanced::available(); }
  inline lgt::Status spiBegin(uint32_t clock, uint8_t mode = 0, bool lsbFirst = false, bool master = true) const {
    lgt::USARTSPI0::Mode m = (lgt::USARTSPI0::Mode)mode;
    return lgt::USARTSPI0::begin(clock, m, lsbFirst, master);
  }
  inline uint8_t spiTransfer(uint8_t v) const { return lgt::USARTSPI0::transfer(v); }
  inline void spiEnd() const { lgt::USARTSPI0::end(); }
};
constexpr UsartObj Usart{};

// ===========================================================================
// 9. SPI dual (P only; D -> Unsupported)
// ===========================================================================
struct SpiDualObj {
  inline lgt::Status begin() const { return lgt::SPIDual::begin(); }
  inline lgt::SPIDualResult receive2() const { return lgt::SPIDual::receive2(); }
  inline lgt::Status receive(void *buf, size_t n) const { return lgt::SPIDual::receive(buf, n); }
  inline void end(bool restore = true) const { lgt::SPIDual::end(restore); }
};
constexpr SpiDualObj SpiDual{};

// ===========================================================================
// 10. PINMUX
// ===========================================================================
struct PinMuxObj {
  inline void ssToPB1(bool yes = true) const { lgt::PinMux::spiSsToPB1(yes); }
  inline void txdToPD6(bool yes = true) const { lgt::PinMux::txdToPD6(yes); }
  inline void rxdFromPD5(bool yes = true) const { lgt::PinMux::rxdFromPD5(yes); }
  inline void pc6AsGPIO(bool yes = true) const { lgt::PinMux::pc6AsGPIO(yes); }
  inline void pe6AsGPIO(bool yes = true) const { lgt::PinMux::pe6AsGPIO(yes); }
  inline void extOscPins(bool yes = true) const { lgt::PinMux::externalOscillatorPins(yes); }
};
constexpr PinMuxObj PinMux{};

// ===========================================================================
// 11. SYSTEM
// ===========================================================================
struct SysObj {
  inline uint32_t chipId() const {
    union { uint8_t b[4]; uint32_t w; } u;
    lgt::DeviceId id = lgt::System::guid();
    u.b[0] = id.b[0]; u.b[1] = id.b[1]; u.b[2] = id.b[2]; u.b[3] = id.b[3];
    return u.w;
  }
  inline uint8_t lastResetCause() const { return lgt::System::resetCause(); }
  inline void clearResetCause() const { lgt::System::clearResetCause(); }
  inline void softwareReset() const { lgt::System::softwareReset(); }
  inline uint16_t programLimit() const { return lgt::System::programLimit(); }
  inline bool swdDisabled() const { return lgt::System::swdDisabled(); }
};
constexpr SysObj Sys{};

// ===========================================================================
// 12. POWER
// ===========================================================================
struct PwrObj {
  inline void idle() const { lgt::Power::idle(); }
  inline void adcNoiseReduction() const { lgt::Power::adcNoiseReduction(); }
  inline void powerSave() const { lgt::Power::save(); }
  inline void powerDown0() const { lgt::Power::dps0(); }
  inline void powerDown1() const { lgt::Power::dps1(); }
  // Peripheral power domains (PRR): cut the clock of unused peripherals.
  // Safe in ANY build; re-enable before using the peripheral again.
  inline lgt::Status peripheral(lgt::Power::Peripheral p, bool on) const {
    return on ? lgt::Power::enable(p) : lgt::Power::disable(p);
  }
  // DPS2 (periodic deep sleep) exists on 328P but is Locked in the
  // recovery-safe build by design - needs a non-recovery rebuild to use
  // (raw API: lgt::Power::dps2/dps2Timer/dps2WakePins).
};
constexpr PwrObj Pwr{};

// ===========================================================================
// 13. OPA — op-amp (D only; P -> Unsupported)
// ===========================================================================
struct OpaObj {
  inline lgt::Status enable(uint8_t ch, bool yes = true) const {
#if LGT8_UNLOCKED_HAS_OPA
    if (ch == 0) lgt::OPA0::enable(yes); else lgt::OPA1::enable(yes); return lgt::Ok;
#else
    (void)ch; (void)yes; return lgt::Unsupported;
#endif
  }
};
constexpr OpaObj Opa{};

// ===========================================================================
// 14. PCINT — pin-change interrupt dengan callback
// ===========================================================================
// Forward declaration (implementasi di arduino_ext.cpp)
namespace lgt {
Status pinChangeAttach(uint8_t pin, void (*cb)(void), uint8_t mode);
void   pinChangeDetach(uint8_t pin);
}

struct PcintObj {
  inline lgt::Status attach(uint8_t pin, void (*cb)(void), uint8_t mode) const {
    return lgt::pinChangeAttach(pin, cb, mode);
  }
  inline void detach(uint8_t pin) const { lgt::pinChangeDetach(pin); }
};
constexpr PcintObj Pcint{};

// ===========================================================================
// 15. LVD
// ===========================================================================
struct LvdObj {
  inline lgt::Status setThreshold(lgt::LVDThreshold t, bool resetEnable = true, bool enable = true) const {
    return lgt::LVD::configure(t, resetEnable, enable);
  }
};
constexpr LvdObj Lvd{};

// ===========================================================================
// 16. RTC — seconds counter on the async Timer2 32 kHz clock
// ===========================================================================
// Runs while the chip is in power-save/idle (async timer keeps ticking),
// so it survives sleeps. Drift is set by the internal 32 kHz RC (~+/-1%);
// for precision use an external 32.768 kHz crystal (Timer2Async).
// Exclusive with tone()/other Timer2 use.
namespace lgt {
Status   rtcBegin();          // internal 32 kHz, CTC 1 Hz
void     rtcEnd();
uint32_t rtcSeconds();        // cli-guarded read
void     rtcSet(uint32_t s);
}
struct RtcObj {
  inline lgt::Status begin() const { return lgt::rtcBegin(); }
  inline void end() const { lgt::rtcEnd(); }
  inline uint32_t seconds() const { return lgt::rtcSeconds(); }
  inline void set(uint32_t s) const { lgt::rtcSet(s); }
};
constexpr RtcObj Rtc{};

// ===========================================================================
// Free functions mirroring the objects for plain-Arduino style code
// ===========================================================================
inline uint16_t analogReadAverage(uint8_t pin, uint8_t n = 8) { return AdcExt.readAverage(pin, n); }
inline uint8_t  pwmFrequency(uint32_t hz) { return Pwm.timer1Frequency(hz); }
inline void     dacWriteMillivolt(uint8_t ch, uint16_t mv, uint16_t ref = 3300) { Dac.writeMillivolt(ch, mv, ref); }
inline lgt::Status attachPCINT(uint8_t pin, void (*cb)(void), uint8_t mode) { return lgt::pinChangeAttach(pin, cb, mode); }
inline void       detachPCINT(uint8_t pin) { lgt::pinChangeDetach(pin); }

#endif // LGT8_UNLOCKED_ARDUINO_EXT_H
