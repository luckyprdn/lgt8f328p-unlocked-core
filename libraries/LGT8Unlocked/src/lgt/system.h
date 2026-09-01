#ifndef LGT8_UNLOCKED_SYSTEM_H
#define LGT8_UNLOCKED_SYSTEM_H
#include "common.h"
namespace lgt {

enum ClockSource:uint8_t{Internal32M=0,ExternalHigh=1,Internal32K=2,ExternalLow=3};
enum LVDThreshold:uint8_t{LVD_1V8=0,LVD_2V2=1,LVD_2V5=2,LVD_2V9=3,LVD_3V2=4,LVD_3V6=5,LVD_4V0=6,LVD_4V4=7};
struct DeviceId{uint8_t b[4];};

struct Clock {
  static inline Status enable(ClockSource src,bool yes=true){if((uint8_t)src>3u)return InvalidArgument;uint8_t bit=(src==Internal32M)?0:(src==Internal32K?1:(src==ExternalHigh?2:3));uint8_t v=PMCR;yes?v|=_BV(bit):v&=(uint8_t)~_BV(bit);detail::pmcrWrite(v);return Ok;}
  static inline Status select(ClockSource src){if((uint8_t)src>3u)return InvalidArgument;
#if LGT8_UNLOCKED_RECOVERY_SAFE
    return Locked;
#else
    uint8_t v=(uint8_t)(PMCR&~(_BV(CLKFS)|_BV(CLKSS)));v|=(uint8_t)(((uint8_t)src&3u)<<5);detail::pmcrWrite(v);return Ok;
#endif
  }
  static inline Status switchTo(ClockSource src,uint32_t startupDelayUs=0){
    if((uint8_t)src>3u)return InvalidArgument;
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)startupDelayUs;
    return Locked;
#else
    enable(src,true);
    if(startupDelayUs)delayMicroseconds((double)startupDelayUs);
    select(src);
    return Ok;
#endif
  }
  static inline Status prescaler(uint8_t code){if(code>8u)return InvalidArgument;uint8_t v=(uint8_t)((CLKPR&0x60u)|(code&0x0fu));detail::clkprWrite(v);return Ok;}
#if LGT8_UNLOCKED_HAS_HF_CAL
  static inline uint8_t hfCalibration(){return RCMCAL;} static inline Status hfCalibration(uint8_t v){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)v; return Locked;
#else
    RCMCAL=v; return Ok;
#endif
  }
#else
  // LGT8F328D/E: no high-frequency RC calibration register (only LF RCKCAL).
  static inline uint8_t hfCalibration(){return 0;} static inline Status hfCalibration(uint8_t v){(void)v;return Unsupported;}
#endif
  static inline uint8_t lfCalibration(){return RCKCAL;} static inline Status lfCalibration(uint8_t v){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)v; return Locked;
#else
    RCKCAL=v; return Ok;
#endif
  }
  static inline void outputPB0(bool yes=true){uint8_t v=CLKPR;yes?v|=_BV(CLKOE0):v&=(uint8_t)~_BV(CLKOE0);detail::clkprWrite(v);}
  static inline void outputPE5(bool yes=true){uint8_t v=CLKPR;yes?v|=_BV(CLKOE1):v&=(uint8_t)~_BV(CLKOE1);detail::clkprWrite(v);}
};

