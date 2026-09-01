#ifndef LGT8_UNLOCKED_PINMUX_H
#define LGT8_UNLOCKED_PINMUX_H
#include "common.h"

namespace lgt {

struct PinMux {
  // PMX0 routes
  static inline Status oc1bToPF4(bool yes=true) {
#if !defined(__LGT8FX8P48__)
    (void)yes; return Unsupported;
#else
    uint8_t v=PMX0; yes?v|=_BV(6):v&=(uint8_t)~_BV(6); detail::pmx0Write(v); return Ok;
#endif
  }
  static inline Status oc1aToPF5(bool yes=true) {
#if !defined(__LGT8FX8P48__)
    (void)yes; return Unsupported;
#else
    uint8_t v=PMX0; yes?v|=_BV(5):v&=(uint8_t)~_BV(5); detail::pmx0Write(v); return Ok;
#endif
  }
  static inline Status oc0bToPF3(bool yes=true) {
#if !defined(__LGT8FX8P48__)
    (void)yes; return Unsupported;
#else
    uint8_t v=PMX0; yes?v|=_BV(4):v&=(uint8_t)~_BV(4); detail::pmx0Write(v); return Ok;
#endif
  }
  static inline void spiSsToPB1(bool yes=true) { uint8_t v=PMX0; yes?v|=_BV(2):v&=(uint8_t)~_BV(2); detail::pmx0Write(v); }
  static inline void txdToPD6(bool yes=true) { uint8_t v=PMX0; yes?v|=_BV(1):v&=(uint8_t)~_BV(1); detail::pmx0Write(v); }
  static inline void rxdFromPD5(bool yes=true) { uint8_t v=PMX0; yes?v|=_BV(0):v&=(uint8_t)~_BV(0); detail::pmx0Write(v); }

  enum OC0ARoute : uint8_t { OC0A_PD6=0, OC0A_PE4=1, OC0A_PC0=2, OC0A_PE4_PC0=3 };
  static inline Status oc0a(OC0ARoute route) {
    if((uint8_t)route>3u)return InvalidArgument;
    uint8_t v=PMX0;
    if ((uint8_t)route & 0x2) v|=_BV(3); else v&=(uint8_t)~_BV(3);
    detail::pmx0Write(v);
    if ((uint8_t)route & 0x1) TCCR0B|=_BV(OC0AS); else TCCR0B&=(uint8_t)~_BV(OC0AS);
    return Ok;
  }

  // PMX1 routes
  static inline Status oc3aToAC0P(bool yes=true) {
#if !defined(__LGT8FX8P48__)
    (void)yes; return Unsupported;
#else
    uint8_t v=PMX1; yes?v|=_BV(2):v&=(uint8_t)~_BV(2); detail::pmx1Write(v); return Ok;
#endif
  }
  static inline Status oc2bToPF7(bool yes=true) {
#if !defined(__LGT8FX8P48__)
    (void)yes; return Unsupported;
#else
    uint8_t v=PMX1; yes?v|=_BV(1):v&=(uint8_t)~_BV(1); detail::pmx1Write(v); return Ok;
#endif
  }
  static inline Status oc2aToPF6(bool yes=true) {
#if !defined(__LGT8FX8P48__)
    (void)yes; return Unsupported;
#else
    uint8_t v=PMX1; yes?v|=_BV(0):v&=(uint8_t)~_BV(0); detail::pmx1Write(v); return Ok;
#endif
  }

  // PMX2 pin ownership
  static inline void pc6AsGPIO(bool yes=true) { uint8_t v=PMX2; yes?v|=_BV(0):v&=(uint8_t)~_BV(0); detail::pmx2Write(v); }
  static inline void pe6AsGPIO(bool yes=true) { uint8_t v=PMX2; yes?v|=_BV(1):v&=(uint8_t)~_BV(1); detail::pmx2Write(v); }
  static inline void externalOscillatorPins(bool yes=true) { uint8_t v=PMX2; yes?v|=_BV(2):v&=(uint8_t)~_BV(2); detail::pmx2Write(v); }
};

}
#endif
