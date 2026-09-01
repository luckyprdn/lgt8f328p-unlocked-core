#ifndef LGT_AVR_STUB_INTERRUPT_H
#define LGT_AVR_STUB_INTERRUPT_H
#define cli() __asm__ __volatile__("cli" ::: "memory")
#define sei() __asm__ __volatile__("sei" ::: "memory")
#define ISR_ALIASOF(v)
#define ISR(vector, ...) void vector(void)
#endif
