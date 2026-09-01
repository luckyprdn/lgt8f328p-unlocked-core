#ifndef LGT8_UNLOCKED_UDSC_H
#define LGT8_UNLOCKED_UDSC_H
#include "common.h"

namespace lgt {

#if LGT8_UNLOCKED_HAS_UDSC
namespace dsp {
enum RawOpcode:uint8_t{OP_ADD_U=0x05,OP_ADD_S=0x25,OP_SUB_U=0x01,OP_SUB_S=0x21,OP_LOAD_Y=0x1D,OP_NEG_Y_U=0x19,OP_NEG_Y_S=0x39,OP_ACC_Y_U=0x17,OP_ACC_Y_S=0x37,OP_SUBACC_Y_U=0x13,OP_SUBACC_Y_S=0x33,OP_CLEAR=0x80,OP_NEG_U=0x84,OP_NEG_S=0x85,OP_SQUARE_X_U=0x88,OP_SQUARE_X_S=0x89,OP_SQUARE_Y_U=0x8A,OP_SQUARE_Y_S=0x8B,OP_ABS_U=0xA0,OP_ABS_S=0xA1,OP_DIV=0xB0,OP_DIVMOD=0xB1};
static inline void enable(){DSCR|=_BV(DSUEN);}static inline void mappingIO(bool y=true){y?DSCR|=_BV(DSMM):DSCR&=~_BV(DSMM);}
static inline uint8_t flags(){return DSCR&0x37u;}static inline bool negativeFlag(){return DSCR&_BV(2);}static inline bool zeroFlag(){return DSCR&_BV(1);}static inline bool carryFlag(){return DSCR&_BV(0);}static inline bool divideDone(){return DSCR&_BV(DSD1);}static inline bool divideByZero(){return DSCR&_BV(DSD0);}
static inline void command(uint8_t o){DSIR=o;}static inline void waitDivision(){while(!divideDone()){}}
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
static inline uint32_t add(uint16_t x,uint16_t y,bool s=false){enable();setX(x);setY(y);command(s?OP_ADD_S:OP_ADD_U);return accumulator();}
static inline uint32_t sub(uint16_t x,uint16_t y,bool s=false){enable();setX(x);setY(y);command(s?OP_SUB_S:OP_SUB_U);return accumulator();}
static inline uint32_t loadY(uint16_t y){enable();setY(y);command(OP_LOAD_Y);return accumulator();}
static inline uint32_t negY(uint16_t y,bool s=false){enable();setY(y);command(s?OP_NEG_Y_S:OP_NEG_Y_U);return accumulator();}
static inline uint32_t accumulateY(uint16_t y,bool s=false){enable();setY(y);command(s?OP_ACC_Y_S:OP_ACC_Y_U);return accumulator();}
static inline uint32_t subtractY(uint16_t y,bool s=false){enable();setY(y);command(s?OP_SUBACC_Y_S:OP_SUBACC_Y_U);return accumulator();}
struct DivResult{uint32_t quotient;uint16_t remainder;bool zero;};
static inline uint8_t mulOpcode(bool xS,bool yS,bool neg=false,bool half=false){uint8_t o=0x40u;if(xS)o|=0x20u;if(yS)o|=0x10u;if(half)o|=0x08u;if(!neg)o|=0x04u;return o;}
static inline uint8_t macOpcode(bool xS,bool yS,bool sub=false,bool half=false,bool sA=false){uint8_t o=0x40u;if(xS)o|=0x20u;if(yS)o|=0x10u;if(half)o|=0x08u;if(!sub)o|=0x04u;o|=0x02u;if(sA)o|=0x01u;return o;}
static inline uint32_t mul(uint16_t x,uint16_t y,bool xS=false,bool yS=false){enable();setX(x);setY(y);command(mulOpcode(xS,yS));return accumulator();}
static inline uint32_t mulNegative(uint16_t x,uint16_t y,bool xS=false,bool yS=false){enable();setX(x);setY(y);command(mulOpcode(xS,yS,true));return accumulator();}
static inline uint32_t mulHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false){enable();setX(x);setY(y);command(mulOpcode(xS,yS,false,true));return accumulator();}
static inline uint32_t mulNegativeHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false){enable();setX(x);setY(y);command(mulOpcode(xS,yS,true,true));return accumulator();}
static inline uint32_t mac(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){enable();setX(x);setY(y);command(macOpcode(xS,yS,false,false,sA));return accumulator();}
static inline uint32_t macHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){enable();setX(x);setY(y);command(macOpcode(xS,yS,false,true,sA));return accumulator();}
static inline uint32_t msc(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){enable();setX(x);setY(y);command(macOpcode(xS,yS,true,false,sA));return accumulator();}
static inline uint32_t mscHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){enable();setX(x);setY(y);command(macOpcode(xS,yS,true,true,sA));return accumulator();}
static inline uint32_t squareX(uint16_t x,bool s=false){enable();setX(x);command(s?OP_SQUARE_X_S:OP_SQUARE_X_U);return accumulator();}
static inline uint32_t squareY(uint16_t y,bool s=false){enable();setY(y);command(s?OP_SQUARE_Y_S:OP_SQUARE_Y_U);return accumulator();}
static inline uint32_t abs32(bool s=true){enable();command(s?OP_ABS_S:OP_ABS_U);return accumulator();}
static inline uint32_t neg32(bool s=true){enable();command(s?OP_NEG_S:OP_NEG_U);return accumulator();}
static inline void shiftLeft(uint8_t n){if(n>15)return;enable();command((uint8_t)(0xC0u|(n&0x0fu)));}
static inline void shiftRight(uint8_t n,bool a=true){if(n>15)return;enable();command((uint8_t)(0xD0u|(a?0x20u:0u)|(n&0x0fu)));}
static inline bool validSram16(const void *p){uintptr_t a=(uintptr_t)p;return a>=0x0100u&&a<=0x08FEu&&!(a&1u);}
static inline uintptr_t alias16Address(const void *p){return (uintptr_t)p+0x2000u;}
static inline uint16_t load16(const uint16_t *p){if(!validSram16(p))return 0;enable();mappingIO(false);const void *z=(const void*)alias16Address(p);__asm__ __volatile__("ld r0,Z"::"z"(z):"r0","memory");return getX();}
static inline Status store16(uint16_t *p,uint16_t v){if(!validSram16(p))return OutOfRange;enable();mappingIO(false);setX(v);void *z=(void*)alias16Address(p);__asm__ __volatile__("st Z,r0"::"z"(z):"r0","memory");return Ok;}
static inline DivResult divmod(uint32_t d,uint16_t v){enable();setAccumulator(d);setY(v);command(OP_DIVMOD);waitDivision();DivResult r={accumulator(),getY(),divideByZero()};return r;}
static inline int32_t dotProduct(const int16_t *a,const int16_t *b,uint16_t c){
  if(c==0){return 0;}
  if(!a||!b){return 0;}
  enable();command(OP_CLEAR);
  for(uint16_t i=0;i<c;++i){setX((uint16_t)a[i]);setY((uint16_t)b[i]);command(macOpcode(true,true,false,false,true));}
  return (int32_t)accumulator();
}

