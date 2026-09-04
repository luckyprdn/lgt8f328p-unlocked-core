/*
 * Example_PWM — PWM frequency control + Timer3 + dead-time
 * ----------------------------------------------------------
 *   Connect : LED + 220R on pin 9 (OC1A). QFP48: pin 33/PF1 for Timer3.
 *   Watch   : Serial Monitor @115200; brightness/frequency on the LED.
 *   Silicon : Timer1 = any chip. Timer3 = 328P QFP48 only.
 *             Dead-time (anti shoot-through) = 328P only.
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== PWM demo ==="));

  Serial.println(F("[1] Timer1 PWM, 1 kHz, 50% on pin 9 ..."));
  uint8_t cs = Pwm.timer1Frequency(1000);
  analogWrite(9, 128);
  Serial.print(F("    prescaler code = ")); Serial.println(cs);
  Serial.println(F("    (see it on pin 9 with an LED or scope)"));

  Serial.println(F("[2] Timer3 PWM, 25 kHz (328P QFP48) ..."));
  lgt::Status s = Pwm.timer3Frequency(25000);
  if (s == lgt::Ok) {
    pinMode(33, OUTPUT);                    // PF1 / OC3A
    lgt::Timer3::output(lgt::Timer3::A, true, false);
    lgt::Timer3::duty(lgt::Timer3::A, 512); // ~50%
    Serial.println(F("    25 kHz on pin 33."));
  } else {
    Serial.println(F("    Timer3 n/a on this chip/package."));
  }

  Serial.println(F("[3] dead-time 5 ticks (328P only) ..."));
  s = Pwm.deadTime3(5, 5);
  Serial.println(s == lgt::Ok ? F("    dead-time armed between OC3A/OC3B.")
                              : F("    n/a here."));

  Serial.println(F("=== done. ==="));
}

void loop() {}
