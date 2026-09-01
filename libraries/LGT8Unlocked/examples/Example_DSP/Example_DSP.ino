// Example_DSP — aritmatika cepat via uDSC (lgt::Dsp)
// Di 328P otomatis pakai koprosesor uDSC; di 328D pakai AVR native.
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  // Operasi dasar (saturating di 328P)
  int16_t m = Dsp.multiply(30000, 2);   // 60000 → saturate 32767
  int16_t d = Dsp.divide(60000, 7);     // 8571
  int16_t r = Dsp.modulo(60000, 7);     // 8571*7=59997 → sisa 3
  Serial.print("mul="); Serial.println(m);
  Serial.print("div="); Serial.println(d);
  Serial.print("mod="); Serial.println(r);

  // map() versi DSP
  int32_t mp = Dsp.map(512, 0, 1023, -100, 100);   // ≈ 0
  Serial.print("map="); Serial.println(mp);

  // FIR filter / dot product
  int16_t x[4] = {10, 20, 30, 40};
  int16_t h[4] = {1, 1, 1, 1};
  int16_t fir = Dsp.fir(x, h, 4);       // 10+20+30+40 = 100
  Serial.print("fir="); Serial.println(fir);

  // Rata-rata array
  int16_t avg = Dsp.average(x, 4);      // (10+20+30+40)/4 = 25
  Serial.print("avg="); Serial.println(avg);
}

void loop() {}
