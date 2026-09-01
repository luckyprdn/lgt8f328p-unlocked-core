#include <LGT8Unlocked.h>
void setup() {
  lgt::ADCAdvanced::configurePGA(lgt::PGAPosAPP0, lgt::PGANegAPN0, lgt::Gain16, true);
  lgt::ADCAdvanced::differentialEnable(true);
}
void loop() { int v = analogReadFast(A0); (void)v; }
