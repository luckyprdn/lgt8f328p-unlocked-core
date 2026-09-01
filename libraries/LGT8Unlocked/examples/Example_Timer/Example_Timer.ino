// Example_Timer — Timer1 capture + Timer2 async (lgt::Timer)
// Timer1: input capture di pin 8 (PB0/ICP1) — semua silicon.
// Timer2: async 32kHz internal / 32768Hz external — semua silicon.
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  // Timer2 async — jam internal 32 kHz (untuk RTC presisi)
  lgt::Status s = Timer.asyncInt32K();
  Serial.print("async_int32k="); Serial.println((int)s);

  // Timer1 capture: rising edge + noise cancel
  Timer.captureEdge(true, true);
  Serial.println("capture ready (rising + noise cancel)");
}

void loop() {
  // Baca Timer1 counter & ICR1 (jarak 2 edge)
  static uint16_t last = 0;
  uint16_t icr = ICR1;
  if (icr != last) {
    last = icr;
    Serial.print("icr="); Serial.println(icr);
  }
  delay(100);
}
