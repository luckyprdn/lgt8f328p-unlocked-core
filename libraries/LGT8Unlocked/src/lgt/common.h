#ifndef LGT8_UNLOCKED_COMMON_H
#define LGT8_UNLOCKED_COMMON_H

#include <Arduino.h>
#include "safety.h"
#include "features.h"
#include <avr/io.h>
#include <avr/interrupt.h>

namespace lgt {

enum Status : uint8_t {
  Ok = 0,
  InvalidArgument,
  OutOfRange,
  NotReady,
  Unsupported,
  Locked
};

namespace detail {
#if LGT8_UNLOCKED_HAS_PMX012
static inline void pmx0Write(uint8_t value) {
  uint8_t s = SREG; cli();
  PMX0 = (uint8_t)(PMX0 | 0x80u);
  PMX0 = (uint8_t)(value & 0x7Fu);
  SREG = s;
}
static inline void pmx1Write(uint8_t value) {
  uint8_t s = SREG; cli();
  PMX0 = (uint8_t)(PMX0 | 0x80u);
  PMX1 = (uint8_t)(value & 0x07u);
  SREG = s;
}
static inline void pmx2Write(uint8_t value) {
  uint8_t s = SREG; cli();
  PMX2 = (uint8_t)(PMX2 | 0x80u);
  PMX2 = (uint8_t)(value & 0x67u);
  SREG = s;
}
#else
// LGT8F328D/E: pin ownership & function routes live in IOCR and PMXCR,
// which are written through the IOCE/CE enable bit (bit 7).
static inline void iocrWrite(uint8_t value) {
  uint8_t s = SREG; cli();
  IOCR = (uint8_t)(IOCR | 0x80u);           // CE/IOCE write enable
  IOCR = (uint8_t)(value & 0x7Fu);
  SREG = s;
}
static inline void pmxcrWrite(uint8_t value) {
  uint8_t s = SREG; cli();
  PMXCR = (uint8_t)(PMXCR | 0x80u);         // WCE-style bit 7
  PMXCR = (uint8_t)(value & 0x7Fu);
  SREG = s;
}
#endif
static inline void pmcrWrite(uint8_t value) {
  uint8_t s = SREG; cli();
  PMCR = (uint8_t)(PMCR | 0x80u);
  PMCR = (uint8_t)(value & 0x7Fu);
  SREG = s;
}
static inline void vdtcrWrite(uint8_t value) {
  uint8_t s = SREG; cli();
  VDTCR = (uint8_t)(VDTCR | 0x80u);
  VDTCR = (uint8_t)(value & 0x7Fu);
  SREG = s;
}
static inline void mcucrWrite(uint8_t value) {
  uint8_t s = SREG; cli();
  MCUCR = (uint8_t)(MCUCR | _BV(0)); // WCE
  MCUCR = (uint8_t)(value & (uint8_t)~_BV(0));
  SREG = s;
}
static inline void clkprWrite(uint8_t value) {
  uint8_t s = SREG; cli();
  CLKPR = _BV(WCE);
  CLKPR = (uint8_t)(value & (uint8_t)~_BV(WCE));
  SREG = s;
}
// RC32M doubler (rcm2x) startup: after F2XEN=1 the datasheet requires
// waiting until the doubled clock output is stable before TC2XS* selects it.
// A bounded delay loop (~32 cycles at 32MHz) covers the RC doubler PLL
// lock-in; loop is volatile so the compiler cannot elide it.
static inline void dscWaitStable() {
  volatile uint8_t w = 16;
  while (w--) { nop(); }
}
}
}
#endif