struct Power {
  enum Peripheral:uint8_t{
    PeripheralADC=0, PeripheralUSART=1, PeripheralSPI=2,
    PeripheralTimer1=3, PeripheralTimer0=5, PeripheralTimer2=6,
    PeripheralTWI=7
  };
  static inline bool validPeripheral(Peripheral p){uint8_t v=(uint8_t)p;return v<=7u&&v!=4u;}
  static inline Status disable(Peripheral p){if(!validPeripheral(p))return InvalidArgument;PRR|=_BV((uint8_t)p);return Ok;}static inline Status enable(Peripheral p){if(!validPeripheral(p))return InvalidArgument;PRR&=(uint8_t)~_BV((uint8_t)p);return Ok;}
#if LGT8_UNLOCKED_HAS_TIMER3
  static inline void timer3(bool on){if(on)PRR1&=(uint8_t)~_BV(3);else PRR1|=_BV(3);}
#else
  static inline void timer3(bool on){(void)on;}   // no Timer3 on 328D/E
#endif
  static inline void pinChange(bool on){if(on)PRR1&=(uint8_t)~_BV(PRPCI);else PRR1|=_BV(PRPCI);}
  static inline void flashController(bool on){if(on)PRR1&=(uint8_t)~_BV(PREFL);else PRR1|=_BV(PREFL);}
  static inline void watchdogClock(bool on){if(on)PRR1&=(uint8_t)~_BV(PRWDT);else PRR1|=_BV(PRWDT);}
#if LGT8_UNLOCKED_HAS_IVBASE
  static inline Status dps2WakePins(uint8_t portDMask){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)portDMask; return Locked;
#else
    IOCWK=portDMask; return Ok;
#endif
  }
  static inline Status dps2Timer(uint8_t periodCode,bool yes=true){if(periodCode>3u)return InvalidArgument;
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)yes; return Locked;
#else
    DPS2R=(uint8_t)((DPS2R&~0x07u)|periodCode|(yes?_BV(LPRCE):0));return Ok;
#endif
  }
  static inline Status dps2Enable(bool yes=true){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    if(yes)return Locked;
    DPS2R&=(uint8_t)~_BV(DPS2E);return Ok;
#else
    yes?DPS2R|=_BV(DPS2E):DPS2R&=(uint8_t)~_BV(DPS2E);return Ok;
#endif
  }
  static inline Status dps2(){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    return Locked;
#else
    if(!(DPS2R&_BV(DPS2E)))return NotReady;return sleepMode(7);
#endif
  }
#else
  // DPS2 power mode is a 328P extension; 328D/E do not expose DPS2R/IOCWK.
  static inline Status dps2WakePins(uint8_t portDMask){(void)portDMask;return Unsupported;}
  static inline Status dps2Timer(uint8_t periodCode,bool yes=true){(void)periodCode;(void)yes;return Unsupported;}
  static inline Status dps2Enable(bool yes=true){(void)yes;return Unsupported;}
  static inline Status dps2(){return Unsupported;}
#endif
  static inline Status sleepMode(uint8_t mode){
    if(mode==4u||mode==5u||mode>7u)return InvalidArgument;
#if LGT8_UNLOCKED_RECOVERY_SAFE
    if(mode==7u)return Locked;
#endif
    // Configure SE with interrupts masked, then execute SEI+SLEEP back-to-back.
    uint8_t s=SREG;
    cli();
    SMCR=(uint8_t)((SMCR&0xF0u)|((mode&7u)<<1)|_BV(SE));
    if(s&0x80u) __asm__ __volatile__("sei\n\tsleep\n\tcli" ::: "memory");
    else __asm__ __volatile__("sleep" ::: "memory");
    SMCR&=(uint8_t)~_BV(SE);
    SREG=s;
    return Ok;
  }
  static inline void idle(){(void)sleepMode(0);}
  static inline void adcNoiseReduction(){(void)sleepMode(1);}
  static inline void save(){(void)sleepMode(2);}
  static inline void dps1(){(void)sleepMode(3);}
  static inline void dps0(){(void)sleepMode(6);}
};

