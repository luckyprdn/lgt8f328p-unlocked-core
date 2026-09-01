// HighLevel — demo of Arduino-style high-level abstractions in LGT8Unlocked.
// Uses lgt::analogReadAvg, dspMul/dspMap/dspAverage, dacWriteMillivolts,
// pwmFreqT1/T3, opamp helpers and pinChangeAttach — one-line calls, no
// register access needed.  Compiles on both LGT8F328P and LGT8F328D/E.
#include <LGT8Unlocked.h>

static volatile uint8_t buttonPresses = 0;
static void onButton() { buttonPresses++; }

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("HighLevel demo"));

  // --- ADC: averaged read ---
  uint16_t avg = lgt::analogReadAvg(A0, 16);
  Serial.print(F("analogReadAvg(A0,16)="));
  Serial.println(avg);

  // --- DSP helpers (uDSC on 328P, native on 328D/E) ---
  Serial.print(F("dspMul(30000,2)="));   Serial.println(lgt::dspMul(30000, 2));
  Serial.print(F("dspDiv(60000,2)="));   Serial.println(lgt::dspDiv(60000, 2));
  Serial.print(F("dspMap(512,0,1023,-100,100)="));
  Serial.println(lgt::dspMap(512, 0, 1023, -100, 100));

  int16_t samples[4] = { 10, 20, 30, 40 };
  Serial.print(F("dspAverage="));        Serial.println(lgt::dspAverage(samples, 4));

  // --- DAC in millivolts ---
  lgt::dacWriteMillivolts(0, 1650);      // ~1.65V on DAC0
  Serial.print(F("DAC0 set to 1650mV"));

  // --- PWM frequency (Timer1 on pin 9) ---
  uint8_t cs = lgt::pwmFreqT1(1000);     // 1 kHz on D9
  Serial.print(F("pwmFreqT1(1000) prescaler="));
  Serial.println(cs);
  analogWrite(9, 128);

  // --- OPA (328D/E only; returns Unsupported on P) ---
  lgt::Status st = lgt::opamp0Enable(true);
  Serial.print(F("opamp0Enable status="));
  Serial.println((int)st);

  // --- Pin-change interrupt with callback ---
  pinMode(2, INPUT_PULLUP);
  lgt::Status pc = lgt::pinChangeAttach(2, onButton, 0); // CHANGE mode
  Serial.print(F("pinChangeAttach status="));
  Serial.println((int)pc);

  Serial.println(F("READY"));
}

void loop() {
  if (buttonPresses) {
    Serial.print(F("button presses="));
    Serial.println((int)buttonPresses);
    buttonPresses = 0;
  }
  delay(200);
}