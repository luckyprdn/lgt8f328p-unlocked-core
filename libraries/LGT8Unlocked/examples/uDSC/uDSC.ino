#include <LGT8Unlocked.h>
void setup() {
  Serial.begin(115200);
  uint32_t p = lgt::dsp::mul(30000, 2);
  lgt::dsp::setAccumulator(100000UL);
  uint32_t m = lgt::dsp::mac(20, 30);
  lgt::dsp::DivResult d = lgt::dsp::divmod(100000UL, 300);
  Serial.println(p); Serial.println(m); Serial.println(d.quotient); Serial.println(d.remainder);
}
void loop() {}
