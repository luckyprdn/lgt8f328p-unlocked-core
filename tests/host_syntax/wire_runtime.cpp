#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <Wire.h>

volatile uint8_t __fake_mem8[512];
static unsigned calls_read=0,calls_write=0,calls_flush=0;
static uint8_t last_mask=0; static bool last_gc=false;

extern "C" {
void twi_init(void){}
void twi_disable(void){}
void twi_setAddress(uint8_t){}
void twi_setAddressMask(uint8_t v){last_mask=v;}
void twi_setGeneralCall(bool v){last_gc=v;}
void twi_setFrequency(uint32_t){}
void twi_flush(void){++calls_flush;}
uint8_t twi_readFrom(uint8_t addr,uint8_t *p,uint8_t n,uint8_t){
  ++calls_read; if(addr>0x7f||!p)return 0; for(uint8_t i=0;i<n;++i)p[i]=(uint8_t)(i+1); return n;
}
uint8_t twi_writeTo(uint8_t addr,uint8_t*,uint8_t,uint8_t,uint8_t){++calls_write;return addr>0x7f?4:0;}
uint8_t twi_transmit(const uint8_t*,uint8_t){return 0;}
void twi_attachSlaveRxEvent(void (*)(uint8_t*,int)){}
void twi_attachSlaveTxEvent(void (*)(void)){}
void twi_reply(uint8_t){}
void twi_stop(void){}
void twi_releaseBus(void){}
void twi_setTimeoutInMicros(uint32_t,bool){}
void twi_handleTimeout(bool){}
bool twi_manageTimeoutFlag(bool){return false;}
}

int main(){
  Wire.begin();
  assert(Wire.requestFrom((uint8_t)0x42,(uint8_t)0)==0 && calls_read==0);
  assert(Wire.requestFrom((uint8_t)0x80,(uint8_t)1)==0 && calls_read==0);
  assert(Wire.requestFrom((uint8_t)0x42,(uint8_t)3)==3 && calls_read==1);
  assert(Wire.available()==3); assert(Wire.read()==1); assert(Wire.peek()==2);

  Wire.beginTransmission((uint8_t)0x80u);
  assert(Wire.write((uint8_t)0xaa)==1);
  assert(Wire.endTransmission()==4 && calls_write==0);

  Wire.clearWriteError();
  Wire.beginTransmission((uint8_t)0x42u);
  uint8_t payload[WIRE_BUFFER_LENGTH+5];
  for(size_t i=0;i<sizeof(payload);++i)payload[i]=(uint8_t)i;
  size_t accepted=Wire.write(payload,sizeof(payload));
  assert(accepted==WIRE_BUFFER_LENGTH);
  assert(Wire.getWriteError()!=0);
  assert(Wire.endTransmission()==0 && calls_write==1);

  Wire.setAddressMask(0x55); assert(last_mask==0x55);
  Wire.enableGeneralCall(true); assert(last_gc);
  Wire.flush(); assert(calls_flush==1);
  return 0;
}
