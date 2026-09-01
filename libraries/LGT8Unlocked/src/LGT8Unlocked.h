#ifndef LGT8_UNLOCKED_H
#define LGT8_UNLOCKED_H

#include <Arduino.h>

#if !defined(__LGT8FX8P__)
#error "LGT8Unlocked currently targets LGT8F328P/LGT8FX8P silicon only"
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
#if !LGT8_UNLOCKED_RECOVERY_SAFE
#include "lgt/flash_iap.h"
#endif

#endif
