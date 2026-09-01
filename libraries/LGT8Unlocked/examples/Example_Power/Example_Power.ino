// Example_Power — sleep modes hemat daya (lgt::Pwr)
// Mode diurut dari paling hemat: idle < ADC noise < save < power-down.
#include <LGT8Unlocked.h>

volatile bool awake = false;
void wakeISR() { awake = true; }

void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);
  Pcint.attach(2, wakeISR, PCINT_FALLING);   // bangun dari pin 2
  Serial.println("press button on pin 2 to wake");
  delay(200);
}

void loop() {
  Serial.println("sleeping: idle 2s");
  delay(2000);
  Pwr.idle();                 // tidur sampai interrupt
  Serial.println("awake (idle)");

  Serial.println("sleeping: ADC noise reduction 2s");
  delay(2000);
  Pwr.adcNoiseReduction();    // tidur sambil tunggu ADC
  Serial.println("awake (adc)");

  Serial.println("sleeping: power-save 2s");
  delay(2000);
  Pwr.powerSave();            // timer2 async tetap jalan
  Serial.println("awake (save)");
}
