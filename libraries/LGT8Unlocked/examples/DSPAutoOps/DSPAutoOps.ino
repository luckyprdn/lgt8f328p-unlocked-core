// DSPAutoOps — demo of transparent uDSC arithmetic via lgt::dsp::DSP16
// Write z = a * b + c and the uDSC coprocessor does the math automatically.
// On LGT8F328D/E the same code uses native AVR arithmetic (portable).
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  delay(200);

  using lgt::dsp::DSP16;

  DSP16 a = 30000;
  DSP16 b = 2;
  DSP16 c = 5000;

  DSP16 p = a * b;               // uDSC multiply: 30000 * 2 = 60000 (saturated)
  DSP16 s = a * b + c;           // uDSC multiply then add: 30000*2 + 5000 = 65000
  DSP16 d = p / b;               // uDSC divmod: 60000 / 2 = 30000
  DSP16 r = p % 7;               // uDSC divmod: 60000 % 7 = 4

  Serial.print("a*b=");          Serial.println((int)p.v);
  Serial.print("a*b+c=");        Serial.println((int)s.v);
  Serial.print("(a*b)/b=");      Serial.println((int)d.v);
  Serial.print("(a*b)%7=");      Serial.println((int)r.v);

  // Saturating MAC pipeline
  DSP16 acc = 0;
  for (uint8_t i = 0; i < 10; i++) {
    DSP16 x = (int16_t)(i + 1);
    DSP16 y = (int16_t)(i + 1);
    acc.mac(x, y);               // acc += x * y (1 uDSC instruction each)
  }
  // acc = 1*1 + 2*2 + ... + 10*10 = 385
  Serial.print("sum_of_squares="); Serial.println((int)acc.v);

  // Signed division
  DSP16 neg = -100;
  DSP16 q = neg / 7;             // -100 / 7 = -14
  DSP16 m = neg % 7;             // -100 % 7 = -2
  Serial.print("-100/7=");       Serial.println((int)q.v);
  Serial.print("-100%7=");       Serial.println((int)m.v);

  Serial.println("DSP_AUTO_DONE");
}

void loop() {}