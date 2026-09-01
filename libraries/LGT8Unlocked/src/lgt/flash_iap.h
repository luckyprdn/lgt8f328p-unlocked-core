#ifndef LGT8_UNLOCKED_FLASH_IAP_H
#define LGT8_UNLOCKED_FLASH_IAP_H
#include "common.h"
#include <avr/pgmspace.h>
namespace lgt {

class FlashIAP {
public:
  FlashIAP():start_(0),end_(0),armed_(false){}
  Status writableRegion(uint16_t start,uint16_t end){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)start;(void)end;armed_=false;return Locked;
#else
    if(start>=end||end>programLimit()||(start&3u)||(end&3u))return InvalidArgument;start_=start;end_=end;armed_=true;return Ok;
#endif
  }
  void lock(){armed_=false;}
  static uint16_t programLimit(){if(!(ECCR&_BV(EEN)))return 32768u;switch(ECCR&3u){case 0:return 30720u;case 1:return 28672u;case 2:return 24576u;default:return 16384u;}}
  static Status read32Checked(uint16_t address,uint32_t *out){if(out==0)return InvalidArgument;if((address&3u)||address>programLimit()||4u>(uint16_t)(programLimit()-address))return OutOfRange;*out=pgm_read_dword_near(address);return Ok;}
  static uint32_t read32(uint16_t address){uint32_t v=0xffffffffUL;return read32Checked(address,&v)==Ok?v:0xffffffffUL;}
  Status erasePage(uint16_t address){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)address;return Locked;
#else
    if(address&0x03ffu)return InvalidArgument;uint16_t page=address;if(!allowed(page,1024u))return Locked;EEARH=(uint8_t)(page>>8);EEARL=(uint8_t)page;return issue(0x90u);
#endif
  }
  Status write32(uint16_t address,uint32_t value){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)address;(void)value;return Locked;
#else
    if(address&3u)return InvalidArgument;if(!allowed(address,4u))return Locked;E2PD0=(uint8_t)value;E2PD1=(uint8_t)(value>>8);E2PD2=(uint8_t)(value>>16);E2PD3=(uint8_t)(value>>24);EEARH=(uint8_t)(address>>8);EEARL=(uint8_t)address;return issue(0xA0u);
#endif
  }
private:
  uint16_t start_,end_;bool armed_;
  bool allowed(uint16_t address,uint16_t bytes)const{return armed_&&address>=start_&&address<end_&&bytes<=(uint16_t)(end_-address)&&bytes<=(uint16_t)(programLimit()-address);}
  static Status issue(uint8_t mode){
    // E2P clock is derived from HFRC/32.  Do not silently alter the main clock
    // tree from a flash primitive; caller must establish a known-good source.
    if(!(PMCR&_BV(RCMEN))) return NotReady;
    if(PRR1&_BV(PREFL)) return NotReady;
    if(EECR&_BV(EEPE)) return NotReady;
    uint8_t s=SREG;cli();
    EECR=(uint8_t)(mode|_BV(EEMPE));
    EECR=(uint8_t)(mode|_BV(EEPE));
    SREG=s;
    while(EECR&_BV(EEPE)){}
    return Ok;
  }
};

}
#endif
