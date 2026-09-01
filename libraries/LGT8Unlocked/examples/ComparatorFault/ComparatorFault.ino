#include <LGT8Unlocked.h>
void setup() {
  lgt::Comparator0::positive(lgt::AC0P);
  lgt::Comparator0::negative(lgt::AC0ADCMux);
  lgt::Comparator0::hysteresis(true);
  lgt::Comparator0::filter(lgt::Filter32us);
  lgt::Comparator0::enable(true);
  lgt::Timer3::faultSources(lgt::T13FaultComparator0);
  lgt::Timer3::protect(lgt::Timer3::A, true);
  lgt::Timer3::protect(lgt::Timer3::B, true);
}
void loop() {}
