#ifndef LGT_HOST_FAKE_AVR_INTERRUPT_H
#define LGT_HOST_FAKE_AVR_INTERRUPT_H
#ifndef cli
#define cli() do{}while(0)
#define sei() do{}while(0)
#endif
#ifndef ISR
#define ISR(vector) void vector(void)
#endif
#endif
