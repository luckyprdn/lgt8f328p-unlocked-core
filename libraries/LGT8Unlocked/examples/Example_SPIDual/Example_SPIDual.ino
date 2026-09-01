// Example_SPIDual — SPI dual (MOSI+SCK terbalik utk baca 2 byte/clock)
// HANYA 328P (fitur SPFR). 328D → Unsupported.
// Koneksi: dua slave memakai MISO (PB4) & MOSI (PB3) sebagai input.
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  lgt::Status s = SpiDual.begin();
  Serial.print("begin="); Serial.println((int)s);
  if (s != lgt::Ok) return;

  // Terima 2 byte sekaligus
  lgt::SPIDualResult r = SpiDual.receive2();
  Serial.print("first="); Serial.println(r.first, HEX);
  Serial.print("second="); Serial.println(r.second, HEX);

  // Atau buffer
  uint8_t buf[4];
  s = SpiDual.receive(buf, 4);
  Serial.print("receive="); Serial.println((int)s);

  SpiDual.end();
}

void loop() {}
