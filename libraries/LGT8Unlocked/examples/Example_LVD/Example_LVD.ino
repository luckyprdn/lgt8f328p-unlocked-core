/*
 * Example_LVD — low-voltage detector
 * -----------------------------------
 *   Connect : nothing.
 *   Watch   : Serial Monitor @115200.
 *   What it does: if VCC drops below the threshold the chip resets
 *   (or can fire an interrupt) - protection against brown-outs.
 *   Silicon : 328P = 8 thresholds 1.8..4.4 V; 328D = 3 levels.
 *             In the recovery-safe build configure() returns Locked -
 *             that is the safety gate doing its job, not an error.
 */
#include <LGT8Unlocked.h>

static const char *statusText(lgt::Status s) {
  switch (s) {
    case lgt::Ok:             return "ok (armed)";
    case lgt::Unsupported:    return "not available on this chip";
    case lgt::Locked:         return "LOCKED by recovery-safe gate (expected)";
    default:                  return "invalid argument";
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== LVD demo ==="));

  Serial.println(F("  Asking for: reset when VCC < 2.9 V"));
  lgt::Status s = Lvd.setThreshold(lgt::LVD_2V9, true, true);
  Serial.print(F("  -> ")); Serial.println(statusText(s));

  Serial.println(F("=== done. Lower VCC below the threshold to see the reset. ==="));
}

void loop() {}
