#ifndef LGT8_UNLOCKED_UDSC_H
#define LGT8_UNLOCKED_UDSC_H
#include "common.h"

namespace lgt {

#if LGT8_UNLOCKED_HAS_UDSC
namespace dsp {
enum RawOpcode:uint8_t{OP_ADD_U=0x05,OP_ADD_S=0x25,OP_SUB_U=0x01,OP_SUB_S=0x21,OP_LOAD_Y=0x1D,OP_NEG_Y_U=0x19,OP_NEG_Y_S=0x39,OP_ACC_Y_U=0x17,OP_ACC_Y_S=0x37,OP_SUBACC_Y_U=0x13,OP_SUBACC_Y_S=0x33,OP_CLEAR=0x80,OP_NEG_U=0x84,OP_NEG_S=0x85,OP_SQUARE_X_U=0x88,OP_SQUARE_X_S=0x89,OP_SQUARE_Y_U=0x8A,OP_SQUARE_Y_S=0x8B,OP_ABS_U=0xA0,OP_ABS_S=0xA1,OP_DIV=0xB0,OP_DIVMOD=0xB1};
// Silicon quirk (DOC-018): leaving DSUEN=1 while compiler-generated code runs
// can corrupt AVR-compatible control-flow instructions (reported: BRTS/SBRC
// mis-branch). Every arithmetic call must be a scoped transaction:
//   enable -> arithmetic -> read result -> disable.
// We also mask interrupts for the duration so an IRQ cannot slot AVR code
// between enable and disable. SREG is restored exactly.
static inline void enable(){DSCR|=_BV(DSUEN);}
// Disable the DSU co-processor. Called at the end of every dsp op so DSUEN
// is never left high. See DOC-018.
static inline void disable(){DSCR&=~(uint8_t)_BV(DSUEN);}
static inline void mappingIO(bool y=true){y?DSCR|=_BV(DSMM):DSCR&=~_BV(DSMM);}
static inline uint8_t flags(){return DSCR&0x37u;}static inline bool negativeFlag(){return DSCR&_BV(2);}static inline bool zeroFlag(){return DSCR&_BV(1);}static inline bool carryFlag(){return DSCR&_BV(0);}static inline bool divideDone(){return DSCR&_BV(DSD1);}static inline bool divideByZero(){return DSCR&_BV(DSD0);}
static inline void command(uint8_t o){DSIR=o;}static inline void waitDivision(){while(!divideDone()){}}
// DOC-021 (silicon-verified 2026-09-04): DX/DY/AL/AH are 16-bit registers
// accessed by REGISTER-PAIR transfer: one `out <reg>, r_lo` moves the whole
// pair r_lo:r_lo+1 (vendor dsu_xmuluu pattern + ibawizard LGT8F328P notes).
// The earlier DOC-019 "write the high byte to the adjacent IO slot" fix was
// WRONG on silicon: slot 0x11 is DSDY itself, so setX's second write
// clobbered the Y operand (probe-verified: every mul result corrupted).
// PIPELINE HAZARD (P0 #2, confirmed): uDSC samples the HIGH register first;
// if the compiler's last load (high byte) just executed, it may not have
// propagated -> stale high byte. A `nop` between operand load and `out`
// fixes it (ibawizard works_ok1). Reads need no guard.
static inline void setX(uint16_t v){__asm__ __volatile__("nop" "\n\t" "out %0,%A1"::"I"(_SFR_IO_ADDR(DSDX)),"w"(v):"memory");}
static inline void setY(uint16_t v){__asm__ __volatile__("nop" "\n\t" "out %0,%A1"::"I"(_SFR_IO_ADDR(DSDY)),"w"(v):"memory");}
static inline uint16_t getX(){uint16_t v;__asm__ __volatile__("in %A0,%1":"=w"(v):"I"(_SFR_IO_ADDR(DSDX)):"memory");return v;}
static inline uint16_t getY(){uint16_t v;__asm__ __volatile__("in %A0,%1":"=w"(v):"I"(_SFR_IO_ADDR(DSDY)):"memory");return v;}
static inline uint16_t low(){uint16_t v;__asm__ __volatile__("in %A0,%1":"=w"(v):"I"(_SFR_IO_ADDR(DSAL)):"memory");return v;}
static inline uint16_t high(){uint16_t v;__asm__ __volatile__("in %A0,%1":"=w"(v):"I"(_SFR_IO_ADDR(DSAH)):"memory");return v;}
static inline uint16_t saturated(){uint16_t v;__asm__ __volatile__("in %A0,%1":"=w"(v):"I"(_SFR_IO_ADDR(DSSD)):"memory");return v;}
static inline uint32_t accumulator(){uint32_t v=(uint32_t)low();v|=((uint32_t)high()<<16);return v;}
static inline void setAccumulator(uint32_t v){uint16_t lo=(uint16_t)v,hi=(uint16_t)(v>>16);__asm__ __volatile__("nop" "\n\t" "out %0,%A1"::"I"(_SFR_IO_ADDR(DSAL)),"w"(lo):"memory");__asm__ __volatile__("nop" "\n\t" "out %0,%A1"::"I"(_SFR_IO_ADDR(DSAH)),"w"(hi):"memory");}
static inline void clear(){enable();command(OP_CLEAR);disable();}
static inline uint32_t add(uint16_t x,uint16_t y,bool s=false){enable();setX(x);setY(y);command(s?OP_ADD_S:OP_ADD_U);uint32_t r=accumulator();disable();return r;}
static inline uint32_t sub(uint16_t x,uint16_t y,bool s=false){enable();setX(x);setY(y);command(s?OP_SUB_S:OP_SUB_U);uint32_t r=accumulator();disable();return r;}
static inline uint32_t loadY(uint16_t y){enable();setY(y);command(OP_LOAD_Y);uint32_t r=accumulator();disable();return r;}
static inline uint32_t negY(uint16_t y,bool s=false){enable();setY(y);command(s?OP_NEG_Y_S:OP_NEG_Y_U);uint32_t r=accumulator();disable();return r;}
static inline uint32_t accumulateY(uint16_t y,bool s=false){enable();setY(y);command(s?OP_ACC_Y_S:OP_ACC_Y_U);uint32_t r=accumulator();disable();return r;}
static inline uint32_t subtractY(uint16_t y,bool s=false){enable();setY(y);command(s?OP_SUBACC_Y_S:OP_SUBACC_Y_U);uint32_t r=accumulator();disable();return r;}
struct DivResult{uint32_t quotient;uint16_t remainder;bool zero;};
static inline uint8_t mulOpcode(bool xS,bool yS,bool neg=false,bool half=false){uint8_t o=0x40u;if(xS)o|=0x20u;if(yS)o|=0x10u;if(half)o|=0x08u;if(!neg)o|=0x04u;return o;}
static inline uint8_t macOpcode(bool xS,bool yS,bool sub=false,bool half=false,bool sA=false){uint8_t o=0x40u;if(xS)o|=0x20u;if(yS)o|=0x10u;if(half)o|=0x08u;if(!sub)o|=0x04u;o|=0x02u;if(sA)o|=0x01u;return o;}
static inline uint32_t mul(uint16_t x,uint16_t y,bool xS=false,bool yS=false){
  // DOC-029 (perf 2026-09-04): byte fast-path. When BOTH operands are < 0x100
  // the 16-bit value is in [0,255] regardless of the sign flags (int16 of an
  // x<0x100 is non-negative), so the product is plain x*y and native 8x8 mul
  // beats the whole uDSC transaction (~6 vs ~63 cycles). Byte-identical to
  // the uDSC result by construction.
  if (((x | y) & 0xFF00u) == 0) return (uint32_t)((uint16_t)(uint8_t)x * (uint16_t)(uint8_t)y);
  enable();setX(x);setY(y);command(mulOpcode(xS,yS));uint32_t r=accumulator();disable();return r;}
static inline uint32_t mulHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false){enable();setX(x);setY(y);command(mulOpcode(xS,yS,false,true));uint32_t r=accumulator();disable();return r;}
// DOC-026 (silicon-audited 2026-09-04): the IR "neg" bit encoding used by
// mulNegative/mulNegativeHalf returns 0 on real silicon (wrong opcode class);
// macHalf's half bit is likewise ignored for MACs. These variants are now
// composed from the proven mul/mulHalf primitives (byte-exact by
// construction) instead of raw undocumented opcodes.
static inline uint32_t mulNegative(uint16_t x,uint16_t y,bool xS=false,bool yS=false){return (uint32_t)(-(int32_t)mul(x,y,xS,yS));}
static inline uint32_t mulNegativeHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false){return (uint32_t)(-(int32_t)mulHalf(x,y,xS,yS));}
static inline uint32_t mac(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){enable();setX(x);setY(y);command(macOpcode(xS,yS,false,false,sA));uint32_t r=accumulator();disable();return r;}
static inline uint32_t macHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){enable();setX(x);setY(y);command(macOpcode(xS,yS,false,true,sA));uint32_t r=accumulator();disable();return r;}
static inline uint32_t msc(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){enable();setX(x);setY(y);command(macOpcode(xS,yS,true,false,sA));uint32_t r=accumulator();disable();return r;}
static inline uint32_t mscHalf(uint16_t x,uint16_t y,bool xS=false,bool yS=false,bool sA=false){enable();setX(x);setY(y);command(macOpcode(xS,yS,true,true,sA));uint32_t r=accumulator();disable();return r;}
static inline uint32_t squareX(uint16_t x,bool s=false){enable();setX(x);command(s?OP_SQUARE_X_S:OP_SQUARE_X_U);uint32_t r=accumulator();disable();return r;}
static inline uint32_t squareY(uint16_t y,bool s=false){enable();setY(y);command(s?OP_SQUARE_Y_S:OP_SQUARE_Y_U);uint32_t r=accumulator();disable();return r;}
static inline uint32_t abs32(bool s=true){enable();command(s?OP_ABS_S:OP_ABS_U);uint32_t r=accumulator();disable();return r;}
static inline uint32_t neg32(bool s=true){enable();command(s?OP_NEG_S:OP_NEG_U);uint32_t r=accumulator();disable();return r;}
static inline void shiftLeft(uint8_t n){if(n>15)return;enable();command((uint8_t)(0xC0u|(n&0x0fu)));disable();}
static inline void shiftRight(uint8_t n,bool a=true){if(n>15)return;enable();command((uint8_t)(0xD0u|(a?0x20u:0u)|(n&0x0fu)));disable();}
static inline bool validSram16(const void *p){uintptr_t a=(uintptr_t)p;return a>=0x0100u&&a<=0x08FEu&&!(a&1u);}
static inline uintptr_t alias16Address(const void *p){return (uintptr_t)p+0x2000u;}
static inline uint16_t load16(const uint16_t *p){if(!validSram16(p))return 0;enable();mappingIO(false);const void *z=(const void*)alias16Address(p);__asm__ __volatile__("ld r0,Z"::"z"(z):"r0","memory");uint16_t r=getX();disable();return r;}
static inline Status store16(uint16_t *p,uint16_t v){if(!validSram16(p))return OutOfRange;enable();mappingIO(false);setX(v);void *z=(void*)alias16Address(p);__asm__ __volatile__("st Z,r0"::"z"(z):"r0","memory");disable();return Ok;}
static inline DivResult divmod(uint32_t d,uint16_t v){
  // DOC-025 (silicon-verified 2026-09-04): the uDSC DIVMOD is a SIGNED
  // 32-bit division (dividend treated as int32; 0xFFFFFFFF as unsigned
  // dividend gives q=0 r=1 = |INT32_MIN-1| path). Byte-exact unsigned
  // semantics for the full uint32 range require a software fallback for
  // dividends >= 0x80000000 (rare; correctness first). Dividends below
  // that are positive-as-signed, where the uDSC path is exact.
  if (d >= 0x80000000ul) {
    DivResult r;
    if (v == 0u) { r.zero = true; r.quotient = 0ul; r.remainder = 0u; return r; }
    r.zero = false; r.quotient = d / v; r.remainder = (uint16_t)(d % v); return r;
  }
  enable();setAccumulator(d);setY(v);command(OP_DIVMOD);waitDivision();DivResult r={accumulator(),getY(),divideByZero()};disable();return r;
}
static inline int32_t dotProduct(const int16_t *a,const int16_t *b,uint16_t c){
  if(c==0){return 0;}
  if(!a||!b){return 0;}
  enable();command(OP_CLEAR);
  for(uint16_t i=0;i<c;++i){setX((uint16_t)a[i]);setY((uint16_t)b[i]);command(macOpcode(true,true,false,false,true));}
  int32_t r=(int32_t)accumulator();disable();return r;
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
  int32_t rf=(int32_t)accumulator();disable();return rf;
}

