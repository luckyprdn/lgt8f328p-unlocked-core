#ifndef LGT8_UNLOCKED_H
#define LGT8_UNLOCKED_H

#include <Arduino.h>

// Allow compilation on both LGT8F328P and LGT8F328D/E variant.
// Individual feature headers wrap P-specific registers with feature guards.
#if !defined(__LGT8FX8P__) && !defined(__LGT8FX8E__)
#error "LGT8Unlocked targets LGT8Fx8P or LGT8Fx8E silicon only"
#endif

#include "lgt/gpio.h"
#include "lgt/pinmux.h"
#include "lgt/timer.h"
#include "lgt/adc.h"
#include "lgt/dac.h"
#include "lgt/comparator.h"
#include "lgt/udsc.h"
#include "lgt/spi_ext.h"
#include "lgt/usart.h"
#include "lgt/system.h"
#include "lgt/arduino_ext.h"
#if LGT8_UNLOCKED_HAS_OPA
#include "lgt/opa.h"
#endif
#if !LGT8_UNLOCKED_RECOVERY_SAFE
#include "lgt/flash_iap.h"
#endif

#endif