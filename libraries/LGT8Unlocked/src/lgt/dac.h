#ifndef LGT8_UNLOCKED_DAC_H
#define LGT8_UNLOCKED_DAC_H
#include "common.h"
namespace lgt {
enum DACReference:uint8_t{DACVcc=0,DACExternal=1,DACInternal=2,DACReferenceOff=3};
struct DAC0Advanced {
  static inline void enable(bool yes=true){yes?DACON|=_BV(DACEN):DACON&=(uint8_t)~_BV(DACEN);}
  static inline void output(bool yes=true){yes?DACON|=_BV(DAOE):DACON&=(uint8_t)~_BV(DAOE);}
  static inline void reference(DACReference r){DACON=(uint8_t)((DACON&~3u)|((uint8_t)r&3u));}
  static inline void write(uint8_t value){DALR=value;}
  static inline uint8_t read(){return DALR;}
  static inline void begin(DACReference r=DACVcc,bool externalOutput=true){reference(r);enable(r!=DACReferenceOff);output(externalOutput&&r!=DACReferenceOff);}
  static inline void end(){output(false);enable(false);reference(DACReferenceOff);}
};
}
#endif
