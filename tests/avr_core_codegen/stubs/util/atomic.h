#ifndef LGT_ATOMIC_STUB_H
#define LGT_ATOMIC_STUB_H
#include <stdint.h>
#define ATOMIC_RESTORESTATE 0
#define ATOMIC_BLOCK(type) for (uint8_t __lgt_atomic_once=1; __lgt_atomic_once; __lgt_atomic_once=0)
#endif
