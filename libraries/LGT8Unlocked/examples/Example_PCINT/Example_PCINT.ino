/*
 * Example_PCINT — pin-change interrupts on ANY pin
 * --------------------------------------------------
 *   Connect : (optional) buttons from pin 2 -> GND and pin 3 -> GND.
 *             Without buttons the counters just stay 0 - no crash.
 *   Watch   : Serial Monitor @115200 - press/release counts per pin.
 *   Note    : one callback per pin (this is NOT Arduino attachInterrupt
 *             on INT0/1 - it works on every pin).
 */
#include <LGT8Unlocked.h>

static volatile uint8_t presses[2] = {0, 0};

void onPin2() { presses[0]++; }
void onPin3() { presses[1]++; }

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== PCINT demo ==="));

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  Pcint.attach(2, onPin2, PCINT_CHANGE);
  Pcint.attach(3, onPin3, PCINT_CHANGE);
  Serial.println(F("  watching D2 and D3 (pull-up). Press buttons to GND."));
}

void loop() {
  static uint8_t last[2] = {0, 0};
  bool changed = false;
  for (uint8_t i = 0; i < 2; ++i) {
    if (presses[i] != last[i]) { last[i] = presses[i]; changed = true; }
  }
  if (changed) {
    Serial.print(F("  pin2 presses = ")); Serial.print((int)presses[0]);
    Serial.print(F("   pin3 presses = ")); Serial.println((int)presses[1]);
  }
  delay(50);
}
