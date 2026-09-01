#ifndef LGT8_UNLOCKED_COMPARATOR_H
#define LGT8_UNLOCKED_COMPARATOR_H
#include "common.h"

namespace lgt {

enum ComparatorEdge:uint8_t{CompareToggle=0,CompareFalling=2,CompareRising=3};
enum ComparatorFilter:uint8_t{FilterOff=0,Filter32us=1,Filter64us=2,Filter96us=3};
enum AC0Positive:uint8_t{AC0P=0,ACXP0=1,AC0DAC=2,AC0PositiveOff=3};
enum AC0Negative:uint8_t{ACXN0=0,AC0ADCMux=1,AC0Differential=2,AC0NegativeOff=3};
enum AC1Positive:uint8_t{ACXP1=0,AC1P=1,AC1DAC=2,AC1PositiveOff=3};
enum AC1Negative:uint8_t{ACXN1=0,AC1N=1,AC1VDO=2,AC1Differential=3};

namespace detail {
static inline void c0Write(uint8_t v){ C0SR=(uint8_t)(v & (uint8_t)~_BV(C0I)); }
static inline void c1Write(uint8_t v){ C1SR=(uint8_t)(v & (uint8_t)~_BV(C1I)); }
}

struct Comparator0 {
  static inline void enable(bool yes=true){uint8_t v=C0SR;yes?v&=(uint8_t)~_BV(C0D):v|=_BV(C0D);detail::c0Write(v);}
  static inline void positive(AC0Positive src){if((uint8_t)src>3u)return;uint8_t v=C0SR;if((uint8_t)src&2u)v|=_BV(C0BG);else v&=(uint8_t)~_BV(C0BG);detail::c0Write(v);if((uint8_t)src&1u)C0XR|=_BV(C0PS0);else C0XR&=(uint8_t)~_BV(C0PS0);}
  static inline void negative(AC0Negative src){if((uint8_t)src>3u)return;ADCSRB=(uint8_t)((ADCSRB&~(_BV(ACME01)|_BV(ACME00)))|(((uint8_t)src&3u)<<6));}
  static inline bool output(){return (C0SR&_BV(C0O))!=0;}
  static inline void edge(ComparatorEdge e){if((uint8_t)e>3u || (uint8_t)e==1u)return;detail::c0Write((uint8_t)((C0SR&~3u)|((uint8_t)e&3u)));}
  static inline void interrupt(bool yes=true){uint8_t v=C0SR;yes?v|=_BV(C0IE):v&=(uint8_t)~_BV(C0IE);detail::c0Write(v);}
  static inline void clearFlag(){C0SR=(uint8_t)((C0SR&~_BV(C0I))|_BV(C0I));}
  static inline void routeToTimer1Capture(bool yes=true){uint8_t v=C0SR;yes?v|=_BV(C0IC):v&=(uint8_t)~_BV(C0IC);detail::c0Write(v);}
  static inline void outputPin(bool yes=true){yes?C0XR|=_BV(C0OE):C0XR&=(uint8_t)~_BV(C0OE);}
  static inline void hysteresis(bool yes=true){yes?C0XR|=_BV(C0HSYE):C0XR&=(uint8_t)~_BV(C0HSYE);}
  static inline void wake(bool yes=true){yes?C0XR|=_BV(C0WKE):C0XR&=(uint8_t)~_BV(C0WKE);}
  static inline void filter(ComparatorFilter f){if((uint8_t)f>3u)return;C0XR=(uint8_t)((C0XR&~7u)|((f==FilterOff)?0u:(_BV(C0FEN)|((uint8_t)f&3u))));}
};

struct Comparator1 {
  static inline void enable(bool yes=true){uint8_t v=C1SR;yes?v&=(uint8_t)~_BV(C1D):v|=_BV(C1D);detail::c1Write(v);}
  static inline void positive(AC1Positive src){if((uint8_t)src>3u)return;uint8_t v=C1SR;if((uint8_t)src&2u)v|=_BV(C1BG);else v&=(uint8_t)~_BV(C1BG);detail::c1Write(v);if((uint8_t)src&1u)C1XR|=_BV(C1PS0);else C1XR&=(uint8_t)~_BV(C1PS0);}
  static inline void negative(AC1Negative src){if((uint8_t)src>3u)return;ADCSRB=(uint8_t)((ADCSRB&~(_BV(ACME11)|_BV(ACME10)))|(((uint8_t)src&3u)<<4));}
  static inline bool output(){return (C1SR&_BV(C1O))!=0;}
  static inline void edge(ComparatorEdge e){if((uint8_t)e>3u || (uint8_t)e==1u)return;detail::c1Write((uint8_t)((C1SR&~3u)|((uint8_t)e&3u)));}
  static inline void interrupt(bool yes=true){uint8_t v=C1SR;yes?v|=_BV(C1IE):v&=(uint8_t)~_BV(C1IE);detail::c1Write(v);}
  static inline void clearFlag(){C1SR=(uint8_t)((C1SR&~_BV(C1I))|_BV(C1I));}
  static inline void routeToTimerCapture(bool yes=true){uint8_t v=C1SR;yes?v|=_BV(C1IC):v&=(uint8_t)~_BV(C1IC);detail::c1Write(v);}
  static inline void outputPin(bool yes=true){yes?C1XR|=_BV(C1OE):C1XR&=(uint8_t)~_BV(C1OE);}
  static inline void hysteresis(bool yes=true){yes?C1XR|=_BV(C1HSYE):C1XR&=(uint8_t)~_BV(C1HSYE);}
  static inline void wake(bool yes=true){yes?C1XR|=_BV(C1WKE):C1XR&=(uint8_t)~_BV(C1WKE);}
  static inline void filter(ComparatorFilter f){if((uint8_t)f>3u)return;C1XR=(uint8_t)((C1XR&~7u)|((f==FilterOff)?0u:(_BV(C1FEN)|((uint8_t)f&3u))));}
};

}
#endif
