// Example_HDR — high-drive GPIO (lgt::HdDrive)
// 328P LQFP32: PD5 (pin 5) & PD6 (pin 6).
// 328P QFP48: tambah PF1/PF2/PF4/PF5.
// 328D: return Unsupported (tidak punya register HDR).
// Koneksi: LED + resistor 220Ω di pin 5 & 6.
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  pinMode(5, OUTPUT);   // PD5
  pinMode(6, OUTPUT);   // PD6

  // Aktifkan high-drive di kedua pin
  HdDrive.enable(lgt::HighDrive_PD5);
  HdDrive.enable(lgt::HighDrive_PD6);

  Serial.print("PD5 high="); Serial.println(HdDrive.enabled(lgt::HighDrive_PD5) ? 1 : 0);
  Serial.print("PD6 high="); Serial.println(HdDrive.enabled(lgt::HighDrive_PD6) ? 1 : 0);
}

void loop() {
  digitalWrite(5, HIGH);   // LED PD5 nyala terang
  digitalWrite(6, LOW);    // LED PD6 mati
  delay(500);
  digitalWrite(5, LOW);
  digitalWrite(6, HIGH);   // gantian
  delay(500);
}
