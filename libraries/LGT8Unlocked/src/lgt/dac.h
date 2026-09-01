#ifndef LGT8_UNLOCKED_DAC_H
#define LGT8_UNLOCKED_DAC_H
#include "common.h"
namespace lgt {
enum DACReference:uint8_t{DACVcc=0,DACExternal=1,DACInternal=2,DACReferenceOff=3};

#if !LGT8_UNLOCKED_HAS_DAC1
// ---- LGT8F328P: single DAC on PD4, controlled via DACON/DALR ----
struct DAC0Advanced {
  static inline void enable(bool yes=true){yes?DACON|=_BV(DACEN):DACON&=(uint8_t)~_BV(DACEN);}
  static inline void output(bool yes=true){yes?DACON|=_BV(DAOE):DACON&=(uint8_t)~_BV(DAOE);}
  static inline void reference(DACReference r){DACON=(uint8_t)((DACON&~3u)|((uint8_t)r&3u));}
  static inline void write(uint8_t value){DALR=value;}
  static inline uint8_t read(){return DALR;}
  static inline void begin(DACReference r=DACVcc,bool externalOutput=true){reference(r);enable(r!=DACReferenceOff);output(externalOutput&&r!=DACReferenceOff);}
  static inline void end(){output(false);enable(false);reference(DACReferenceOff);}
};
#else
// ---- LGT8F328D/E: DAC0/DAC1 output level via DAL0/DAL1, enable via IOCR ----
// DAC references are fixed to the internal IVREF on 328D/E; the reference
// enum is accepted for API compatibility and mapped to VCC-mode behaviour.
struct DAC0Advanced {
  static inline void enable(bool yes=true){uint8_t v=IOCR;yes?v|=_BV(DACEN0):v&=(uint8_t)~_BV(DACEN0);detail::iocrWrite(v);}
  static inline void output(bool yes=true){enable(yes);}   // no separate DAOE on 328D
  static inline void reference(DACReference r){(void)r;}   // fixed internal ref
  static inline void write(uint8_t value){DAL0=value;}
  static inline uint8_t read(){return DAL0;}
  static inline void begin(DACReference r=DACVcc,bool externalOutput=true){(void)r;output(externalOutput);}
  static inline void end(){output(false);}
};

// DAC1 exists only on LGT8F328D/E (second output channel).
struct DAC1Advanced {
  static inline void enable(bool yes=true){uint8_t v=IOCR;yes?v|=_BV(DACEN1):v&=(uint8_t)~_BV(DACEN1);detail::iocrWrite(v);}
  static inline void output(bool yes=true){enable(yes);}
  static inline void reference(DACReference r){(void)r;}
  static inline void write(uint8_t value){DAL1=value;}
  static inline uint8_t read(){return DAL1;}
  static inline void begin(DACReference r=DACVcc,bool externalOutput=true){(void)r;output(externalOutput);}
  static inline void end(){output(false);}
};
#endif

}
#endif