#include <LGT8Unlocked.h>
void setup() {
  Serial.begin(115200);
  lgt::USART0Advanced::nineBit(true);
  lgt::USART0Advanced::write9(0x155);
}
void loop() {}
