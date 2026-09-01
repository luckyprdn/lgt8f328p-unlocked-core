#include <LGT8Unlocked.h>
void setup() {
  Serial.begin(115200);
  lgt::ADCAdvanced::channelFromPin(A0);
  lgt::ADCAdvanced::monitor(1000, 3000, 3);
  lgt::ADCAdvanced::interrupt(false); // poll ADIF in this minimal example
}
void loop() {
  if (lgt::ADCAdvanced::monitorTriggered()) {
    Serial.println(lgt::ADCAdvanced::monitorOverflow() ? "above" : "below");
    lgt::ADCAdvanced::clearInterruptFlag();
    lgt::ADCAdvanced::monitor(1000, 3000, 3); // re-arm
  }
}
