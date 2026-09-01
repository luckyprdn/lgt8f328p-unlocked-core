// Example_ADC_Throughput — ukur sample rate analogRead vs analogReadFast
// vs high-speed mode (SPD bit), 12-bit resolution.
//
// P only meaningful; D compiles (fast path = same as normal read there).
// Prints KSPS per path so hardware validation can compare against the
// datasheet 3MHz ADC clock claim.
#include <LGT8Unlocked.h>

#define N 64

void setup() {
  Serial.begin(115200);
  delay(200);

  uint16_t buf[N];
  (void)buf;  // kept for future buffer-based burst timing
  uint32_t t0, t1;

  Serial.println(F("--- ADC throughput (KSPS) ---"));

  // analogRead (10-bit path, Arduino default)
  cli(); t0 = TCNT1; sei();
  for (uint8_t i = 0; i < N; ++i) { volatile uint16_t v = analogRead(A0); (void)v; }
  cli(); t1 = TCNT1; sei();
  Serial.print(F("analogRead:      "));
  Serial.println(1000.0f * (float)N * 32.0f / (float)(t1 - t0), 1);

  // analogReadFast (single conversion, 12-bit)
  cli(); t0 = TCNT1; sei();
  for (uint8_t i = 0; i < N; ++i) { volatile uint16_t v = analogReadFast(A0); (void)v; }
  cli(); t1 = TCNT1; sei();
  Serial.print(F("analogReadFast:  "));
  Serial.println(1000.0f * (float)N * 32.0f / (float)(t1 - t0), 1);

  // high-speed mode (SPD) — datasheet claims faster conversion clock
  lgt::ADCAdvanced::highSpeed(true);
  cli(); t0 = TCNT1; sei();
  for (uint8_t i = 0; i < N; ++i) { volatile uint16_t v = analogReadFast(A0); (void)v; }
  cli(); t1 = TCNT1; sei();
  lgt::ADCAdvanced::highSpeed(false);
  Serial.print(F("highSpeed+Fast:  "));
  Serial.println(1000.0f * (float)N * 32.0f / (float)(t1 - t0), 1);

  // 8-bit left-justified fast path (ADLAR) for max throughput
  ADMUX |= _BV(ADLAR);
  cli(); t0 = TCNT1; sei();
  for (uint8_t i = 0; i < N; ++i) { volatile uint16_t v = analogReadFast(A0); (void)v; }
  cli(); t1 = TCNT1; sei();
  ADMUX &= (uint8_t)~_BV(ADLAR);
  Serial.print(F("8-bit Fast:      "));
  Serial.println(1000.0f * (float)N * 32.0f / (float)(t1 - t0), 1);

  Serial.println(F("(cycle estimate: 32MHz / ticks, 1 tick = 1 timer1 count)"));
}

void loop() { delay(1000); }