// dotProductFast — same math, but X is loaded through the DSC direct-SRAM
// alias window (ld r0,Z) instead of two I/O writes.  Requires both arrays in
// SRAM at even addresses (global/static/locals all qualify on this part).
// Falls back to the I/O path for any sample whose address is out of range.
static inline int32_t dotProductFast(const int16_t *a,const int16_t *b,uint16_t c){
  if(c==0){return 0;}
  if(!a||!b){return 0;}
  if(!validSram16((const void*)a) || !validSram16((const void*)b)){return dotProduct(a,b,c);}
  enable();mappingIO(false);command(OP_CLEAR);
  const uintptr_t za=alias16Address((const void*)a);
  for(uint16_t i=0;i<c;++i){
    // ld r0,Z loads SRAM[addr] straight into the DSC DX operand (1 cycle),
    // then Y is written via I/O and the MAC fires.
    __asm__ __volatile__("ld r0,Z"::"z"(za+2u*i):"r0","memory");
    setY((uint16_t)b[i]);
    command(macOpcode(true,true,false,false,true));
  }
  mappingIO(true);
  return (int32_t)accumulator();
}

// firFast — one new sample in, one filtered sample out, no copy of the
// history buffer.  Caller keeps a ring of the last N inputs; dotProductFast
// does the convolution on the ring + kernel (both in SRAM).
static inline int32_t firFast(const int16_t *ring,const int16_t *h,uint16_t n){
  return dotProductFast(ring,h,n);
}

// DSP16 — 16-bit signed int whose arithmetic TRANSPARENTLY uses uDSC.
// Write z = a * b + c and the uDSC does the math.  Mixed int constants
// work: z = x * 2 / 3 + 7.
struct DSP16 {
  int16_t v;
  DSP16(int x = 0) : v((int16_t)x) {}
  operator int16_t() const { return v; }
  operator int32_t() const { return (int32_t)v; }