// firFast — one new sample in, one filtered sample out, no copy of the
// history buffer.  Caller keeps a ring of the last N inputs; dotProductFast
// does the convolution on the ring + kernel (both in SRAM).
static inline int32_t firFast(const int16_t *ring,const int16_t *h,uint16_t n){
  return dotProductFast(ring,h,n);
}

// DOC-022 (silicon-verified 2026-09-04): the DSSD register (IO 0x02, "16-bit
// saturation result") reads 0xFF/0xFFFF on real silicon regardless of the
// operation issued (constant across every probe dump). It is NOT a usable
// saturation result. DSP16 therefore reads the proven 32-bit accumulator and
// clamps in software (sat16). dsp::saturated() is kept for API compatibility
// but must not be used for results.
static inline int16_t sat16(int32_t v){if(v>32767)return 32767;if(v<-32768)return -32768;return (int16_t)v;}

// DSP16 — 16-bit signed int whose arithmetic TRANSPARENTLY uses uDSC.
// Write z = a * b + c and the uDSC does the math.  Mixed int constants
// work: z = x * 2 / 3 + 7.
struct DSP16 {
  int16_t v;
  DSP16(int x = 0) : v((int16_t)x) {}
  operator int16_t() const { return v; }
  operator int32_t() const { return (int32_t)v; }

