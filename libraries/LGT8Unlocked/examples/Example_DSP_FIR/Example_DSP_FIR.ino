// Example_DSP_FIR — uDSC FIR benchmark: dotProduct (I/O path) vs
// dotProductFast (direct-SRAM ld r0,Z path).
//
// LGT8F328P: prints both results (must be identical) and a coarse cycle
// estimate via loop-count timing.  LGT8F328D/E: prints the portable
// fallback result (fast path returns 0 there — DSP is P-only).
//
// Hardware validation aid: identical dot/fast results prove the DSC
// direct-SRAM alias window (0x2000 + addr) works on real silicon.
#include <LGT8Unlocked.h>

static const int16_t h[8]  = { 100, 200, 300, 400, 500, 600, 700, 800 };
static int16_t x[8]        = { 1, -2, 3, -4, 5, -6, 7, -8 };

void setup() {
  Serial.begin(115200);
  delay(200);

  int32_t slow = Dsp.dot(x, h, 8);
  int32_t fast = Dsp.dotFast(x, h, 8);
  int32_t fir  = Dsp.firFast(x, h, 8);

  Serial.println(F("--- uDSC FIR benchmark ---"));
  Serial.print(F("dot (I/O path)      = ")); Serial.println(slow);
  Serial.print(F("dotFast (SRAM path) = ")); Serial.println(fast);
  Serial.print(F("firFast             = ")); Serial.println(fir);

#if defined(__LGT8FX8P__)
  // Rough timing: 256 iterations of each path, timed with a free-running
  // counter.  On 32MHz the count difference is a proxy for cycles saved.
  uint32_t t0, t1, dt_slow, dt_fast;
  cli(); t0 = TCNT1; sei();
  for (volatile uint16_t k = 0; k < 256; ++k) { volatile int32_t r = Dsp.dot(x, h, 8); (void)r; }
  cli(); t1 = TCNT1; sei(); dt_slow = (uint32_t)(t1 - t0);

  cli(); t0 = TCNT1; sei();
  for (volatile uint16_t k = 0; k < 256; ++k) { volatile int32_t r = Dsp.dotFast(x, h, 8); (void)r; }
  cli(); t1 = TCNT1; sei(); dt_fast = (uint32_t)(t1 - t0);

  Serial.print(F("256x dot:    ")); Serial.println(dt_slow);
  Serial.print(F("256x dotFast:")); Serial.println(dt_fast);
  Serial.print(F("speedup:     ")); Serial.println((float)dt_slow / (float)dt_fast, 2);
  Serial.println(F("PASS if dot == dotFast == firFast"));
  Serial.println((slow == fast && fast == fir) ? F("PASS") : F("FAIL"));
#else
  (void)fir;
  Serial.println(F("(D/E silicon: DSP fast path unavailable)"));
#endif
}

void loop() { delay(1000); }
