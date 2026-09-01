// Example_HDR — high-drive GPIO (lgt::HdDrive)
// 328P: 6 pin dukungan high-drive (PD5/PD6/PF1/PF2/PF4/PF5).
// 328D: return Unsupported.
// Koneksi: LED + resistor kecil di PD5 (pin 5) → test driver kuat.
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  pinMode(5, OUTPUT);   // PD5

  // Aktifkan high-drive
  lgt::Status s = HdDrive.enable(lgt::HighDrive_PD5);
  Serial.print("enable_status="); Serial.println((int)s);
  Serial.print("is_high="); Serial.println(HdDrive.enabled(lgt::HighDrive_PD5) ? 1 : 0);
}

void loop() {
  digitalWrite(5, HIGH);   // LED nyala terang (drive lebih kuat)
  delay(500);
  digitalWrite(5, LOW);
  delay(500);
}
