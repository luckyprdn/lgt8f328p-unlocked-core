#include <LGT8Unlocked.h>

static void printStatus(const char *name, lgt::Status st) {
  Serial.print(name);
  Serial.print(": ");
  Serial.println((int)st);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("LGT8F328P Unlocked RC2 recovery-safe smoke test");

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13, LOW);

  lgt::DeviceId id = lgt::System::guid();
  Serial.print("GUID=");
  for (uint8_t i = 0; i < 4; ++i) {
    if (id.b[i] < 16) Serial.print('0');
    Serial.print(id.b[i], HEX);
  }
  Serial.print(" reset=0x");
  Serial.println(lgt::System::resetCause(), HEX);

  uint32_t product = lgt::dsp::mul(30000, 2);
  Serial.print("uDSC 30000*2=");
  Serial.println(product);

  printStatus("ADC A0 select", lgt::ADCAdvanced::channelFromPin(A0));

  // Prove dangerous recovery-sensitive paths are interlocked without executing them.
  printStatus("SWD disable lock", lgt::System::disableSWD());
  printStatus("clock switch lock", lgt::Clock::select(lgt::ExternalHigh));
  printStatus("LVD mutation lock", lgt::LVD::configure(lgt::LVD_2V9));
  printStatus("DPS2 lock", lgt::Power::dps2());

  Serial.println("SMOKE_DONE");
}

void loop() {
  static uint8_t level = LOW;
  level = (level == LOW) ? HIGH : LOW;
  digitalWrite(13, level);
  delay(500);
}
