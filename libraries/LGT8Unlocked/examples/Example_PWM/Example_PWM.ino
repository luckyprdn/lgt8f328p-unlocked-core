// Example_PWM — PWM frekuensi + dead-time (lgt::Pwm)
// Timer1: pin 9 (PB1/OC1A) — semua silicon.
// Timer3: D33/D34/D35 (PF1/PF2/PF3) — hanya QFP48 328P.
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  // Timer1 PWM 1 kHz di pin 9
  uint8_t cs = Pwm.timer1Frequency(1000);
  analogWrite(9, 128);                 // 50% duty
  Serial.print("timer1_prescaler="); Serial.println(cs);

  // Timer3 PWM 25 kHz di D33 (PF1/OC3A) — QFP48 328P only
  lgt::Status s = Pwm.timer3Frequency(25000);
  Serial.print("timer3_status="); Serial.println((int)s);
  if (s == lgt::Ok) {
    pinMode(33, OUTPUT);
    lgt::Timer3::output(lgt::Timer3::A, true, false);
    lgt::Timer3::duty(lgt::Timer3::A, 512);      // 25kHz, ~50%
  }

  // Dead-time 5 tick antara OC3A/OC3B (anti shoot-through, P only)
  s = Pwm.deadTime3(5, 5);
  Serial.print("deadtime3_status="); Serial.println((int)s);
}

void loop() {}
