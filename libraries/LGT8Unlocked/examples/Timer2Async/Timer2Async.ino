#include <LGT8Unlocked.h>
void setup() {
  // External 32.768-kHz watch crystals can need up to roughly a second to
  // stabilise. Delay before this call, then use a bounded sync wait so a
  // missing crystal does not hang production firmware forever.
  delay(1000);
  lgt::Status st = lgt::Timer2Async::beginExternal32768Hz(
      0,                 // TCCR2A: normal mode
      _BV(CS22)|_BV(CS20), // TCCR2B: /128 -> nominal 1 Hz overflow
      0, 0xff, 0xff,
      2000000UL);
  if (st != lgt::Ok) {
    // Crystal/update synchronisation failed. Handle according to application.
  }
}
void loop() {}