struct LVD {
  static inline Status configure(LVDThreshold threshold,bool resetEnable=true,bool enable=true){
#if LGT8_UNLOCKED_HAS_IVBASE
    // ---- LGT8F328P ----
    if((uint8_t)threshold>7u)return InvalidArgument;
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)resetEnable;(void)enable;return Locked;
#else
    // SWR is active-low on this device.  Keep it explicitly high for normal
    // LVD writes rather than trusting the contradictory reset value printed
    // in the databook.
    uint8_t v=_BV(SWR);
    v|=(uint8_t)(((uint8_t)threshold&7u)<<2);
    if(resetEnable)v|=_BV(VDREN);
    if(enable)v|=_BV(VDTEN);
    detail::vdtcrWrite(v);
    return Ok;
#endif
#else
    // ---- LGT8F328D/E: VDTS[1:0] selects 1.8V / 2.7V / 4.3V only ----
    if((uint8_t)threshold>2u)return InvalidArgument;   // only 3 thresholds exist
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)resetEnable;(void)enable;return Locked;
#else
    uint8_t v=_BV(SWR);                                // active-low SWR, keep high
    v|=(uint8_t)(((uint8_t)threshold&3u)<<2);          // VDTS1:0 -> bits 3:2
    if(resetEnable)v|=_BV(LVREN);
    if(enable)v|=_BV(VDTEN);
    detail::vdtcrWrite(v);
    return Ok;
#endif
#endif
  }
};

struct System {
  static inline uint8_t resetCause(){return MCUSR&0x3fu;}
  static inline void clearResetCause(){uint8_t swd=(uint8_t)(MCUSR&_BV(SWDD));MCUSR=swd;}
  static inline void softwareReset(){uint8_t v=VDTCR|_BV(SWR);detail::vdtcrWrite(v);v&=(uint8_t)~_BV(SWR);detail::vdtcrWrite(v);for(;;){} }
  static inline DeviceId guid(){DeviceId d={{GUID0,GUID1,GUID2,GUID3}};return d;}
#if defined(IFAIL)
  static inline bool configurationLoadFailed(){return (MCUCR&_BV(IFAIL))!=0;}
  static inline Status reloadConfiguration(){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    return Locked;
#else
    detail::mcucrWrite((uint8_t)(MCUCR|_BV(IRLD)));return Ok;
#endif
  }
#else
  // 328D/E MCUCR has no IFAIL/IRLD configuration-reload bits.
  static inline bool configurationLoadFailed(){return false;}
  static inline Status reloadConfiguration(){return Unsupported;}
#endif
  static inline bool swdDisabled(){return (MCUSR&_BV(SWDD))!=0;}
  static inline Status disableSWD(){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    return Locked;
#else
    uint8_t s=SREG;cli();uint8_t v=(uint8_t)(MCUSR|_BV(SWDD));MCUSR=v;MCUSR=v;SREG=s;return Ok;
#endif
  }
  // programLimit: ECCR EC1:0 (D) / ECS1:0 (P) both live in bits 1:0, same table.
  static inline uint16_t programLimit(){if(!(ECCR&_BV(EEN)))return 32768u;switch(ECCR&3u){case 0:return 30720u;case 1:return 28672u;case 2:return 24576u;default:return 16384u;}}
#if LGT8_UNLOCKED_HAS_IVBASE
  static inline Status relocateVectors(uint16_t byteAddress){
#if LGT8_UNLOCKED_RECOVERY_SAFE
    (void)byteAddress;return Locked;
#else
    if(byteAddress&0x1ffu)return InvalidArgument;
    uint16_t limit=programLimit();
    if(byteAddress>limit || (uint16_t)(limit-byteAddress)<512u)return OutOfRange;
    IVBASE=(uint8_t)(byteAddress>>8);uint8_t v=MCUCR|_BV(IVSEL);detail::mcucrWrite(v);return Ok;
#endif
  }
  static inline void restoreVectors(){uint8_t v=(uint8_t)(MCUCR&~_BV(IVSEL));detail::mcucrWrite(v);}
#else
  // 328D/E: no IVBASE register; vector table stays at the reset address.
  static inline Status relocateVectors(uint16_t byteAddress){(void)byteAddress;return Unsupported;}
  static inline void restoreVectors(){}
#endif
};

}
#endif