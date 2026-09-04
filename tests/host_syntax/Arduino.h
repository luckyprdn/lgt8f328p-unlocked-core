#ifndef Arduino_h
#define Arduino_h
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "WString.h"

#define __LGT8F__ 1
#define __LGT8FX8P__ 1
#define __LGT8FX8P48__ 1
#define F_CPU 32000000UL
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define CHANGE 3
#define FALLING 4
#define RISING 5
#define SS 10
#define LED_BUILTIN 13
#define DEFAULT 0
#define INTERNAL1V1 2
#define INTERNAL2V56 3
#define INTERNAL1V024 4
#define INTERNAL2V048 5
#define INTERNAL4V096 6
#define INTERNAL 2
#define HEX 16
#define _BV(b) ((uint8_t)(1u<<(b)))

extern volatile uint8_t __fake_mem8[512];
#define _SFR_IO8(a) (__fake_mem8[(a)])
#define _SFR_MEM8(a) (__fake_mem8[(a)])
#define _SFR_MEM16(a) (*((volatile uint16_t*)&__fake_mem8[(a)]))
#define _SFR_IO_ADDR(x) 0
#define _SFR_MEM_ADDR(x) 0
#define _SFR_BYTE(x) (x)
struct __FlashStringHelper;
#define F(x) ((const __FlashStringHelper *)(x))
#ifndef PSTR
#define PSTR(x) ((const __FlashStringHelper *)(x))
#endif
#include "lgtx8p.h"

// Standard AVR registers/bits used by the extension but not defined by the
// LGT supplement header.
#define SREG __fake_mem8[0x3f]
#define PORTB __fake_mem8[0x25]
#define DDRB __fake_mem8[0x24]
#define PINB __fake_mem8[0x23]
#define PORTC __fake_mem8[0x28]
#define DDRC __fake_mem8[0x27]
#define PINC __fake_mem8[0x26]
#define PORTD __fake_mem8[0x2b]
#define DDRD __fake_mem8[0x2a]
#define PIND __fake_mem8[0x29]
#define DDB3 3
#define DDB4 4
#define DDD4 4
#define SPCR __fake_mem8[0x4c]
#define SPSR __fake_mem8[0x4d]
#define SPDR __fake_mem8[0x4e]
#define MSTR 4
#define SPE 6
#define SPIF 7
#define UCSR0A __fake_mem8[0xc0]
#define UCSR0B __fake_mem8[0xc1]
#define UCSR0C __fake_mem8[0xc2]
#define UBRR0L __fake_mem8[0xc4]
#define UBRR0H __fake_mem8[0xc5]
#define UDR0 __fake_mem8[0xc6]
#define UMSEL01 7
#define UMSEL00 6
#define UCPOL0 0
#define UCSZ02 2
#define UCSZ01 2
#define UCSZ00 1
#define MPCM0 0
#define TXB80 0
#define RXB80 1
#define UDRE0 5
#define RXC0 7
#define RXEN0 4
#define TXEN0 3
#define TCCR0A __fake_mem8[0x44]
#define TCCR0B __fake_mem8[0x45]
#define TCNT0 __fake_mem8[0x46]
#define OCR0A __fake_mem8[0x47]
#define OCR0B __fake_mem8[0x48]
#define TCCR1A __fake_mem8[0x80]
#define TCCR1B __fake_mem8[0x81]
#define TCCR1C __fake_mem8[0x82]
#define TCNT1 _SFR_MEM16(0x84)
#define ICR1 _SFR_MEM16(0x86)
#define OCR1A _SFR_MEM16(0x88)
#define OCR1B _SFR_MEM16(0x8a)
#define TIMSK1 __fake_mem8[0x6f]
#define ICES1 6
#define ICNC1 7
#define ICIE1 5
#define TCCR2A __fake_mem8[0xb0]
#define TCCR2B __fake_mem8[0xb1]
#define TCNT2 __fake_mem8[0xb2]
#define OCR2A __fake_mem8[0xb3]
#define OCR2B __fake_mem8[0xb4]
#define TIMSK2 __fake_mem8[0x70]
#define TIFR2 __fake_mem8[0x37]
#define OCIE2A 1
#define OCIE2B 2
#define TOIE2 0
#define OCF2A 1
#define OCF2B 2
#define TOV2 0
#define CS22 2
#define CS21 1
#define CS20 0
#define ASSR __fake_mem8[0xb6]
#define INTCK 7
#define AS2 5
#define TCN2UB 4
#define OCR2AUB 3
#define OCR2BUB 2
#define TCR2AUB 1
#define TCR2BUB 0
#define ADMUX __fake_mem8[0x7c]
#define ADCSRA __fake_mem8[0x7a]
#define ADCSRB __fake_mem8[0x7b]
#define ADC _SFR_MEM16(0x78)
#define ADEN 7
#define ADSC 6
#define ADATE 5
#define ADIF 4
#define ADIE 3
#define REFS0 6
#define PRR __fake_mem8[0x64]
#define SMCR __fake_mem8[0x53]
#define SE 0
#define MCUSR __fake_mem8[0x54]
#define MCUCR __fake_mem8[0x55]
#define CLKPR __fake_mem8[0x61]
#define IVSEL 1
#define EEARL __fake_mem8[0x41]
#define EEARH __fake_mem8[0x42]
#define EEDR __fake_mem8[0x40]
#define EECR __fake_mem8[0x3f]
#define EERE 0
#define EEPE 1
#define EEMPE 2
#define PCIFR __fake_mem8[0x3b]
#define PCICR __fake_mem8[0x68]

