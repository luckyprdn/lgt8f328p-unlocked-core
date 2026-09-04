/*
 * Example_SPIDual — SPI dual receive (2 bits per clock)
 * ------------------------------------------------------
 *   Connect : a device driving BOTH the MISO (PB4) and MOSI (PB3) pins
 *             as data lines, SCK from this master.
 *   Watch   : Serial Monitor @115200 - prints two bytes per transfer.
 *   Silicon : 328P only (SPFR feature); 328D reports Unsupported.
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== SPI dual receive demo ==="));

  lgt::Status s = SpiDual.begin();
  if (s != lgt::Ok) {
    Serial.println(F("  SPI dual not available on this silicon."));
    return;
  }
  Serial.println(F("  SPI dual armed - reading 2x2 bytes..."));

  lgt::SPIDualResult r = SpiDual.receive2();
  Serial.print(F("  byte A = 0x")); Serial.println(r.first, HEX);
  Serial.print(F("  byte B = 0x")); Serial.println(r.second, HEX);

  uint8_t buf[4];
  s = SpiDual.receive(buf, sizeof(buf));
  Serial.println(s == lgt::Ok ? F("  bulk receive of 4 bytes ok.")
                              : F("  bulk receive failed."));

  SpiDual.end();
  Serial.println(F("=== done. Feed both data lines to see values. ==="));
}

void loop() {}
