#ifndef LGT8_UNLOCKED_ADC_H
#define LGT8_UNLOCKED_ADC_H
#include "common.h"

namespace lgt {
namespace detail {
static inline void adcUpdate(uint8_t setMask,uint8_t clearMask){uint8_t v=(uint8_t)(ADCSRA & (uint8_t)~_BV(ADIF));v=(uint8_t)((v|setMask)&(uint8_t)~clearMask);ADCSRA=v;}
static inline void adcClearFlag(){ADCSRA=(uint8_t)((ADCSRA&~_BV(ADIF))|_BV(ADIF));}
}

enum ADCTrigger : uint8_t {
  ADCContinuous=0, ADCComparator=1, ADCExternalInt0=2,
  ADCTimer0Compare=3, ADCTimer0Overflow=4, ADCTimer1CompareB=5,
  ADCTimer1Overflow=6, ADCTimer1Capture=7
};
enum ADCComparatorTrigger : uint8_t { TriggerComparator0=0, TriggerComparator1=1 };
enum ADCPGAGain : uint8_t { Gain1=0, Gain8=1, Gain16=2, Gain32=3 };
enum ADCPGAPositive : uint8_t { PGAPosADCMux=0, PGAPosAPP0=1, PGAPosAPP1=2, PGAPosAGND=3 };
enum ADCPGANegative : uint8_t { PGANegAPN0=0, PGANegAPN1=1, PGANegAPN2=2, PGANegAPN3=3, PGANegAPN4=4, PGANegADCMux=5, PGANegAGND=6, PGANegOff=7 };

struct ADCAdvanced {
  static inline int readFast(uint8_t pin){return analogReadFast(pin);}
  static inline void highSpeed(bool yes=true){yes?ADCSRC|=_BV(SPD):ADCSRC&=(uint8_t)~_BV(SPD);}
  static inline void fixedOffsetCompensation(bool yes=true){yes?ADCSRC|=_BV(OFEN):ADCSRC&=(uint8_t)~_BV(OFEN);}
  static inline void offsetRegisters(int8_t positive,int8_t negative){OFR0=(uint8_t)positive;OFR1=(uint8_t)negative;}

  static inline Status channel(uint8_t rawChannel){if(rawChannel>31u)return InvalidArgument;ADMUX=(uint8_t)((ADMUX&0xE0u)|(rawChannel&0x1Fu));return Ok;}
  static inline Status channelFromPin(uint8_t pin){
    uint8_t ch=pin;
    if(pin>=14u){ch=(uint8_t)(pin-14u);}
    if(ch>15u)return InvalidArgument;
    // CHMUX 8 is the internal 1/5 VDO channel. QFP48 Arduino A8 begins at
    // digital pin 23 and therefore naturally maps to channel 9.
    return channel(ch);
  }

  static inline void trigger(ADCTrigger source, ADCComparatorTrigger comparator=TriggerComparator0){
    ADCSRB=(uint8_t)((ADCSRB&~0x0Fu)|((uint8_t)source&0x07u));
    if(comparator==TriggerComparator1)ADCSRB|=_BV(ACTS);else ADCSRB&=(uint8_t)~_BV(ACTS);
    // ADTS=0 is the continuous/free-running selection.  As on AVR-style ADCs,
    // ADATE remains enabled so ADSC can start the first conversion and the
    // converter can continue autonomously.  Nonzero ADTS values use the
    // selected event edge as the trigger source.
    detail::adcUpdate(_BV(ADATE),0);
  }
  static inline void disableTrigger(){detail::adcUpdate(0,_BV(ADATE));}
  static inline void start(){detail::adcUpdate(_BV(ADSC),0);}
  static inline bool busy(){return (ADCSRA&_BV(ADSC))!=0;}
  static inline uint16_t result(){return ADC;}