#ifndef TWBR
#define TWBR __fake_mem8[0xb8]
#define TWSR __fake_mem8[0xb9]
#define TWAR __fake_mem8[0xba]
#define TWDR __fake_mem8[0xbb]
#define TWCR __fake_mem8[0xbc]
#define TWAMR __fake_mem8[0xbd]
#define TWPS0 0
#define TWPS1 1
#define TWIE 0
#define TWEN 2
#define TWWC 3
#define TWSTO 4
#define TWSTA 5
#define TWEA 6
#define TWINT 7
#define TWGCE 0
#define TWI_vect __fake_twi_vect
#endif

#define NUM_PIN_TABLE_ENTRIES 40
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19
#define A6 20
#define A7 21
#define A8 23
#define A9 24
#define A10 25
#define A11 26
#define SDA 18
#define SCL 19
#define D33 33
#define D34 34
#define D35 35

static inline volatile uint8_t* digitalPinToPCMSK(uint8_t){return &__fake_mem8[0x6b];}
static inline volatile uint8_t* digitalPinToPCICR(uint8_t){return &PCICR;}
static inline uint8_t digitalPinToPCMSKbit(uint8_t p){return p&7;}
static inline uint8_t digitalPinToPCICRbit(uint8_t p){return (uint8_t)(p/8);}
static inline int analogReadFast(uint8_t){return 0;}
static inline int analogRead(uint8_t){return 0;}
static inline void analogReadResolution(int){}
static inline void analogReference(int){}
static inline void analogWrite(uint8_t,int){}
static inline void pinMode(uint8_t,uint8_t){}
static inline void digitalWrite(uint8_t,uint8_t){}
static inline int digitalRead(uint8_t){return 0;}
static inline unsigned long micros(){return 0;}
static inline unsigned long millis(){return 0;}
static inline void delay(unsigned long){}
static inline void delayMicroseconds(double){}
static inline void noInterrupts(){}
static inline void interrupts(){}
#define cli() do{}while(0)
#define sei() do{}while(0)
#ifndef nop
#define nop() __asm__ __volatile__("nop")
#endif

struct FakeSerialClass {
  void begin(unsigned long) {}
  operator bool() const { return true; }
  bool operator!() const { return false; }
  template <typename T> void print(const T&) {}
  template <typename T> void print(const T&, int) {}
  template <typename T> void println(const T&) {}
  template <typename T> void println(const T&, int) {}
  void println() {}
  void flush() {}
};
static FakeSerialClass Serial __attribute__((unused));

#endif

// Timer1 bit names come from avr-libc <avr/io.h> on a real build; the host
// stub replaces io.h, so define the ones the library headers use.
#ifndef WGM10
#define WGM10 0
#endif
#ifndef WGM11
#define WGM11 1
#endif
#ifndef WGM12
#define WGM12 3
#endif
#ifndef WGM13
#define WGM13 4
#endif
#ifndef COM1A0
#define COM1A0 6
#endif
#ifndef COM1A1
#define COM1A1 7
#endif

// Common AVR-libc bit constants the examples/headers use (host stub).
#define CS00 0
#define CS01 1
#define CS02 2
#define CS10 0
#define CS11 1
#define CS12 2
#define CS20 0
#define CS21 1
#define CS22 2
#define WGM20 0
#define WGM21 1
#define WGM22 3
#define WGM23 4
#define COM0A0 6
#define COM0A1 7
#define COM0B0 4
#define COM0B1 5
#define COM1B0 4
#define COM1B1 5
#define COM2A0 6
#define COM2A1 7
#define COM2B0 4
#define COM2B1 5
#define ADLAR 5
#define ADSC 6
#define ADEN 7
#define MUX0 0
#define MUX1 1
#define MUX2 2
#define MUX3 3
#define ADPS0 0
#define ADPS1 1
#define ADPS2 2
#define ADIE 3
#define ADIF 4
