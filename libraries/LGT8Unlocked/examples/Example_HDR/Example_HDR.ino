/*
 * Example_HDR — high-drive pins for LEDs / relays
 * ------------------------------------------------
 *   Connect : LED + 220R on pin 5 (PD5) and pin 6 (PD6) [LQFP32],
 *             plus PF1/PF2/PF4/PF5 on QFP48.
 *   Watch   : LEDs blink alternately, noticeably brighter than normal.
 *   Silicon : HDR pins source up to ~20 mA (normal GPIO ~4 mA).
 *             328D has no HDR register -> status Unsupported.
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== HDR demo: high-drive GPIO ==="));

  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  lgt::Status a = HdDrive.enable(lgt::HighDrive_PD5);
  lgt::Status b = HdDrive.enable(lgt::HighDrive_PD6);
  Serial.print(F("  PD5 high-drive: "));
  Serial.println(a == lgt::Ok ? F("on") : F("unsupported"));
  Serial.print(F("  PD6 high-drive: "));
  Serial.println(b == lgt::Ok ? F("on") : F("unsupported"));
  Serial.print(F("  PD5 enabled() = "));
  Serial.println(HdDrive.enabled(lgt::HighDrive_PD5) ? F("yes") : F("no"));

  Serial.println(F("  Blinking pin 5 & 6 alternately (10x)..."));
}

void loop() {
  static uint8_t n = 0;
  if (n >= 10) {
    HdDrive.disable(lgt::HighDrive_PD5);
    HdDrive.disable(lgt::HighDrive_PD6);
    digitalWrite(5, LOW); digitalWrite(6, LOW);
    Serial.println(F("  done - drive restored to normal."));
    for (;;) delay(1000);
  }
  n++;
  digitalWrite(5, HIGH); digitalWrite(6, LOW);
  delay(300);
  digitalWrite(5, LOW); digitalWrite(6, HIGH);
  delay(300);
}
