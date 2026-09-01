#include <LGT8Unlocked.h>
void setup() {
  Serial.begin(115200);
  lgt::DeviceId id = lgt::System::guid();
  Serial.print("GUID: ");
  for (uint8_t i=0;i<4;i++){ if(id.b[i]<16) Serial.print('0'); Serial.print(id.b[i],HEX); }
  Serial.print(" reset=0x"); Serial.println(lgt::System::resetCause(),HEX);
  Serial.print("HFRC cal=0x"); Serial.println(lgt::Clock::hfCalibration(),HEX);
  Serial.print("LFRC cal=0x"); Serial.println(lgt::Clock::lfCalibration(),HEX);
}
void loop() {}
