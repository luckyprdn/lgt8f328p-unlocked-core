#ifndef LGT8_UNLOCKED_TIMER_H
#define LGT8_UNLOCKED_TIMER_H
#include "common.h"

namespace lgt {

enum Timer0Fault : uint8_t {
  T0FaultComparator0 = _BV(0), T0FaultComparator1 = _BV(1),
  T0FaultInt0 = _BV(4), T0FaultPinChange0 = _BV(5),
  T0FaultTimer2Overflow = _BV(6), T0FaultTimer1Overflow = _BV(7)
};
enum Timer13Fault : uint8_t {
  T13FaultComparator0 = _BV(0), T13FaultComparator1 = _BV(1),
  T13FaultInt1 = _BV(4), T13FaultPinChange1 = _BV(5),
  T13FaultTimer2Overflow = _BV(6), T13FaultTimer0Overflow = _BV(7)
};

struct Timer0Advanced {
  static inline Status deadTime(uint8_t oc0aTicks, uint8_t oc0bTicks) {
    if(oc0aTicks>15u||oc0bTicks>15u)return InvalidArgument;
    DTR0=(uint8_t)((oc0bTicks<<4)|oc0aTicks); TCCR0B|=_BV(DTEN0);return Ok;
  }
  static inline void disableDeadTime() { TCCR0B&=(uint8_t)~_BV(DTEN0); }
  static inline void faultSources(uint8_t mask) { TCCR0C = (uint8_t)(mask & 0xF3u); }
  static inline void protectA(bool yes=true) { yes?TCCR0A|=_BV(2):TCCR0A&=(uint8_t)~_BV(2); }
  static inline void protectB(bool yes=true) { yes?TCCR0A|=_BV(3):TCCR0A&=(uint8_t)~_BV(3); }
  static inline void highSpeed(bool yes=true) {
    if (yes) { TCKCSR|=_BV(F2XEN); nop(); TCKCSR|=_BV(TC2XS0); }
    else { TCKCSR&=(uint8_t)~_BV(TC2XS0); nop(); if(!(TCKCSR&_BV(TC2XS1)))TCKCSR&=(uint8_t)~_BV(F2XEN); }
  }
};

struct Timer1Advanced {
  static inline void deadTime(uint8_t oc1aTicks, uint8_t oc1bTicks) { DTR1L=oc1aTicks; DTR1H=oc1bTicks; TCCR1C|=_BV(DTEN1); }
  static inline void disableDeadTime() { TCCR1C&=(uint8_t)~_BV(DTEN1); }
  static inline void faultSources(uint8_t mask) { TCCR1D=(uint8_t)(mask&0xF3u); }
  static inline void protectA(bool yes=true) { yes?TCCR1C|=_BV(DOC1A):TCCR1C&=(uint8_t)~_BV(DOC1A); }
  static inline void protectB(bool yes=true) { yes?TCCR1C|=_BV(DOC1B):TCCR1C&=(uint8_t)~_BV(DOC1B); }
  static inline void captureEdge(bool rising, bool noiseCancel=false) {
    if(rising) TCCR1B|=_BV(ICES1); else TCCR1B&=(uint8_t)~_BV(ICES1);
    if(noiseCancel) TCCR1B|=_BV(ICNC1); else TCCR1B&=(uint8_t)~_BV(ICNC1);
  }
  static inline uint16_t captured() { volatile uint8_t *p=(volatile uint8_t*)&ICR1;uint8_t lo=p[0];if(TCKCSR&_BV(TC2XF1))nop();uint8_t hi=p[1];return (uint16_t)lo|((uint16_t)hi<<8); }
  static inline void captureInterrupt(bool yes=true) { yes?TIMSK1|=_BV(ICIE1):TIMSK1&=(uint8_t)~_BV(ICIE1); }
  static inline void independentPrescaler(bool yes=true) { yes?PSSR|=_BV(PSS1):PSSR&=(uint8_t)~_BV(PSS1); }
  static inline void resetPrescaler() { PSSR|=_BV(PSR1); }
  static inline void highSpeed(bool yes=true) {
    if (yes) { TCKCSR|=_BV(F2XEN); nop(); TCKCSR|=_BV(TC2XS1); }
    else { TCKCSR&=(uint8_t)~_BV(TC2XS1); nop(); if(!(TCKCSR&_BV(TC2XS0)))TCKCSR&=(uint8_t)~_BV(F2XEN); }
  }
};

struct Timer3 {
  enum Channel : uint8_t { A=0, B=1, C=2 };
#if !LGT8_UNLOCKED_HAS_TIMER3
  // LGT8F328D/E has no Timer3.  Methods are stubs that return Unsupported.
  static inline void stop() {}
  static inline void clock(uint8_t csBits){(void)csBits;}
  static inline void mode(uint8_t wgm){(void)wgm;}
  static inline void fastPwmICR(uint16_t top, uint8_t csBits=1){(void)top;(void)csBits;}
  static inline Status duty(Channel ch, uint16_t value){(void)ch;(void)value;return Unsupported;}
  static inline Status output(Channel ch, bool enable=true, bool inverted=false){(void)ch;(void)enable;(void)inverted;return Unsupported;}
  static inline void deadTime(uint8_t a, uint8_t b){(void)a;(void)b;}
  static inline void disableDeadTime(){}
  static inline void faultSources(uint8_t mask){(void)mask;}
  static inline Status protect(Channel ch,bool yes=true){(void)ch;(void)yes;return Unsupported;}
  static inline void captureEdge(bool rising,bool noiseCancel=false){(void)rising;(void)noiseCancel;}
  static inline uint16_t captured(){return 0;}
  static inline void captureInterrupt(bool yes=true){(void)yes;}
  static inline void independentPrescaler(bool yes=true){(void)yes;}
  static inline void resetPrescaler(){}
#else
  static inline void stop() { TCCR3B&=(uint8_t)~(_BV(CS32)|_BV(CS31)|_BV(CS30)); }
  static inline void clock(uint8_t csBits) { TCCR3B=(uint8_t)((TCCR3B&~0x07u)|(csBits&0x07u)); }
  static inline void mode(uint8_t wgm) {
    TCCR3A=(uint8_t)((TCCR3A&~0x03u)|(wgm&0x03u));
    TCCR3B=(uint8_t)((TCCR3B&~(_BV(WGM33)|_BV(WGM32)))|((wgm&0x0Cu)<<1));
  }
  static inline void fastPwmICR(uint16_t top, uint8_t csBits=1) { mode(14); ICR3=top; clock(csBits); }
  static inline Status duty(Channel ch, uint16_t value) { if((uint8_t)ch>2u)return InvalidArgument;if(ch==A)OCR3A=value; else if(ch==B)OCR3B=value; else OCR3C=value;return Ok; }
  static inline Status output(Channel ch, bool enable=true, bool inverted=false) {
    if((uint8_t)ch>2u)return InvalidArgument;
    uint8_t shift=(ch==A)?6:((ch==B)?4:2); uint8_t mask=(uint8_t)(3u<<shift);
    uint8_t v=TCCR3A&~mask; if(enable)v|=(uint8_t)((inverted?3u:2u)<<shift); TCCR3A=v;return Ok;
  }
  static inline void deadTime(uint8_t oc3aTicks, uint8_t oc3bTicks) { DTR3L=oc3aTicks; DTR3H=oc3bTicks; TCCR3C|=_BV(DTEN3); }
  static inline void disableDeadTime(){TCCR3C&=(uint8_t)~_BV(DTEN3);}
  static inline void faultSources(uint8_t mask){TCCR3D=(uint8_t)(mask&0xF3u);}
  static inline Status protect(Channel ch,bool yes=true){if((uint8_t)ch>2u)return InvalidArgument;uint8_t b=(ch==A)?DOC30:((ch==B)?DOC31:DOC32);yes?TCCR3C|=_BV(b):TCCR3C&=(uint8_t)~_BV(b);return Ok;}
  static inline void captureEdge(bool rising,bool noiseCancel=false){rising?TCCR3B|=_BV(ICES3):TCCR3B&=(uint8_t)~_BV(ICES3);noiseCancel?TCCR3B|=_BV(ICNC3):TCCR3B&=(uint8_t)~_BV(ICNC3);}
  static inline uint16_t captured(){return ICR3;}
  static inline void captureInterrupt(bool yes=true){yes?TIMSK3|=_BV(ICIE3):TIMSK3&=(uint8_t)~_BV(ICIE3);}
  static inline void independentPrescaler(bool yes=true){yes?PSSR|=_BV(PSS3):PSSR&=(uint8_t)~_BV(PSS3);}
  static inline void resetPrescaler(){PSSR|=_BV(PSR3);}
#endif
};

struct Timer2Async {
  enum Source : uint8_t { External32768Hz = 0, Internal32KHz = 1 };

