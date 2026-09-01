// Example_DAC — output analog presisi dalam millivolt (lgt::Dac)
// DAC0 di pin analog out (QFP32/QFP48). 328D juga punya DAC1.
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  // 1.65V di DAC0 (referensi default 3.3V)
  Dac.writeMillivolt(0, 1650);
  Serial.println("DAC0 = 1.65V");

  // 0.5V di DAC1 (hanya 328D/E)
  Dac.writeMillivolt(1, 500);
  Serial.println("DAC1 = 0.50V (328D only)");

  delay(2000);
  // Matikan: 0V
  Dac.writeMillivolt(0, 0);
}

void loop() {}
