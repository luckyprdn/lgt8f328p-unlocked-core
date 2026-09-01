#include <LGT8Unlocked.h>
void setup() {
#if defined(__LGT8FX8P48__)
  // Pin mux is reversible after reset and remains available.
  lgt::PinMux::oc1aToPF5(true);

  // RC2 safe-profile exception: HDR is explicitly enabled while other recovery-risk features stay locked.
  lgt::DriveStrength::high(lgt::HighDrive_PF5);
#endif
}
void loop() {}
