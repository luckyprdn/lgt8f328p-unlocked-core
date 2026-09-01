#ifndef LGT8_UNLOCKED_FEATURES_H
#define LGT8_UNLOCKED_FEATURES_H

// Single source of truth for per-silicon feature availability.
// LGT8F328P (__LGT8FX8P__) and LGT8F328D/E (__LGT8FX8E__) differ in extensions.

#if defined(__LGT8FX8E__)
// LGT8F328D/E — smaller feature set
#define LGT8_UNLOCKED_SILICON_D    1
#define LGT8_UNLOCKED_HAS_UDSC        0
#define LGT8_UNLOCKED_HAS_TIMER3      0
#define LGT8_UNLOCKED_HAS_PGA         0
#define LGT8_UNLOCKED_HAS_ADC_MONITOR 0
#define LGT8_UNLOCKED_HAS_ADC_CAL     0
#define LGT8_UNLOCKED_HAS_SPI_DUAL    0
#define LGT8_UNLOCKED_HAS_HDR         0
#define LGT8_UNLOCKED_HAS_IVBASE      0
#define LGT8_UNLOCKED_HAS_HF_CAL      0
#define LGT8_UNLOCKED_HAS_PMX012      0
#define LGT8_UNLOCKED_HAS_OPA         1
#define LGT8_UNLOCKED_HAS_DAC1        1
#define LGT8_UNLOCKED_HAS_ADTMR_DIFF  1
#else
// LGT8F328P — full feature set
#define LGT8_UNLOCKED_SILICON_D    0
#define LGT8_UNLOCKED_HAS_UDSC        1
#define LGT8_UNLOCKED_HAS_TIMER3      1
#define LGT8_UNLOCKED_HAS_PGA         1
#define LGT8_UNLOCKED_HAS_ADC_MONITOR 1
#define LGT8_UNLOCKED_HAS_ADC_CAL     1
#define LGT8_UNLOCKED_HAS_SPI_DUAL    1
#define LGT8_UNLOCKED_HAS_HDR         1
#define LGT8_UNLOCKED_HAS_IVBASE      1
#define LGT8_UNLOCKED_HAS_HF_CAL      1
#define LGT8_UNLOCKED_HAS_PMX012      1
#define LGT8_UNLOCKED_HAS_OPA         0
#define LGT8_UNLOCKED_HAS_DAC1        0
#define LGT8_UNLOCKED_HAS_ADTMR_DIFF  0
#endif

#endif