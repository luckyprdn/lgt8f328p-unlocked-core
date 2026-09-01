// Example_LVD — low-voltage detector (lgt::Lvd)
// Reset otomatis kalau VCC turun di bawah threshold.
// 328P: 8 level (1.8V..4.4V); 328D: 3 level.
// NOTE: di build recovery-safe, configure() return Locked (bukan error).
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  // Reset kalau VCC < 2.9V
  lgt::Status s = Lvd.setThreshold(lgt::LVD_2V9, true, true);
  Serial.print("lvd_status="); Serial.println((int)s);
  // 0=Ok, 6=Locked (recovery-safe), 1=InvalidArgument

  Serial.println("LVD configured (2.9V, reset enable)");
}

void loop() {}
