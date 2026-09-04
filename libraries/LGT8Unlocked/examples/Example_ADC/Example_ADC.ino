/*
 * Example_ADC — 12-bit ADC: averaging, window monitor, PGA gain
 * --------------------------------------------------------------
 *   Connect : A0 -> potentiometer / analog sensor (optional).
 *             Floating A0 is detected and reported, not trusted.
 *   Watch   : Serial Monitor @115200.
 *   Silicon : 328P = 12-bit ADC, PGA x1/x8/x16/x32 + offset cal.
 *             328D = ADC without PGA (status prints Unsupported).
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== ADC demo (12-bit) ==="));

  analogReadResolution(12);

  // 1) Single + averaged readings
  Serial.println(F("[1] reading A0 ..."));
  uint16_t one  = analogRead(A0);
  uint16_t avg  = AdcExt.readAverage(A0, 16);
  Serial.print(F("    single      = ")); Serial.println(one);
  Serial.print(F("    avg of 16   = ")); Serial.println(avg);
  int16_t spread = one > avg ? one - avg : avg - one;
  if (spread > 60)
    Serial.println(F("    -> A0 looks FLOATING: tie it to GND or VCC, then reset."));
  else
    Serial.println(F("    -> stable source (0..4095 = 0..VCC)."));

  // 2) Window monitor: true while the reading stays in [low..high]
  Serial.println(F("[2] window monitor  [100..200] ..."));
  Serial.print(F("    in-window = "));
  Serial.println(AdcExt.monitor(A0, 100, 200) ? F("yes") : F("no"));

  // 3) PGA gain x16 then back to x1 (328P only)
  Serial.println(F("[3] PGA gain x16 -> x1 ..."));
  lgt::Status s = AdcExt.setGain(lgt::Gain16);
  if (s == lgt::Ok) {
    Serial.println(F("    PGA x16 ok (amplifies small analog signals)."));
    AdcExt.setGain(lgt::Gain1);
    Serial.println(F("    restored to x1."));
  } else if (s == lgt::Unsupported) {
    Serial.println(F("    PGA not available on 328D - skipped."));
  } else {
    Serial.println(F("    PGA locked by recovery-safe gate."));
  }

  // 4) Offset calibration (328P only)
  Serial.println(F("[4] ADC offset calibration ..."));
  s = AdcExt.calibrate(8);
  Serial.print(F("    calibration "));
  Serial.println(s == lgt::Ok ? F("ok") : s == lgt::Unsupported ? F("n/a on 328D") : F("failed/locked"));

  Serial.println(F("=== done. Re-run with a wired A0 for real readings. ==="));
}

void loop() {}
