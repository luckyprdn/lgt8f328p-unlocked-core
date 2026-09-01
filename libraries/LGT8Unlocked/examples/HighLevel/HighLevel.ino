// HighLevel — Arduino-style high-level API demo.
// Pakai kayak Wire / Serial / SPI: objek.DOT().
#include <LGT8Unlocked.h>

static volatile uint8_t clicks = 0;
static void onButton() { clicks++; }

void setup() {
  Serial.begin(115200);

  // 1. ADC — rata-rata + monitor
  uint16_t v = AdcExt.readAverage(A0, 16);
  Serial.print("avg="); Serial.println(v);

  // 2. DSP — uDSC di P, native di D
  Serial.print("mul="); Serial.println(Dsp.multiply(30000, 2));
  Serial.print("map="); Serial.println(Dsp.map(512, 0, 1023, -100, 100));
  int16_t xv[4] = {10, 20, 30, 40};
  int16_t h[4]  = {1, 1, 1, 1};
  Serial.print("fir="); Serial.println(Dsp.fir(xv, h, 4));

  // 3. DAC — millivolt
  Dac.writeMillivolt(0, 1650);            // DAC0 1.65V

  // 4. PWM — frekuensi Timer1 (pin 9) + dead-time Timer3
  Pwm.timer1Frequency(1000);
  analogWrite(9, 128);
  Pwm.deadTime3(5, 5);                    // P only

  // 5. HDR — high drive (P only; D → Unsupported)
  pinMode(5, OUTPUT);
  HdDrive.enable(lgt::HighDrive_PD5);

  // 6. Comparator
  Comp.enable(0, true);
  Comp.setEdge(0, 1);                     // 1=RISING

  // 7. USART — 9-bit
  Usart.enable9Bit(true);

  // 8. System / Power / LVD
  Serial.print("id="); Serial.println(Sys.chipId(), HEX);
  Pwr.idle();
  Lvd.setThreshold(lgt::LVD_2V9);         // P only

  // 9. PCINT — pin-change interrupt
  pinMode(2, INPUT_PULLUP);
  Pcint.attach(2, onButton, PCINT_CHANGE);
}

void loop() {
  if (clicks) { Serial.println((int)clicks); clicks = 0; }
  delay(100);
}
