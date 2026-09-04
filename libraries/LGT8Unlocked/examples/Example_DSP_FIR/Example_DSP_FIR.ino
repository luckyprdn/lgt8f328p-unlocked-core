/*
 * Example_DSP_FIR — FIR filter: uDSC SRAM window path vs I/O path
 * ----------------------------------------------------------------
 *   Connect : nothing.
 *   Watch   : Serial Monitor @115200.
 *   Silicon : dotProduct (I/O path) and dotProductFast (direct-SRAM
 *             window) must print IDENTICAL results - that proves the
 *             DSC SRAM alias window works. 328D prints the portable
 *             result (fast path is 328P-only).
 *
 * Timing uses Timer1/64 (started here - it is off by default).
 */
#include <LGT8Unlocked.h>

static const int16_t h[8] = {100, 200, 300, 400, 500, 600, 700, 800};
static int16_t x[8]       = {1, -2, 3, -4, 5, -6, 7, -8};

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== FIR demo: I/O path vs SRAM path ==="));

  int32_t slow = Dsp.dot(x, h, 8);
  int32_t fast = Dsp.dotFast(x, h, 8);
  int32_t fir  = Dsp.firFast(x, h, 8);
  Serial.print(F("  dot      (I/O)  = ")); Serial.println(slow);
  Serial.print(F("  dotFast  (SRAM) = ")); Serial.println(fast);
  Serial.print(F("  firFast         = ")); Serial.println(fir);

#if defined(__LGT8FX8P__)
  TCCR1B = (1 << CS11) | (1 << CS10);   // Timer1 = F_CPU/64
  cli(); uint16_t t0 = TCNT1; sei();
  for (volatile uint16_t k = 0; k < 256; ++k) { volatile int32_t r = Dsp.dot(x, h, 8); (void)r; }
  cli(); uint16_t t1 = TCNT1; sei();
  uint32_t cycSlow = (uint32_t)(uint16_t)(t1 - t0) * 64u;

  cli(); t0 = TCNT1; sei();
  for (volatile uint16_t k = 0; k < 256; ++k) { volatile int32_t r = Dsp.dotFast(x, h, 8); (void)r; }
  cli(); t1 = TCNT1; sei();
  uint32_t cycFast = (uint32_t)(uint16_t)(t1 - t0) * 64u;
  TCCR1B = 0;

  Serial.print(F("  256x dot     = ")); Serial.println(cycSlow);
  Serial.print(F("  256x dotFast = ")); Serial.println(cycFast);
  Serial.print(F("  ratio        = ")); Serial.println((float)cycSlow / (float)cycFast, 2);
  Serial.println((slow == fast && fast == fir) ? F("  PASS: paths agree")
                                               : F("  FAIL: paths differ!"));
#else
  (void)fir;
  Serial.println(F("  (D/E silicon: fast SRAM path unavailable)"));
#endif
  Serial.println(F("=== done. ==="));
}

void loop() { delay(1000); }
