#include <LGT8Unlocked.h>
void setup() {
#if defined(__LGT8FX8P48__)
  pinMode(D33, OUTPUT); // PF1 / OC3A
  lgt::Timer3::fastPwmICR(1599, 1); // example TOP; derive for your F_CPU/prescaler
  lgt::Timer3::duty(lgt::Timer3::A, 800);
  lgt::Timer3::output(lgt::Timer3::A, true, false);
#endif
}
void loop() {}
