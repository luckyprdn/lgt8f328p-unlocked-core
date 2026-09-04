/*
 * Example_SPI_Dual_Throughput — SPI dual speed test
 * --------------------------------------------------
 *   Connect : MISO (PB4) to MOSI (PB3) for loopback.
 *   Watch   : Serial Monitor @115200 - tick counts per path.
 *   Silicon : 328P only. Timer1/64 is started here for timing.
 */
#include <SPI.h>
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== SPI dual throughput (Timer1 ticks @/64) ==="));

  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  TCCR1B = (1 << CS11) | (1 << CS10);

  lgt::Status st = lgt::SPIDual::begin();
  if (st != lgt::Ok) {
    Serial.println(F("  SPI dual unavailable on this silicon."));
    return;
  }

  uint8_t buf[32];
  uint16_t t0, t1;

  cli(); t0 = TCNT1; sei();
  for (uint8_t i = 0; i < 32; ++i) { SPI.transfer(0); }
  cli(); t1 = TCNT1; sei();
  Serial.print(F("  single, 32 bytes = ")); Serial.println((uint16_t)(t1 - t0));

  cli(); t0 = TCNT1; sei();
  for (uint8_t i = 0; i < 16; ++i) { lgt::SPIDual::receive2(); }
  cli(); t1 = TCNT1; sei();
  Serial.print(F("  dual,   16x2      = ")); Serial.println((uint16_t)(t1 - t0));

  cli(); t0 = TCNT1; sei();
  lgt::SPIDual::receive(buf, sizeof(buf));
  cli(); t1 = TCNT1; sei();
  Serial.print(F("  dual,   32 bulk   = ")); Serial.println((uint16_t)(t1 - t0));

  lgt::SPIDual::end();
  TCCR1B = 0;
  SPI.endTransaction();
  Serial.println(F("=== done. Fewer ticks = faster. ==="));
}

void loop() { delay(1000); }
