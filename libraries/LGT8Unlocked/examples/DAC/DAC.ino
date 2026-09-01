#include <LGT8Unlocked.h>
void setup() {
  lgt::DAC0Advanced::begin(lgt::DACInternal, true);
  lgt::DAC0Advanced::write(127);
}
void loop() {}
