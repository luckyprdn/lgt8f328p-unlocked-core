#include <LGT8Unlocked.h>
int main(){
  lgt::ADCAdvanced::monitor(1,2,1);
  lgt::Timer3::deadTime(1,2);
  lgt::Comparator0::filter(lgt::Filter32us);
  (void)lgt::dsp::mul(1,2);
  (void)lgt::Clock::prescaler(0);
  lgt::LVD::configure(lgt::LVD_2V9);
  lgt::USARTSPI0::end();
  return 0;
}