  static inline uint8_t updateMask() {
    uint8_t m=0;
#ifdef TCN2UB
    m|=_BV(TCN2UB);
#endif
#ifdef OCR2AUB
    m|=_BV(OCR2AUB);
#endif
#ifdef OCR2BUB
    m|=_BV(OCR2BUB);
#endif
#ifdef TCR2AUB
    m|=_BV(TCR2AUB);
#endif
#ifdef TCR2BUB
    m|=_BV(TCR2BUB);
#endif
    return m;
  }

  static inline bool updating() { return (ASSR & updateMask()) != 0; }

  // maxSpins == 0 deliberately means wait forever, matching normal Arduino
  // register semantics.  A non-zero budget lets production code fail closed
  // when an external 32.768-kHz crystal is absent or has not started.
  static inline Status waitSync(uint32_t maxSpins=0) {
    if(maxSpins==0){while(updating()){} return Ok;}
    while(updating() && maxSpins){--maxSpins;}
    return updating()?NotReady:Ok;
  }

  static inline Status begin(Source source, uint8_t tccr2a, uint8_t tccr2b,
                             uint8_t counter=0, uint8_t compareA=0xff,
                             uint8_t compareB=0xff, uint32_t maxSyncSpins=0) {
    // Databook safe transition sequence: mask TC2 interrupts, select the
    // asynchronous source, rewrite all synchronised registers, wait for the
    // update-busy flags, clear stale flags, then restore the interrupt mask.
    uint8_t s=SREG; cli();
    uint8_t oldMask=TIMSK2;
    TIMSK2=(uint8_t)(oldMask & (uint8_t)~(_BV(OCIE2A)|_BV(OCIE2B)|_BV(TOIE2)));
    uint8_t a=(uint8_t)(ASSR & (uint8_t)~(_BV(INTCK)|_BV(AS2)));
    if(source==Internal32KHz)a|=_BV(INTCK);
    a|=_BV(AS2);
    ASSR=a;
    TCNT2=counter;
    OCR2A=compareA;
    OCR2B=compareB;
    TCCR2A=tccr2a;
    TCCR2B=tccr2b;
    SREG=s;

    Status st=waitSync(maxSyncSpins);
    if(st!=Ok){TIMSK2=oldMask; return st;}
    TIFR2=(uint8_t)(_BV(OCF2A)|_BV(OCF2B)|_BV(TOV2));
    TIMSK2=oldMask;
    return Ok;
  }