  static inline Status monitor(uint16_t low,uint16_t high,uint8_t consecutive=1){
    if(low>0x0FFFu||high>0x0FFFu||low>high||consecutive==0u||consecutive>15u)return InvalidArgument;
    ADT0=low;ADT1=high;ADMSC=(uint8_t)(consecutive&0x0Fu);clearInterruptFlag();ADCSRC|=_BV(AMEN);return Ok;
  }
  static inline void stopMonitor(){ADCSRC&=(uint8_t)~_BV(AMEN);}
  static inline bool monitorOverflow(){return (ADMSC&_BV(AMOF))!=0;}
  static inline bool monitorTriggered(){return (ADCSRA&_BV(ADIF))!=0;}
  static inline void clearInterruptFlag(){detail::adcClearFlag();}
  static inline void interrupt(bool yes=true){yes?detail::adcUpdate(_BV(ADIE),0):detail::adcUpdate(0,_BV(ADIE));}

  static inline void differentialEnable(bool yes=true){yes?ADCSRC|=_BV(DIFS):ADCSRC&=(uint8_t)~_BV(DIFS);}
  static inline Status configurePGA(ADCPGAPositive positive,ADCPGANegative negative,ADCPGAGain gain,bool enable=true){
    if((uint8_t)positive>3u||(uint8_t)negative>7u||(uint8_t)gain>3u)return InvalidArgument;
    DAPCR=(uint8_t)((enable?_BV(DAPEN):0u)|(((uint8_t)gain)<<5)|(((uint8_t)negative)<<2)|((uint8_t)positive));return Ok;
  }
  static inline void pgaRaw(uint8_t dapcr){DAPCR=dapcr;}

  // Performs the fixed-offset calibration sequence documented by LogicGreen.
  // It temporarily uses AVCC as both VDS input and ADC reference, averages the
  // requested number of conversions, writes signed OFR0/OFR1 and enables OFEN.
  static inline Status calibrateOffset(uint8_t samples=8){
    if(samples==0u||samples>64u)return InvalidArgument;
    if((PRR&_BV(0)) || (ADCSRA&(_BV(ADSC)|_BV(ADIF))) || (ADCSRC&_BV(AMEN)))return NotReady;
    uint8_t saveMux=ADMUX,saveA=ADCSRA,saveB=ADCSRB,saveC=ADCSRC,saveD=ADCSRD;
    ADCSRA=(uint8_t)(((saveA|_BV(ADEN))&~(_BV(ADATE)|_BV(ADIE)|_BV(ADIF))));
    ADCSRC=(uint8_t)(ADCSRC&~(_BV(OFEN)|_BV(AMEN)|_BV(DIFS)|_BV(SPN)));
    ADCSRD=(uint8_t)((ADCSRD&0xF8u)|0x06u); // VDS input = VCC
    ADCSRD&=(uint8_t)~_BV(REFS2);

    ADMUX=(uint8_t)(_BV(REFS0)|14u); // AVCC ref, 4/5 VDO
    ADCSRC&=(uint8_t)~_BV(SPN);int32_t p4=sampleAverage(samples);
    ADCSRC|=_BV(SPN);int32_t n4=sampleAverage(samples);
    int16_t o0=(int16_t)((n4-p4)/2);
    if(o0>127) o0=127;
    if(o0<-128) o0=-128;
    OFR0=(uint8_t)(int8_t)o0;

    ADMUX=(uint8_t)(_BV(REFS0)|8u); // AVCC ref, 1/5 VDO
    ADCSRC|=_BV(SPN);int32_t n1=sampleAverage(samples);
    ADCSRC&=(uint8_t)~_BV(SPN);int32_t p1=sampleAverage(samples);
    int16_t o1=(int16_t)((n1-p1)/2);
    if(o1>127) o1=127;
    if(o1<-128) o1=-128;
    OFR1=(uint8_t)(int8_t)o1;

    detail::adcClearFlag();
    ADMUX=saveMux;ADCSRB=saveB;ADCSRD=saveD;
    ADCSRC=(uint8_t)((saveC&~_BV(SPN))|_BV(OFEN));
    ADCSRA=(uint8_t)(saveA&~_BV(ADIF));
    return Ok;
  }

private:
  static inline uint16_t convertRaw(){detail::adcClearFlag();detail::adcUpdate(_BV(ADSC),0);while(ADCSRA&_BV(ADSC)){}return ADC;}
  static inline int32_t sampleAverage(uint8_t n){uint32_t s=0;for(uint8_t i=0;i<n;++i)s+=convertRaw();return (int32_t)(s/n);}
};

}
#endif
