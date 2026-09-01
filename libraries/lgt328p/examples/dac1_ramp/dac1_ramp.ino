//============================================
// Larduino w/ 328D
// DAC1 output demo 
// DAC1 output ==> PE5 on board
// NOTE: DAC1 exists only on LGT8F328D/E (modelD variant), not on LGT8F328P
//============================================

#if !defined(__LGT8FX8E__)
#error "dac1_ramp targets the LGT8F328D/E (modelD) variant which has DAC1"
#endif

unsigned char value = 0;

void setup() {
  // put your setup code here, to run once:
  analogReference(INTERNAL2V56);
  pinMode(DAC1, ANALOG);
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(DAC1, value);
  delay(10);
  value += 10;
}

