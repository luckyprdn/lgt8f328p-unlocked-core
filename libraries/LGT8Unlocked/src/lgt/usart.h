#ifndef LGT8_UNLOCKED_USART_H
#define LGT8_UNLOCKED_USART_H
#include "common.h"
namespace lgt {

struct USART0Advanced {
  static inline void modeAsync(){UCSR0C&=(uint8_t)~(_BV(UMSEL01)|_BV(UMSEL00));}
  static inline void modeSync(bool master,bool polarity=false){
    UCSR0C=(uint8_t)((UCSR0C&~(_BV(UMSEL01)|_BV(UMSEL00)|_BV(UCPOL0)))|_BV(UMSEL00)|(polarity?_BV(UCPOL0):0));
    if(master) DDRD|=_BV(DDD4); else DDRD&=(uint8_t)~_BV(DDD4);
  }
  static inline void nineBit(bool yes=true){
    if(yes){UCSR0B|=_BV(UCSZ02);UCSR0C|=_BV(UCSZ01)|_BV(UCSZ00);}else{UCSR0B&=(uint8_t)~_BV(UCSZ02);}
  }
  static inline void multiprocessor(bool yes=true){uint8_t v=(uint8_t)(UCSR0A&0x03u);yes?v|=_BV(MPCM0):v&=(uint8_t)~_BV(MPCM0);UCSR0A=v;}
  static inline void write9(uint16_t v){while(!(UCSR0A&_BV(UDRE0))){}if(v&0x100u)UCSR0B|=_BV(TXB80);else UCSR0B&=(uint8_t)~_BV(TXB80);UDR0=(uint8_t)v;}
  static inline bool available(){return (UCSR0A&_BV(RXC0))!=0;}
  static inline uint16_t read9(){uint8_t b=(UCSR0B&_BV(RXB80))?1:0;uint8_t lo=UDR0;return ((uint16_t)b<<8)|lo;}
};

struct USARTSPI0 {
  enum Mode:uint8_t{Mode0=0,Mode1=1,Mode2=2,Mode3=3};
  static inline Status begin(uint32_t clock,Mode mode=Mode0,bool lsbFirst=false,bool master=true){
    if((uint8_t)mode>3u)return InvalidArgument;
    uint32_t ubrr=0;
    if(master){
      if(clock==0)return InvalidArgument;
      uint32_t div=F_CPU/(2UL*clock);if(div==0)div=1;ubrr=div-1;if(ubrr>4095UL)return OutOfRange;
    }
    UCSR0B=0;
    uint8_t c=master?(_BV(UMSEL01)|_BV(UMSEL00)):_BV(UMSEL01);
    if(lsbFirst)c|=_BV(UCSZ01); // in USPI mode bit2 becomes DORD
    if((uint8_t)mode&1)c|=_BV(UCSZ00); // bit1 becomes UCPHA
    if((uint8_t)mode&2)c|=_BV(UCPOL0);
    UCSR0C=c;UBRR0H=(uint8_t)(ubrr>>8);UBRR0L=(uint8_t)ubrr;
    if(master)DDRD|=_BV(DDD4);else DDRD&=(uint8_t)~_BV(DDD4);
    UCSR0B=_BV(RXEN0)|_BV(TXEN0);return Ok;
  }
  static inline uint8_t transfer(uint8_t v){while(!(UCSR0A&_BV(UDRE0))){}UDR0=v;while(!(UCSR0A&_BV(RXC0))){}return UDR0;}
  static inline void end(){UCSR0B=0;modeAsync();DDRD&=(uint8_t)~_BV(DDD4);}
private:
  static inline void modeAsync(){
    // In USPI mode UCSRC bits 2:1 are DORD/UCPHA.  Once UMSEL returns to
    // asynchronous USART those same bits are UCSZ01:0, so restore 8N1 rather
    // than carrying the SPI phase/order bits into the UART frame format.
    UCSR0C=(uint8_t)(_BV(UCSZ01)|_BV(UCSZ00));
  }
};

}
#endif
