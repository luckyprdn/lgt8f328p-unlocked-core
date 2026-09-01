#ifndef LGT_AVR_STUB_IO_H
#define LGT_AVR_STUB_IO_H
#include <stdint.h>
#define _BV(b) (1u << (b))
#define _SFR_IO8(a) (*(volatile uint8_t *)((a)+0x20u))
#define _SFR_MEM8(a) (*(volatile uint8_t *)(a))
#define _SFR_MEM16(a) (*(volatile uint16_t *)(a))
#define _SFR_IO_ADDR(sfr) ((uint16_t)(&(sfr)) - 0x20u)
#define _SFR_MEM_ADDR(sfr) ((uint16_t)(&(sfr)))
#define _SFR_BYTE(sfr) (sfr)
#define bit_is_set(sfr,bit) ((sfr) & _BV(bit))
#define bit_is_clear(sfr,bit) (!bit_is_set((sfr),(bit)))
#define PINB _SFR_IO8(0x03)
#define DDRB _SFR_IO8(0x04)
#define PORTB _SFR_IO8(0x05)
#define PINC _SFR_IO8(0x06)
#define DDRC _SFR_IO8(0x07)
#define PORTC _SFR_IO8(0x08)
#define PIND _SFR_IO8(0x09)
#define DDRD _SFR_IO8(0x0A)
#define PORTD _SFR_IO8(0x0B)
#define SREG _SFR_IO8(0x3F)
#define GPIOR0 _SFR_IO8(0x1E)

#define EEDR _SFR_IO8(0x20)
#define EEARL _SFR_IO8(0x21)
#define EEARH _SFR_IO8(0x22)
#define EECR _SFR_IO8(0x1F)
#define EERE 0
#define EEPE 1
#define EEMPE 2
#define EERIE 3

#define TWBR _SFR_MEM8(0xB8)
#define TWSR _SFR_MEM8(0xB9)
#define TWAR _SFR_MEM8(0xBA)
#define TWDR _SFR_MEM8(0xBB)
#define TWCR _SFR_MEM8(0xBC)
#define TWAMR _SFR_MEM8(0xBD)
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
#define TWI_vect __lgt_twi_vect
#define MCUSR _SFR_IO8(0x34)
#define PCMSK0 _SFR_MEM8(0x6B)
#define PCMSK1 _SFR_MEM8(0x6C)
#define PCMSK2 _SFR_MEM8(0x6D)

#define ADCL _SFR_MEM8(0x78)
#define ADCH _SFR_MEM8(0x79)
#define ADC _SFR_MEM16(0x78)
#define ADCSRA _SFR_MEM8(0x7A)
#define ADCSRB _SFR_MEM8(0x7B)
#define ADMUX _SFR_MEM8(0x7C)
#define ADEN 7
#define ADSC 6
#define ADATE 5
#define ADIF 4
#define ADIE 3
#define REFS0 6

#define TCCR0A _SFR_IO8(0x24)
#define TCCR0B _SFR_IO8(0x25)
#define TCNT0 _SFR_IO8(0x26)
#define OCR0A _SFR_IO8(0x27)
#define OCR0B _SFR_IO8(0x28)
#define COM0A1 7
#define COM0B1 5

#define TCCR1A _SFR_MEM8(0x80)
#define TCCR1B _SFR_MEM8(0x81)
#define TCCR1C _SFR_MEM8(0x82)
#define TCNT1 _SFR_MEM16(0x84)
#define ICR1 _SFR_MEM16(0x86)
#define OCR1A _SFR_MEM16(0x88)
#define OCR1B _SFR_MEM16(0x8A)
#define COM1A1 7
#define COM1B1 5

#define TCCR2A _SFR_MEM8(0xB0)
#define TCCR2B _SFR_MEM8(0xB1)
#define TCNT2 _SFR_MEM8(0xB2)
#define OCR2A _SFR_MEM8(0xB3)
#define OCR2B _SFR_MEM8(0xB4)
#define COM2A1 7
#define COM2B1 5
#endif

#ifndef WGM21
#define WGM21 1
#define CS20 0
#define WGM01 1
#define CS00 0
#define WGM12 3
#define CS10 0
#define EICRA _SFR_MEM8(0x69)
#define EIMSK _SFR_MEM8(0x3D)
#define ISC00 0
#define ISC01 1
#define ISC10 2
#define ISC11 3
#define INT0 0
#define INT1 1
#endif
