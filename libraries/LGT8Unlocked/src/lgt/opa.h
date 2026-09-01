#ifndef LGT8_UNLOCKED_OPA_H
#define LGT8_UNLOCKED_OPA_H
#include "common.h"

namespace lgt {

// OPA0 and OPA1 are operational-amplifier blocks present only on
// LGT8F328D/E silicon.  Each OPA contains a 2-channel input mux,
// channel-switch timer, and configurable output that can feed ADC
// and/or analog comparator inputs.

enum OPAChannel : uint8_t { OPACh0=0, OPACh1=1, OPABoth=3 };
enum OPATimerPeriod : uint8_t { OPATimerOff=0 };

struct OPA0 {
  // Enable / disable the op-amp block.
  static inline void enable(bool yes=true) {
    uint8_t v=OP0CRA;
    if(yes) v|=_BV(OPAEN); else v&=(uint8_t)~_BV(OPAEN);
    OP0CRA=v;
  }
  static inline bool enabled() { return (OP0CRA & _BV(OPAEN)) != 0; }

  // Select active channel(s): CH0 only, CH1 only, or both (auto-switch).
  static inline void channel(OPAChannel ch) {
    uint8_t v=OP0CRA & (uint8_t)~(_BV(CH0EN)|_BV(CH1EN));
    if(ch!=OPACh1) v|=_BV(CH0EN);   // Ch0 on unless Ch1-only
    if(ch!=OPACh0) v|=_BV(CH1EN);   // Ch1 on unless Ch0-only
    OP0CRA=v;
  }
  static inline void invertCh0(bool yes=true){if(yes)OP0CRA|=_BV(CH0IM);else OP0CRA&=(uint8_t)~_BV(CH0IM);}
  static inline void invertCh1(bool yes=true){if(yes)OP0CRA|=_BV(CH1IM);else OP0CRA&=(uint8_t)~_BV(CH1IM);}
  static inline uint8_t activeChannel(){return (OP0CRA&_BV(ACCH))?1u:0u;}
  static inline void switchChannel(){OP0CRA|=_BV(ACCH);}

  // Timer / channel-switch period (0 = off, timer runs in auto-switch).
  static inline void enableTimer(bool yes=true){if(yes)OP0CRB|=_BV(OPTEN);else OP0CRB&=(uint8_t)~_BV(OPTEN);}
  static inline void setPeriod(uint8_t cycles){OP0TCNT=cycles;}
  static inline void setHoldTime(uint8_t t){OP0CRB=(uint8_t)((OP0CRB&0x80u)|(t&0x7Fu));}

  // Comparator output filter enable (connects AC0 filter to OPA0).
  // The filter width is configured via AFTCNT0.
  static inline void comparatorFilter(bool yes=true){if(yes)OP0CRA|=_BV(ACFEN);else OP0CRA&=(uint8_t)~_BV(ACFEN);}
};

struct OPA1 {
  static inline void enable(bool yes=true) {
    uint8_t v=OP1CRA;
    if(yes) v|=_BV(OPAEN); else v&=(uint8_t)~_BV(OPAEN);
    OP1CRA=v;
  }
  static inline bool enabled() { return (OP1CRA & _BV(OPAEN)) != 0; }
  static inline void channel(OPAChannel ch) {
    uint8_t v=OP1CRA & (uint8_t)~(_BV(CH0EN)|_BV(CH1EN));
    if(ch!=OPACh1) v|=_BV(CH0EN);
    if(ch!=OPACh0) v|=_BV(CH1EN);
    OP1CRA=v;
  }
  static inline void invertCh0(bool yes=true){if(yes)OP1CRA|=_BV(CH0IM);else OP1CRA&=(uint8_t)~_BV(CH0IM);}
  static inline void invertCh1(bool yes=true){if(yes)OP1CRA|=_BV(CH1IM);else OP1CRA&=(uint8_t)~_BV(CH1IM);}
  static inline uint8_t activeChannel(){return (OP1CRA&_BV(ACCH))?1u:0u;}
  static inline void switchChannel(){OP1CRA|=_BV(ACCH);}
  static inline void enableTimer(bool yes=true){if(yes)OP1CRB|=_BV(OPTEN);else OP1CRB&=(uint8_t)~_BV(OPTEN);}
  static inline void setPeriod(uint8_t cycles){OP1TCNT=cycles;}
  static inline void setHoldTime(uint8_t t){OP1CRB=(uint8_t)((OP1CRB&0x80u)|(t&0x7Fu));}
  static inline void comparatorFilter(bool yes=true){if(yes)OP1CRA|=_BV(ACFEN);else OP1CRA&=(uint8_t)~_BV(ACFEN);}
};

}
#endif