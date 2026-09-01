// Example_OPA — operational amplifier (lgt::Opa)
// HANYA 328D/E (punya OPA0/OPA1). 328P → Unsupported.
// OPA0 pin: ch0=PD7(ACIN1), ch1=PB0(ICP1/OPA1).
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  lgt::Status s = Opa.enable(0, true);
  Serial.print("opa0_enable="); Serial.println((int)s);

  s = Opa.enable(1, true);
  Serial.print("opa1_enable="); Serial.println((int)s);
}

void loop() {
  // OPA output otomatis di-routing ke ADC/comparator
  // (baca lewat ADCAdvanced::channelFromPin atau analogRead)
  delay(1000);
}
