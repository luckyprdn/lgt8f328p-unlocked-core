/*
 * Example_OPA — built-in op-amps (328D/E only)
 * ----------------------------------------------
 *   Connect : signal into the OPA input pin (see 328D pinout).
 *   Watch   : Serial Monitor @115200.
 *   Silicon : 328D/E carry OPA0/OPA1; the 328P has none, so
 *             Opa.enable() reports Unsupported there - expected.
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== OPA demo (328D/E) ==="));

  lgt::Status s0 = Opa.enable(0, true);
  Serial.print(F("  OPA0: "));
  Serial.println(s0 == lgt::Ok ? F("enabled") : s0 == lgt::Unsupported ? F("not on this chip (328P)") : F("failed"));

  lgt::Status s1 = Opa.enable(1, true);
  Serial.print(F("  OPA1: "));
  Serial.println(s1 == lgt::Ok ? F("enabled") : s1 == lgt::Unsupported ? F("not on this chip (328P)") : F("failed"));

  Serial.println(F("=== done. On 328D, route OPA output into the ADC and read it. ==="));
}

void loop() { delay(1000); }
