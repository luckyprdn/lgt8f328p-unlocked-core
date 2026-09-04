/*
 * Example_RTC — 1 Hz seconds clock that survives power-save
 * -----------------------------------------------------------
 *   Connect : nothing.
 *   Watch   : Serial Monitor @115200 - prints RTC seconds & ticks on.
 *   Silicon : runs on the async Timer2 32 kHz clock (328P; 328D reports
 *             Unsupported - it lacks the internal 32 kHz clock bit).
 *             The async timer keeps counting during power-save, so the
 *             clock survives sleeps. Drift is the internal RC's (~+/-1%);
 *             for precision, wire a 32.768 kHz crystal and use Timer2Async.
 *             NOTE: exclusive with tone()/other Timer2 use.
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== RTC demo ==="));

  lgt::Status s = Rtc.begin();
  if (s != lgt::Ok) {
    Serial.println(F("  RTC unavailable (needs 328P async 32 kHz)."));
    for (;;) delay(1000);
  }
  Rtc.set(0);
  Serial.println(F("  RTC started at 0 s - ticking every second..."));
}

void loop() {
  static uint32_t last = 0;
  uint32_t t = Rtc.seconds();
  if (t != last) {
    last = t;
    Serial.print(F("  t = ")); Serial.print(t);
    Serial.println(F(" s  (survives power-save)"));
  }
  // spend most of the loop asleep; RTC interrupt keeps waking us
  Pwr.idle();
}
