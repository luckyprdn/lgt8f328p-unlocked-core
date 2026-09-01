#include <LGT8Unlocked.h>
#include <avr/interrupt.h>

// PCINT callback dispatch for lgt::pinChangeAttach / pinChangeDetach.
// Small fixed-size table (8 slots) — enough for most GPIO-wake / button /
// rotary-encoder use cases while keeping SRAM usage tiny.

namespace lgt {
namespace {
#define LGT_PCINT_SLOTS 8

volatile uint8_t  pcint_pins[LGT_PCINT_SLOTS];
volatile uint8_t  pcint_mode[LGT_PCINT_SLOTS];   // 0=CHANGE 1=RISING 2=FALLING
volatile uint8_t  pcint_last[LGT_PCINT_SLOTS];
void (*volatile pcint_cb[LGT_PCINT_SLOTS])(void);
volatile uint8_t  pcint_count = 0;

inline void pcint_handle(uint8_t group) {
  for (uint8_t i = 0; i < pcint_count; ++i) {
    if (digitalPinToPCICRbit(pcint_pins[i]) != group) continue;
    volatile uint8_t *mask = digitalPinToPCMSK(pcint_pins[i]);
    if (!mask || !(*mask & _BV(digitalPinToPCMSKbit(pcint_pins[i])))) continue;
    uint8_t st = digitalRead(pcint_pins[i]) ? 1 : 0;
    uint8_t m = pcint_mode[i];
    bool fire = false;
    if (m == 0)      fire = (st != pcint_last[i]);
    else if (m == 1) fire = (st && !pcint_last[i]);
    else if (m == 2) fire = (!st && pcint_last[i]);
    pcint_last[i] = st;
    if (fire && pcint_cb[i]) pcint_cb[i]();
  }
}
}

Status pinChangeAttach(uint8_t pin, void (*callback)(void), uint8_t mode) {
  if (!callback || mode > 2) return InvalidArgument;
  if (pin >= NUM_PIN_TABLE_ENTRIES) return InvalidArgument;
  volatile uint8_t *mask = digitalPinToPCMSK(pin);
  volatile uint8_t *ctrl = digitalPinToPCICR(pin);
  if (!mask || !ctrl) return Unsupported;

  uint8_t s = SREG; cli();
  for (uint8_t i = 0; i < pcint_count; ++i)
    if (pcint_pins[i] == pin) { pcint_cb[i] = callback; pcint_mode[i] = mode; SREG = s; return Ok; }
  if (pcint_count >= LGT_PCINT_SLOTS) { SREG = s; return OutOfRange; }

  pcint_pins[pcint_count] = pin;
  pcint_mode[pcint_count] = mode;
  pcint_cb[pcint_count] = callback;
  pcint_last[pcint_count] = digitalRead(pin) ? 1 : 0;
  ++pcint_count;
  *mask |= _BV(digitalPinToPCMSKbit(pin));
  *ctrl |= _BV(digitalPinToPCICRbit(pin));
  SREG = s;
  return Ok;
}

void pinChangeDetach(uint8_t pin) {
  uint8_t s = SREG; cli();
  for (uint8_t i = 0; i < pcint_count; ++i) {
    if (pcint_pins[i] == pin) {
      volatile uint8_t *mask = digitalPinToPCMSK(pin);
      if (mask) *mask &= (uint8_t)~_BV(digitalPinToPCMSKbit(pin));
      // move last entry into the freed slot
      --pcint_count;
      pcint_pins[i] = pcint_pins[pcint_count];
      pcint_mode[i] = pcint_mode[pcint_count];
      pcint_cb[i]  = pcint_cb[pcint_count];
      pcint_last[i] = pcint_last[pcint_count];
      break;
    }
  }
  SREG = s;
}

// PCINT group ISRs (inside the lgt namespace so they can see the tables).
ISR(PCINT0_vect) { lgt::pcint_handle(0); }
ISR(PCINT1_vect) { lgt::pcint_handle(1); }
ISR(PCINT2_vect) { lgt::pcint_handle(2); }
ISR(PCINT3_vect) { lgt::pcint_handle(3); }
#if defined(__LGT8FX8P__)
ISR(PCINT4_vect) { lgt::pcint_handle(4); }
#endif

} // namespace lgt