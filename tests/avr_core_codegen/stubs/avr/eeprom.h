#ifndef LGT_AVR_STUB_EEPROM_H
#define LGT_AVR_STUB_EEPROM_H
#include <stdint.h>
static inline uint8_t eeprom_read_byte(const uint8_t *p){return *(const volatile uint8_t*)p;}
static inline void eeprom_write_byte(uint8_t *p,uint8_t v){*(volatile uint8_t*)p=v;}
#endif
