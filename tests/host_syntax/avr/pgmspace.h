#ifndef LGT_HOST_FAKE_AVR_PGMSPACE_H
#define LGT_HOST_FAKE_AVR_PGMSPACE_H
#include <stdint.h>
#include <string.h>
#define PROGMEM
#define PGM_P const char *
#ifndef PSTR
#define PSTR(x) (x)
#endif
// Real AVR signatures take an address_t; on the host every read is fake, so
// take uint16_t and return 0 - avoids int<->pointer cast warnings under -Werror.
static inline uint8_t  pgm_read_byte(uint16_t) { return 0; }
static inline uint16_t pgm_read_word(uint16_t) { return 0; }
static inline uint32_t pgm_read_dword(uint16_t) { return 0; }
static inline uint8_t  pgm_read_byte_near(uint16_t) { return 0; }
static inline uint16_t pgm_read_word_near(uint16_t) { return 0; }
static inline uint32_t pgm_read_dword_near(uint16_t) { return 0; }
static inline void *memcpy_P(void *dst, const void *src, size_t n) { return memcpy(dst, src, n); }
#endif
