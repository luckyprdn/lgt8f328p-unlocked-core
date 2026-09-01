#ifndef LGT8_UNLOCKED_SPI_EXT_H
#define LGT8_UNLOCKED_SPI_EXT_H
#include "common.h"
#include <SPI.h>

namespace lgt {

struct SPIDualResult{uint8_t first;uint8_t second;};
struct SPIDual {
#if LGT8_UNLOCKED_HAS_SPI_DUAL
  static inline Status begin(){
    if(!(SPCR&_BV(MSTR)) || !(SPCR&_BV(SPE)))return Unsupported;
    DDRB&=(uint8_t)~(_BV(DDB3)|_BV(DDB4));
    SPFR=(uint8_t)(_BV(RDEMPT)|_BV(WREMPT));
    SPSR|=_BV(DUAL);
    return Ok;
  }
  static inline void end(bool restoreMasterMosi=true){
    SPSR&=(uint8_t)~_BV(DUAL);
    if(restoreMasterMosi && (SPCR&_BV(MSTR))){DDRB|=_BV(DDB3);DDRB&=(uint8_t)~_BV(DDB4);}
  }
  static inline Status receive2(SPIDualResult &r){
    if(!(SPCR&_BV(MSTR)) || !(SPCR&_BV(SPE)) || !(SPSR&_BV(DUAL)))return NotReady;
    SPDR=0;
    while(!(SPSR&_BV(SPIF))){}
    r.first=SPDR;
    r.second=SPDR;
    return Ok;
  }
  static inline SPIDualResult receive2(){
    SPIDualResult r={0,0};
    (void)receive2(r);
    return r;
  }
  static inline Status receive(void *buffer,size_t bytes){
    if(bytes==0)return Ok;
    if(buffer==0)return InvalidArgument;
    uint8_t *p=(uint8_t*)buffer;
    while(bytes){
      SPIDualResult r;Status st=receive2(r);if(st!=Ok)return st;
      *p++=r.first;--bytes;
      if(bytes){*p++=r.second;--bytes;}
    }
    return Ok;
  }
#else
  // LGT8F328D/E has no dual-SPI (SPFR) hardware.
  static inline Status begin(){return Unsupported;}
  static inline void end(bool restore=true){(void)restore;}
  static inline Status receive2(SPIDualResult &r){(void)r;return Unsupported;}
  static inline SPIDualResult receive2(){SPIDualResult r={0,0};return r;}
  static inline Status receive(void *buffer,size_t bytes){(void)buffer;(void)bytes;return Unsupported;}
#endif
};

}
#endif