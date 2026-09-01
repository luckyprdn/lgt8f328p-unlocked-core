#ifndef LGT8_UNLOCKED_UDSC_H
#define LGT8_UNLOCKED_UDSC_H
#include "common.h"
#include <stdint.h>

namespace lgt { namespace dsp {

// LGT8FX8P Programming Manual v1.0.5, uDSC IR table.
// S (bit 5 or bit 0, depending on the group) selects signed semantics.
enum RawOpcode : uint8_t {
  OP_ADD_U          = 0x05,
  OP_ADD_S          = 0x25,
  OP_SUB_U          = 0x01,
  OP_SUB_S          = 0x21,
  OP_LOAD_Y         = 0x1D,
  OP_NEG_Y_U        = 0x19,
  OP_NEG_Y_S        = 0x39,
  OP_ACC_Y_U        = 0x17,
  OP_ACC_Y_S        = 0x37,
  OP_SUBACC_Y_U     = 0x13,
  OP_SUBACC_Y_S     = 0x33,

  OP_CLEAR          = 0x80,
  OP_NEG_U          = 0x84,
  OP_NEG_S          = 0x85,
  OP_SQUARE_X_U     = 0x88,
  OP_SQUARE_X_S     = 0x89,
  OP_SQUARE_Y_U     = 0x8A,
  OP_SQUARE_Y_S     = 0x8B,
  OP_ABS_U          = 0xA0,
  OP_ABS_S          = 0xA1,
  OP_DIV            = 0xB0,
  OP_DIVMOD         = 0xB1
};

static inline void enable(bool yes=true){yes?DSCR|=_BV(DSUEN):DSCR&=(uint8_t)~_BV(DSUEN);}
static inline void mappingIO(bool yes=true){yes?DSCR|=_BV(DSMM):DSCR&=(uint8_t)~_BV(DSMM);}
static inline uint8_t flags(){return (uint8_t)(DSCR&0x37u);}
static inline bool negativeFlag(){return (DSCR&_BV(2))!=0;}
static inline bool zeroFlag(){return (DSCR&_BV(1))!=0;}
static inline bool carryFlag(){return (DSCR&_BV(0))!=0;}
static inline bool divideDone(){return (DSCR&_BV(DSD1))!=0;}
static inline bool divideByZero(){return (DSCR&_BV(DSD0))!=0;}
static inline void command(uint8_t opcode){DSIR=opcode;}
static inline void waitDivision(){while(!divideDone()){} }

// The LGT8XM extension treats one IN/OUT targeting DSDX/DSDY/DSAL/DSAH/DSSD
// as a 16-bit transfer using the selected even register pair.  The "w"
// constraint deliberately forces a legal even upper-register pair.
static inline void setX(uint16_t v){__asm__ __volatile__("out %0,%A1"::"I"(_SFR_IO_ADDR(DSDX)),"w"(v):"memory");}
static inline void setY(uint16_t v){__asm__ __volatile__("out %0,%A1"::"I"(_SFR_IO_ADDR(DSDY)),"w"(v):"memory");}
static inline uint16_t getX(){uint16_t v;__asm__ __volatile__("in %A0,%1":"=w"(v):"I"(_SFR_IO_ADDR(DSDX)):"memory");return v;}
static inline uint16_t getY(){uint16_t v;__asm__ __volatile__("in %A0,%1":"=w"(v):"I"(_SFR_IO_ADDR(DSDY)):"memory");return v;}
static inline uint16_t low(){uint16_t v;__asm__ __volatile__("in %A0,%1":"=w"(v):"I"(_SFR_IO_ADDR(DSAL)):"memory");return v;}
static inline uint16_t high(){uint16_t v;__asm__ __volatile__("in %A0,%1":"=w"(v):"I"(_SFR_IO_ADDR(DSAH)):"memory");return v;}
static inline uint16_t saturated(){uint16_t v;__asm__ __volatile__("in %A0,%1":"=w"(v):"I"(_SFR_IO_ADDR(DSSD)):"memory");return v;}
static inline uint32_t accumulator(){uint32_t v=(uint32_t)low();v|=((uint32_t)high()<<16);return v;}
static inline void setAccumulator(uint32_t v){uint16_t lo=(uint16_t)v,hi=(uint16_t)(v>>16);__asm__ __volatile__("out %0,%A1"::"I"(_SFR_IO_ADDR(DSAL)),"w"(lo):"memory");__asm__ __volatile__("out %0,%A1"::"I"(_SFR_IO_ADDR(DSAH)),"w"(hi):"memory");}

static inline void clear(){enable();command(OP_CLEAR);}
static inline uint32_t add(uint16_t x,uint16_t y,bool sign=false){enable();setX(x);setY(y);command(sign?OP_ADD_S:OP_ADD_U);return accumulator();}
static inline uint32_t sub(uint16_t x,uint16_t y,bool sign=false){enable();setX(x);setY(y);command(sign?OP_SUB_S:OP_SUB_U);return accumulator();}
static inline uint32_t loadY(uint16_t y){enable();setY(y);command(OP_LOAD_Y);return accumulator();}
static inline uint32_t negY(uint16_t y,bool sign=false){enable();setY(y);command(sign?OP_NEG_Y_S:OP_NEG_Y_U);return accumulator();}
static inline uint32_t accumulateY(uint16_t y,bool sign=false){enable();setY(y);command(sign?OP_ACC_Y_S:OP_ACC_Y_U);return accumulator();}
static inline uint32_t subtractY(uint16_t y,bool sign=false){enable();setY(y);command(sign?OP_SUBACC_Y_S:OP_SUBACC_Y_U);return accumulator();}

// MAC/MSC table: 01 S1 S0 b3 b2 b1 S.  S1=DX signed, S0=DY signed.
static inline uint8_t mulOpcode(bool xSigned,bool ySigned,bool negative=false,bool half=false){
  uint8_t op=0x40u;
  if(xSigned) op|=0x20u;
  if(ySigned) op|=0x10u;
  if(half) op|=0x08u;
  if(!negative) op|=0x04u;
  return op;
}
static inline uint8_t macOpcode(bool xSigned,bool ySigned,bool subtract=false,bool half=false,bool signedAcc=false){
  uint8_t op=0x40u;
  if(xSigned) op|=0x20u;
  if(ySigned) op|=0x10u;
  if(half) op|=0x08u;
  if(!subtract) op|=0x04u;
  op|=0x02u;
  if(signedAcc) op|=0x01u;
  return op;
}
static inline uint32_t mul(uint16_t x,uint16_t y,bool xSigned=false,bool ySigned=false){enable();setX(x);setY(y);command(mulOpcode(xSigned,ySigned));return accumulator();}
static inline uint32_t mulNegative(uint16_t x,uint16_t y,bool xSigned=false,bool ySigned=false){enable();setX(x);setY(y);command(mulOpcode(xSigned,ySigned,true,false));return accumulator();}
static inline uint32_t mulHalf(uint16_t x,uint16_t y,bool xSigned=false,bool ySigned=false){enable();setX(x);setY(y);command(mulOpcode(xSigned,ySigned,false,true));return accumulator();}
static inline uint32_t mulNegativeHalf(uint16_t x,uint16_t y,bool xSigned=false,bool ySigned=false){enable();setX(x);setY(y);command(mulOpcode(xSigned,ySigned,true,true));return accumulator();}
static inline uint32_t mac(uint16_t x,uint16_t y,bool xSigned=false,bool ySigned=false,bool signedAcc=false){enable();setX(x);setY(y);command(macOpcode(xSigned,ySigned,false,false,signedAcc));return accumulator();}
static inline uint32_t macHalf(uint16_t x,uint16_t y,bool xSigned=false,bool ySigned=false,bool signedAcc=false){enable();setX(x);setY(y);command(macOpcode(xSigned,ySigned,false,true,signedAcc));return accumulator();}
static inline uint32_t msc(uint16_t x,uint16_t y,bool xSigned=false,bool ySigned=false,bool signedAcc=false){enable();setX(x);setY(y);command(macOpcode(xSigned,ySigned,true,false,signedAcc));return accumulator();}
static inline uint32_t mscHalf(uint16_t x,uint16_t y,bool xSigned=false,bool ySigned=false,bool signedAcc=false){enable();setX(x);setY(y);command(macOpcode(xSigned,ySigned,true,true,signedAcc));return accumulator();}

static inline uint32_t squareX(uint16_t x,bool sign=false){enable();setX(x);command(sign?OP_SQUARE_X_S:OP_SQUARE_X_U);return accumulator();}
static inline uint32_t squareY(uint16_t y,bool sign=false){enable();setY(y);command(sign?OP_SQUARE_Y_S:OP_SQUARE_Y_U);return accumulator();}
static inline uint32_t abs32(bool sign=true){enable();command(sign?OP_ABS_S:OP_ABS_U);return accumulator();}
static inline uint32_t neg32(bool sign=true){enable();command(sign?OP_NEG_S:OP_NEG_U);return accumulator();}
static inline uint32_t shiftLeft(uint8_t n){enable();command((uint8_t)(0xC0u|(n&0x0fu)));return accumulator();}
static inline uint32_t shiftRight(uint8_t n,bool arithmetic=true){enable();command((uint8_t)(0xD0u|(arithmetic?0x20u:0u)|(n&0x0fu)));return accumulator();}

struct DivResult{uint32_t quotient;uint16_t remainder;bool divideByZero;};
static inline uint32_t divide(uint32_t dividend,uint16_t divisor,bool *zero=0){enable();setAccumulator(dividend);setY(divisor);command(OP_DIV);waitDivision();if(zero)*zero=divideByZero();return accumulator();}
static inline DivResult divmod(uint32_t dividend,uint16_t divisor){enable();setAccumulator(dividend);setY(divisor);command(OP_DIVMOD);waitDivision();DivResult r={accumulator(),getY(),divideByZero()};return r;}

static inline bool validSram16(const void *p){uintptr_t a=(uintptr_t)p;return a>=0x0100u&&a<=0x08FEu&&!(a&1u);}
static inline uintptr_t alias16Address(const void *p){return (uintptr_t)p+0x2000u;}
static inline volatile uint16_t *alias16(void *p){return validSram16(p)?(volatile uint16_t*)alias16Address(p):(volatile uint16_t*)0;}
static inline uint16_t load16(const uint16_t *p){if(!validSram16(p))return 0;enable();mappingIO(false);const void *z=(const void*)alias16Address(p);__asm__ __volatile__("ld r0,Z"::"z"(z):"r0","memory");return getX();}
static inline Status store16(uint16_t *p,uint16_t value){if(!validSram16(p))return OutOfRange;enable();mappingIO(false);setX(value);void *z=(void*)alias16Address(p);__asm__ __volatile__("st Z,r0"::"z"(z):"r0","memory");return Ok;}

// Convenience algorithm. Kept intentionally simple; release documentation marks
// algorithm-level acceleration as benchmark-gated until measured on real silicon.
static inline int32_t dotProduct(const int16_t *a,const int16_t *b,uint16_t count){
  if(count==0u)return 0;
  if(a==0||b==0)return 0;
  enable();command(OP_CLEAR);
  for(uint16_t i=0;i<count;++i){setX((uint16_t)a[i]);setY((uint16_t)b[i]);command(macOpcode(true,true,false,false,true));}
  return (int32_t)accumulator();
}

} }
#endif
