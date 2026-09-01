#ifndef LGT_AVR_STUB_PGMSPACE_H
#define LGT_AVR_STUB_PGMSPACE_H
#include <stdint.h>
#define PROGMEM __attribute__((section(".progmem.data")))
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#define pgm_read_word(addr) (*(const uint16_t *)(addr))
#define pgm_read_dword_near(addr) (*(const uint32_t *)(uintptr_t)(addr))
#endif
