/*
 * Example_ADC_Throughput — how fast can we sample?
 * -------------------------------------------------
 *   Connect : A0 -> GND or VCC (any stable level).
 *   Watch   : Serial Monitor @115200. Numbers are kilo-samples/sec.
 *   Silicon : 328P has a 12-bit ADC plus a high-speed (SPD) clock bit;
 *             328D runs the same code without the SPD advantage.
 *
 * Timer1 is clocked at /64 for the measurement (it is NOT running by
 * default - the old version of this example forgot to start it!).
 */
#include <LGT8Unlocked.h>

#define N 64

static uint32_t measureTicks(void (*sample)(void)) {
  TCCR1B = (1 << CS11) | (1 << CS10);      // Timer1 = F_CPU/64 (2 us/tick)
  cli(); uint16_t t0 = TCNT1; sei();
  sample();
  cli(); uint16_t t1 = TCNT1; sei();
  TCCR1B = 0;                              // stop Timer1 again
  return (uint32_t)(uint16_t)(t1 - t0) * 64u;   // in CPU cycles
}

static void readNormal()   { for (uint8_t i = 0; i < N; ++i) { volatile uint16_t v = analogRead(A0); (void)v; } }
static void readFast()     { for (uint8_t i = 0; i < N; ++i) { volatile uint16_t v = analogReadFast(A0); (void)v; } }
static void readHighFast() { lgt::ADCAdvanced::highSpeed(true); for (uint8_t i = 0; i < N; ++i) { volatile uint16_t v = analogReadFast(A0); (void)v; } lgt::ADCAdvanced::highSpeed(false); }
static void read8bit()     { ADMUX |= _BV(ADLAR); for (uint8_t i = 0; i < N; ++i) { volatile uint16_t v = analogReadFast(A0); (void)v; } ADMUX &= (uint8_t)~_BV(ADLAR); }

static void report(const char *name, uint32_t cyc) {
  Serial.print(F("  ")); Serial.print(name);
  Serial.print(F(" : "));
  Serial.print(1000.0f * (float)N * 32.0f / (float)cyc, 1);
  Serial.println(F(" kS/s"));
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== ADC throughput (kilo-samples/second) ==="));
  analogReadResolution(12);

  report("analogRead (Arduino)",     measureTicks(readNormal));
  report("analogReadFast",           measureTicks(readFast));
  report("highSpeed + Fast (328P)",  measureTicks(readHighFast));
  report("8-bit left-just + Fast",   measureTicks(read8bit));

  Serial.println(F("=== done (higher = faster). ==="));
}

void loop() { delay(1000); }