  friend DSP16 operator+(DSP16 a, DSP16 b) {
    enable(); setX((uint16_t)a.v); setY((uint16_t)b.v); command(OP_ADD_S);
    return DSP16((int16_t)saturated());
  }
  friend DSP16 operator-(DSP16 a, DSP16 b) {
    enable(); setX((uint16_t)a.v); setY((uint16_t)b.v); command(OP_SUB_S);
    return DSP16((int16_t)saturated());
  }
  friend DSP16 operator*(DSP16 a, DSP16 b) {
    enable(); setX((uint16_t)a.v); setY((uint16_t)b.v); command(mulOpcode(true, true));
    return DSP16((int16_t)saturated());
  }
  friend DSP16 operator/(DSP16 a, DSP16 b) {
    if (b.v == 0) return DSP16(0);
    bool neg = (a.v < 0) != (b.v < 0);
    uint32_t ua = (uint32_t)(a.v < 0 ? (int32_t)-(int32_t)a.v : (int32_t)a.v);
    uint16_t ub = (uint16_t)(b.v < 0 ? (int32_t)-(int32_t)b.v : (int32_t)b.v);
    DivResult r = divmod(ua, ub);
    return DSP16(neg ? -(int16_t)r.quotient : (int16_t)r.quotient);
  }
  friend DSP16 operator%(DSP16 a, DSP16 b) {
    if (b.v == 0) return DSP16(0);
    bool neg = a.v < 0;
    uint32_t ua = (uint32_t)(a.v < 0 ? (int32_t)-(int32_t)a.v : (int32_t)a.v);
    uint16_t ub = (uint16_t)(b.v < 0 ? (int32_t)-(int32_t)b.v : (int32_t)b.v);
    DivResult r = divmod(ua, ub);
    return DSP16(neg ? -(int16_t)r.remainder : (int16_t)r.remainder);
  }
  DSP16 &operator+=(DSP16 o) { *this = *this + o; return *this; }
  DSP16 &operator-=(DSP16 o) { *this = *this - o; return *this; }
  DSP16 &operator*=(DSP16 o) { *this = *this * o; return *this; }
  DSP16 &operator/=(DSP16 o) { *this = *this / o; return *this; }
  DSP16 &operator%=(DSP16 o) { *this = *this % o; return *this; }
  DSP16 &mac(DSP16 a, DSP16 b) {
    enable(); setX((uint16_t)a.v); setY((uint16_t)b.v); command(macOpcode(true, true, false, false, true));
    v = (int16_t)saturated(); return *this;
  }
};
// Mixed-operand overloads so DSP16 * 2 and 2 * DSP16 both compile.
inline DSP16 operator+(DSP16 a, int b) { return a + DSP16(b); } inline DSP16 operator+(int a, DSP16 b) { return DSP16(a) + b; }
inline DSP16 operator-(DSP16 a, int b) { return a - DSP16(b); } inline DSP16 operator-(int a, DSP16 b) { return DSP16(a) - b; }
inline DSP16 operator*(DSP16 a, int b) { return a * DSP16(b); } inline DSP16 operator*(int a, DSP16 b) { return DSP16(a) * b; }
inline DSP16 operator/(DSP16 a, int b) { return a / DSP16(b); } inline DSP16 operator/(int a, DSP16 b) { return DSP16(a) / b; }
inline DSP16 operator%(DSP16 a, int b) { return a % DSP16(b); } inline DSP16 operator%(int a, DSP16 b) { return DSP16(a) % b; }
}
#else
namespace dsp {
struct DivResult{uint32_t quotient;uint16_t remainder;bool zero;};
static inline void enable(){}static inline void mappingIO(bool y){(void)y;}
static inline uint8_t flags(){return 0;}static inline bool negativeFlag(){return false;}static inline bool zeroFlag(){return false;}static inline bool carryFlag(){return false;}static inline bool divideDone(){return false;}static inline bool divideByZero(){return false;}
static inline void command(uint8_t o){(void)o;}static inline void waitDivision(){}
static inline void setX(uint16_t v){(void)v;}static inline void setY(uint16_t v){(void)v;}
static inline uint16_t getX(){return 0;}static inline uint16_t getY(){return 0;}
static inline uint16_t low(){return 0;}static inline uint16_t high(){return 0;}static inline uint16_t saturated(){return 0;}
static inline uint32_t accumulator(){return 0;}static inline void setAccumulator(uint32_t v){(void)v;}
static inline void clear(){}static inline uint32_t add(uint16_t x,uint16_t y,bool s=false){(void)x;(void)y;(void)s;return 0;}
static inline uint32_t sub(uint16_t x,uint16_t y,bool s=false){(void)x;(void)y;(void)s;return 0;}
static inline uint32_t loadY(uint16_t y){(void)y;return 0;}static inline uint32_t negY(uint16_t y,bool s=false){(void)y;(void)s;return 0;}
static inline uint32_t accumulateY(uint16_t y,bool s=false){(void)y;(void)s;return 0;}static inline uint32_t subtractY(uint16_t y,bool s=false){(void)y;(void)s;return 0;}
static inline uint32_t mul(uint16_t x,uint16_t y,bool xS=false,bool yS=false){(void)x;(void)y;(void)xS;(void)yS;return 0;}
static inline uint32_t mulNegative(uint16_t x,uint16_t y,bool xS=false,bool yS=false){(void)x;(void)y;(void)xS;(void)yS;return 0;}
static inline uint32_t mulHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false){(void)x;(void)y;(void)xS;(void)yS;return 0;}
static inline uint32_t mulNegativeHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false){(void)x;(void)y;(void)xS;(void)yS;return 0;}
static inline uint32_t mac(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){(void)x;(void)y;(void)xS;(void)yS;(void)sA;return 0;}
static inline uint32_t macHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){(void)x;(void)y;(void)xS;(void)yS;(void)sA;return 0;}
static inline uint32_t msc(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){(void)x;(void)y;(void)xS;(void)yS;(void)sA;return 0;}
static inline uint32_t mscHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){(void)x;(void)y;(void)xS;(void)yS;(void)sA;return 0;}
static inline uint32_t squareX(uint16_t x,bool s=false){(void)x;(void)s;return 0;}static inline uint32_t squareY(uint16_t y,bool s=false){(void)y;(void)s;return 0;}
static inline uint32_t abs32(bool s=true){(void)s;return 0;}static inline uint32_t neg32(bool s=true){(void)s;return 0;}
static inline void shiftLeft(uint8_t n){(void)n;}static inline void shiftRight(uint8_t n,bool a=true){(void)n;(void)a;}
static inline bool validSram16(const void *p){(void)p;return false;}static inline uintptr_t alias16Address(const void *p){(void)p;return 0;}
static inline uint16_t load16(const uint16_t *p){(void)p;return 0;}static inline Status store16(uint16_t *p,uint16_t v){(void)p;(void)v;return Unsupported;}
static inline DivResult divmod(uint32_t d,uint16_t v){(void)d;(void)v;DivResult r={0,0,true};return r;}
static inline int32_t dotProduct(const int16_t *a,const int16_t *b,uint16_t c){(void)a;(void)b;(void)c;return 0;}
static inline int32_t dotProductFast(const int16_t *a,const int16_t *b,uint16_t c){(void)a;(void)b;(void)c;return 0;}
static inline int32_t firFast(const int16_t *ring,const int16_t *h,uint16_t n){(void)ring;(void)h;(void)n;return 0;}

