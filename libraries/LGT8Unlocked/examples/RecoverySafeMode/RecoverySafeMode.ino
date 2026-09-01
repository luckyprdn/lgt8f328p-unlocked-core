#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  Serial.println("LGT8Unlocked recovery-safe build");
#if LGT8_UNLOCKED_RECOVERY_SAFE
  Serial.println("SAFE MODE: ON");
  Serial.println("Blocked: Flash IAP, SWD disable, vector relocation,");
  Serial.println("external/main clock switching, DPS2, LVD writes,");
  Serial.println("runtime configuration reload.");
#else
  Serial.println("SAFE MODE: OFF");
#endif
}
void loop() {}
