/*
 * Example_Power — sleep modes and waking up
 * -------------------------------------------
 *   Connect : (optional) button from pin 2 -> GND to wake the deep modes.
 *   Watch   : Serial Monitor @115200.
 *   Silicon : idle / ADC-noise reduction wake on the next interrupt
 *             (Timer0 keeps ticking, so they return by themselves);
 *             power-save stops Timer0 -> needs the pin-2 button.
 *             Measure the current dip with a multimeter in series.
 */
#include <LGT8Unlocked.h>

static void wakeTick() {}   // just wakes the CPU; nothing else to do

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== Power demo ==="));
  pinMode(2, INPUT_PULLUP);
  Pcint.attach(2, wakeTick, PCINT_FALLING);   // wake source for power-save
  Serial.println(F("  (wake pin: D2 pull-up, press to GND for deep sleeps)"));
}

static void sleepIdle()   { Pwr.idle(); }
static void sleepAdcNoise(){ Pwr.adcNoiseReduction(); }
static void sleepSave()   { Pwr.powerSave(); }

static void nap(const char *what, void (*sleepFn)(void), bool needsButton) {
  Serial.print(F("  sleeping: ")); Serial.println(what);
  if (needsButton) Serial.println(F("    (press D2 button to wake...)"));
  uint32_t t0 = micros();
  sleepFn();
  Serial.print(F("    awake after ")); Serial.print((micros() - t0) / 1000ul);
  Serial.println(F(" ms"));
}

void loop() {
  nap("idle (Timer0 wakes it)",        sleepIdle,    false);
  nap("ADC noise reduction",           sleepAdcNoise,false);
  nap("power-save (needs D2 button)",  sleepSave,    true);
  delay(1000);
}
