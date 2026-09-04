/*
 * Example_Timer — Timer1 input capture + Timer2 async clock
 * -----------------------------------------------------------
 *   Connect : (capture) a square wave into pin 8 (PB0/ICP1).
 *             (async) a 32.768 kHz crystal on TOSC pins for precision.
 *   Watch   : Serial Monitor @115200 - prints capture timestamps.
 *   Silicon : Timer2 async uses the internal 32 kHz RC here (no crystal
 *             needed); external crystal path needs TOSC wiring.
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== Timer demo ==="));

  Serial.println(F("[1] Timer2 async 32 kHz (internal RC) ..."));
  lgt::Status s = Timer.asyncInt32K();
  Serial.println(s == lgt::Ok ? F("    async timer running (RTC-style).")
                              : F("    async timer failed."));

  Serial.println(F("[2] Timer1 capture: rising edge + noise cancel ..."));
  Timer.captureEdge(true, true);
  Serial.println(F("    feed a square wave to pin 8 to see timestamps below."));
}

void loop() {
  static uint16_t last = 0;
  uint16_t icr = ICR1;
  if (icr != last) {
    last = icr;
    Serial.print(F("  capture @ ")); Serial.println(icr);
  }
  delay(50);
}
