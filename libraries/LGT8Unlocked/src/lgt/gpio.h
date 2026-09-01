#ifndef LGT8_UNLOCKED_GPIO_H
#define LGT8_UNLOCKED_GPIO_H
#include "common.h"

namespace lgt {

enum HighDrivePin : uint8_t {
  HighDrive_PD5 = 0,
  HighDrive_PD6 = 1,
  HighDrive_PF1 = 2,
  HighDrive_PF2 = 3,
  HighDrive_PF4 = 4,
  HighDrive_PF5 = 5
};

struct DriveStrength {
  static inline Status high(HighDrivePin pin) {
    const uint8_t p=(uint8_t)pin;
    if(p>5u) return InvalidArgument;
#if !defined(__LGT8FX8P48__)
    if(p>=2u) return Unsupported;
#endif
#if LGT8_UNLOCKED_RECOVERY_SAFE && !LGT8_UNLOCKED_ALLOW_HIGH_DRIVE
    (void)p;
    return Locked;
#else
    HDR |= _BV(p);
    return Ok;
#endif
  }
  static inline Status normal(HighDrivePin pin) {
    const uint8_t p=(uint8_t)pin;
    if(p>5u) return InvalidArgument;
#if !defined(__LGT8FX8P48__)
    if(p>=2u) return Unsupported;
#endif
    HDR &= (uint8_t)~_BV(p);
    return Ok;
  }
  static inline bool isHigh(HighDrivePin pin) {
    const uint8_t p=(uint8_t)pin;
    if(p>5u)return false;
#if !defined(__LGT8FX8P48__)
    if(p>=2u)return false;
#endif
    return (HDR & _BV(p)) != 0;
  }
};

struct PinChange {
  static inline Status enable(uint8_t pin) {
    if (pin >= NUM_PIN_TABLE_ENTRIES) return InvalidArgument;
    volatile uint8_t *mask = digitalPinToPCMSK(pin);
    volatile uint8_t *ctrl = digitalPinToPCICR(pin);
    if (!mask || !ctrl) return Unsupported;
    *mask |= _BV(digitalPinToPCMSKbit(pin));
    *ctrl |= _BV(digitalPinToPCICRbit(pin));
    return Ok;
  }
  static inline Status disable(uint8_t pin) {
    if (pin >= NUM_PIN_TABLE_ENTRIES) return InvalidArgument;
    volatile uint8_t *mask = digitalPinToPCMSK(pin);
    if (!mask) return Unsupported;
    *mask &= (uint8_t)~_BV(digitalPinToPCMSKbit(pin));
    volatile uint8_t *ctrl = digitalPinToPCICR(pin);
    uint8_t group=digitalPinToPCICRbit(pin);
    if(ctrl && group<=4u && *mask==0u)*ctrl&=(uint8_t)~_BV(group);
    return Ok;
  }
  static inline Status clearFlag(uint8_t pin) {
    if (pin >= NUM_PIN_TABLE_ENTRIES) return InvalidArgument;
    volatile uint8_t *mask = digitalPinToPCMSK(pin);
    if (!mask) return Unsupported;
    uint8_t group = digitalPinToPCICRbit(pin);
    if (group > 4) return Unsupported;
    PCIFR = _BV(group);
    return Ok;
  }
};

}
#endif