  static inline Status beginExternal32768Hz(uint8_t tccr2a, uint8_t tccr2b,
                                            uint8_t counter=0, uint8_t compareA=0xff,
                                            uint8_t compareB=0xff, uint32_t maxSyncSpins=0) {
    return begin(External32768Hz,tccr2a,tccr2b,counter,compareA,compareB,maxSyncSpins);
  }
  static inline Status beginInternal32KHz(uint8_t tccr2a, uint8_t tccr2b,
                                          uint8_t counter=0, uint8_t compareA=0xff,
                                          uint8_t compareB=0xff, uint32_t maxSyncSpins=0) {
    return begin(Internal32KHz,tccr2a,tccr2b,counter,compareA,compareB,maxSyncSpins);
  }

  // Compatibility helper retained for sketches that already used the first
  // Unlocked preview.  It selects the external 32.768-kHz source only; callers
  // that change Timer2 registers should prefer beginExternal32768Hz().
  static inline void enable32768Hz() {
    ASSR=(uint8_t)((ASSR & (uint8_t)~_BV(INTCK)) | _BV(AS2));
  }
  static inline void enableInternal32KHz() { ASSR|=(uint8_t)(_BV(INTCK)|_BV(AS2)); }
  static inline void disable() {
    uint8_t s=SREG; cli();
    uint8_t oldMask=TIMSK2;
    TIMSK2=(uint8_t)(oldMask & (uint8_t)~(_BV(OCIE2A)|_BV(OCIE2B)|_BV(TOIE2)));
    ASSR&=(uint8_t)~_BV(AS2);
    SREG=s;
    (void)waitSync();
    TIFR2=(uint8_t)(_BV(OCF2A)|_BV(OCF2B)|_BV(TOV2));
    TIMSK2=oldMask;
  }
};

}
#endif
