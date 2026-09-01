#include <LGT8Unlocked.h>
#include <SPI.h>
void setup() {
  Serial.begin(115200); SPI.begin();
  if (lgt::SPIDual::begin() == lgt::Ok) {
    lgt::SPIDualResult r;
    if (lgt::SPIDual::receive2(r) == lgt::Ok) {
      Serial.println(r.first, HEX); Serial.println(r.second, HEX);
    }
    lgt::SPIDual::end();
  }
}
void loop() {}