// DSP16 — portable fallback on 328D/E (native AVR arithmetic).
struct DSP16 {
  int16_t v;
  DSP16(int x = 0) : v((int16_t)x) {}
  operator int16_t() const { return v; }
  operator int32_t() const { return (int32_t)v; }
  friend DSP16 operator+(DSP16 a, DSP16 b) { return DSP16((int16_t)((int32_t)a.v + (int32_t)b.v)); }
  friend DSP16 operator-(DSP16 a, DSP16 b) { return DSP16((int16_t)((int32_t)a.v - (int32_t)b.v)); }
  friend DSP16 operator*(DSP16 a, DSP16 b) { return DSP16((int16_t)((int32_t)a.v * (int32_t)b.v)); }
  friend DSP16 operator/(DSP16 a, DSP16 b) { if (b.v == 0) return DSP16(0); return DSP16((int16_t)((int32_t)a.v / (int32_t)b.v)); }
  friend DSP16 operator%(DSP16 a, DSP16 b) { if (b.v == 0) return DSP16(0); return DSP16((int16_t)((int32_t)a.v % (int32_t)b.v)); }
  DSP16 &operator+=(DSP16 o) { *this = *this + o; return *this; }
  DSP16 &operator-=(DSP16 o) { *this = *this - o; return *this; }
  DSP16 &operator*=(DSP16 o) { *this = *this * o; return *this; }
  DSP16 &operator/=(DSP16 o) { *this = *this / o; return *this; }
  DSP16 &operator%=(DSP16 o) { *this = *this % o; return *this; }
  DSP16 &mac(DSP16 a, DSP16 b) { v = (int16_t)((int32_t)v + (int32_t)a.v * (int32_t)b.v); return *this; }
};
inline DSP16 operator+(DSP16 a, int b) { return a + DSP16(b); } inline DSP16 operator+(int a, DSP16 b) { return DSP16(a) + b; }
inline DSP16 operator-(DSP16 a, int b) { return a - DSP16(b); } inline DSP16 operator-(int a, DSP16 b) { return DSP16(a) - b; }
inline DSP16 operator*(DSP16 a, int b) { return a * DSP16(b); } inline DSP16 operator*(int a, DSP16 b) { return DSP16(a) * b; }
inline DSP16 operator/(DSP16 a, int b) { return a / DSP16(b); } inline DSP16 operator/(int a, DSP16 b) { return DSP16(a) / b; }
inline DSP16 operator%(DSP16 a, int b) { return a % DSP16(b); } inline DSP16 operator%(int a, DSP16 b) { return DSP16(a) % b; }
}
#endif

}
#endif