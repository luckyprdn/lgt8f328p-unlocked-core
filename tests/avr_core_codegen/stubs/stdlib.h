#ifndef LGT_AVR_STUB_STDLIB_H
#define LGT_AVR_STUB_STDLIB_H
#include <stddef.h>
#include <stdint.h>
#ifndef __builtin_avr_delay_cycles
#define __builtin_avr_delay_cycles(x) do { (void)(x); } while (0)
#endif
#endif
