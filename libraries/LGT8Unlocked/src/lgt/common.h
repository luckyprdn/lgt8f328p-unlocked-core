#ifndef LGT8_UNLOCKED_COMMON_H
#define LGT8_UNLOCKED_COMMON_H

#include <Arduino.h>
#include "safety.h"
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
}
}
#endif
