/*
 * Example_DAC — precise analog output in millivolts
 * --------------------------------------------------
 *   Connect : voltmeter on the DAC0 output pin.
 *   Watch   : Serial Monitor @115200.
 *   Silicon : DAC0 on 328P and 328D; DAC1 is 328D-only.
 *             writeMillivolt() hides the 8-bit math (0..3.3 V).
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== DAC demo ==="));

  Serial.println(F("[1] DAC0 = 1.65 V (mid-scale) for 2 s ..."));
  Dac.writeMillivolt(0, 1650);
  delay(2000);

  Serial.println(F("[2] DAC0 = 0.10 V ..."));
  Dac.writeMillivolt(0, 100);
  delay(1000);

  Serial.println(F("[3] DAC1 = 0.50 V (328D/E only; no-op elsewhere) ..."));
  Dac.writeMillivolt(1, 500);
  delay(1000);

  Serial.println(F("[4] both off (0 V) ..."));
  Dac.writeMillivolt(0, 0);
  Dac.writeMillivolt(1, 0);
  Serial.println(F("=== done. Measure the steps with a voltmeter. ==="));
}

void loop() {}
