/*
 * Example_DSP — 16-bit DSP math, Arduino style
 * ----------------------------------------------
 *   Connect : nothing.
 *   Watch   : Serial Monitor @115200.
 *   Silicon : 328P runs every op on the uDSC coprocessor; 328D uses the
 *             same API with native AVR math. DSP16 is SATURATING:
 *             30000*2 does NOT wrap - it clamps to 32767 (DOC-025/027).
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== DSP demo (16-bit, saturating) ==="));

  Serial.println(F("[1] multiply / divide / modulo"));
  int16_t m = Dsp.multiply(30000, 2);   // would be 60000 -> saturates
  int16_t d = Dsp.divide(60000, 7);
  int16_t r = Dsp.modulo(60000, 7);
  Serial.print(F("    30000 * 2   = ")); Serial.println(m);
  Serial.println(F("    (32767 = clamped, not a bug - 16-bit saturating)"));
  Serial.print(F("    60000 / 7   = ")); Serial.println(d);
  Serial.print(F("    60000 % 7   = ")); Serial.println(r);
  Serial.print(F("    -32768 / -1 = ")); Serial.println(Dsp.divide(-32768, -1));
  Serial.println(F("    (32767 again - INT16_MIN edge saturates, DOC-027)"));

  Serial.println(F("[2] map() 512/1023 -> -100..100"));
  Serial.print(F("    map = ")); Serial.println(Dsp.map(512, 0, 1023, -100, 100));

  Serial.println(F("[3] dot product + FIR filter"));
  int16_t a[4] = {100, -200, 300, -400};
  int16_t b[4] = {2, 3, -4, 5};
  Serial.print(F("    dot     = ")); Serial.println(Dsp.dot(a, b, 4));
  Serial.println(F("    (expect -3600: 200-600-1200-2000)"));
  int16_t x[4] = {10, 20, 30, 40};
  int16_t h[4] = {1, 1, 1, 1};
  Serial.print(F("    fir(avg)= ")); Serial.println(Dsp.fir(x, h, 4));
  Serial.print(F("    average = ")); Serial.println(Dsp.average(x, 4));

  Serial.println(F("=== done. ==="));
}

void loop() {}
