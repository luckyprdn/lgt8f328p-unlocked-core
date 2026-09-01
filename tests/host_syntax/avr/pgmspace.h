#ifndef LGT_HOST_FAKE_AVR_PGMSPACE_H
#define LGT_HOST_FAKE_AVR_PGMSPACE_H
#include <stdint.h>
static inline uint32_t pgm_read_dword_near(uint16_t){return 0;}
#endif