  friend DSP16 operator+(DSP16 a, DSP16 b) {
    // OP_ADD_S = DA = X+Y overwrite (S8 silicon-verified). MAC chains are
    // avoided: a 2nd MAC whose product is negative double-adds on this die
    // (sub-probe 2026-09-04) - dotProduct is unaffected (verified exact).
    enable(); setX((uint16_t)a.v); setY((uint16_t)b.v); command(OP_ADD_S);
    int16_t _r = sat16((int32_t)accumulator()); disable(); return DSP16(_r);
  }
  friend DSP16 operator-(DSP16 a, DSP16 b) {
    enable(); setX((uint16_t)a.v); setY((uint16_t)b.v); command(OP_SUB_S);
    int16_t _r = sat16((int32_t)accumulator()); disable(); return DSP16(_r);
  }
  friend DSP16 operator*(DSP16 a, DSP16 b) {
    // signed 16x16 mul overwrites DA (same class as verified 0x44 overwrite)
    enable(); setX((uint16_t)a.v); setY((uint16_t)b.v); command(mulOpcode(true, true));
    int16_t _r = sat16((int32_t)accumulator()); disable(); return DSP16(_r);
  }
  friend DSP16 operator/(DSP16 a, DSP16 b) {
    // DOC-027: int16/int16 in int32 is faster than routing through the
    // 32/16 uDSC divmod, and handles INT16_MIN/-1 (32768 -> sat16 32767)
    // without UB. Saturating contract matches +,-,*.
    if (b.v == 0) return DSP16(0);
    return DSP16(sat16((int32_t)a.v / (int32_t)b.v));
  }
  friend DSP16 operator%(DSP16 a, DSP16 b) {
    if (b.v == 0) return DSP16(0);
    return DSP16(sat16((int32_t)a.v % (int32_t)b.v));   // C sign-of-dividend
  }
  DSP16 &operator+=(DSP16 o) { *this = *this + o; return *this; }
  DSP16 &operator-=(DSP16 o) { *this = *this - o; return *this; }
  DSP16 &operator*=(DSP16 o) { *this = *this * o; return *this; }
  DSP16 &operator/=(DSP16 o) { *this = *this / o; return *this; }
  DSP16 &operator%=(DSP16 o) { *this = *this % o; return *this; }
  DSP16 &mac(DSP16 a, DSP16 b) {
    // DOC-025: single uDSC signed mul (overwrite, proven) + SW accumulate.
    // Avoids the negative-product MAC-chain double-add die quirk.
    int32_t p = (int32_t)mul((uint16_t)a.v, (uint16_t)b.v, true, true);
    v = sat16((int32_t)v + p);
    return *this;
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
// Saturating contract identical to the 328P uDSC DSP16 (DOC-027).
static inline int16_t sat16(int32_t v){if(v>32767)return 32767;if(v<-32768)return -32768;return (int16_t)v;}
struct DSP16 {
  int16_t v;
  DSP16(int x = 0) : v((int16_t)x) {}
  operator int16_t() const { return v; }
  operator int32_t() const { return (int32_t)v; }
  friend DSP16 operator+(DSP16 a, DSP16 b) { return DSP16(sat16((int32_t)a.v + (int32_t)b.v)); }
  friend DSP16 operator-(DSP16 a, DSP16 b) { return DSP16(sat16((int32_t)a.v - (int32_t)b.v)); }
  friend DSP16 operator*(DSP16 a, DSP16 b) { return DSP16(sat16((int32_t)a.v * (int32_t)b.v)); }
  friend DSP16 operator/(DSP16 a, DSP16 b) { if (b.v == 0) return DSP16(0); return DSP16(sat16((int32_t)a.v / (int32_t)b.v)); }
  friend DSP16 operator%(DSP16 a, DSP16 b) { if (b.v == 0) return DSP16(0); return DSP16(sat16((int32_t)a.v % (int32_t)b.v)); }
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