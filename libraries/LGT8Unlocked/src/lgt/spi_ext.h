#ifndef LGT8_UNLOCKED_SPI_EXT_H
#define LGT8_UNLOCKED_SPI_EXT_H
#include "common.h"
#include <SPI.h>
namespace lgt {
struct SPIDualResult{uint8_t first;uint8_t second;};
struct SPIDual {
  static inline Status begin(){
    if(!(SPCR&_BV(MSTR)) || !(SPCR&_BV(SPE)))return Unsupported;
    // Dual mode is master receive-only: both serial data pins are inputs.
    DDRB&=(uint8_t)~(_BV(DDB3)|_BV(DDB4));
    // Start from an empty FIFO once.  Do not reset SPFR after each transfer:
    // doing so would discard any data queued by a pipelined caller.
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
    // The LGT dual-input engine receives two complete bytes for one SPDR
    // write (two data bits per SPCK).  The databook explicitly requires two
    // successive SPDR reads after SPIF is asserted.
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
};
}
#endif
