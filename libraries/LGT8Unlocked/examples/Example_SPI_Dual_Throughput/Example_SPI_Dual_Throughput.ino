// Example_SPI_Dual_Throughput — ukur SPI DUAL receive throughput.
//
// LGT8F328P only (SPFR FIFO + DUAL bit).  Wire MISO (PB4) and MOSI (PB3)
// together to see the loopback: DUAL mode reads both simultaneously.
// D/E silicon: prints Unsupported.
#include <SPI.h>
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  delay(200);

  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  lgt::Status st = lgt::SPIDual::begin();
  if (st != lgt::Ok) {
    Serial.println(F("SPI dual unavailable on this silicon"));
    return;
  }

  uint8_t buf[32];
  uint32_t t0, t1;

  // single-channel receive baseline (SPDR per byte)
  cli(); t0 = TCNT1; sei();
  for (uint8_t i = 0; i < 32; ++i) { SPI.transfer(0); }
  cli(); t1 = TCNT1; sei();
  Serial.print(F("single: ")); Serial.println((uint32_t)(t1 - t0));

  // dual receive: 2 bytes per SPDR write
  cli(); t0 = TCNT1; sei();
  for (uint8_t i = 0; i < 16; ++i) { lgt::SPIDual::receive2(); }
  cli(); t1 = TCNT1; sei();
  Serial.print(F("dual2:  ")); Serial.println((uint32_t)(t1 - t0));

  // dual bulk receive
  cli(); t0 = TCNT1; sei();
  lgt::SPIDual::receive(buf, sizeof(buf));
  cli(); t1 = TCNT1; sei();
  Serial.print(F("dual32: ")); Serial.println((uint32_t)(t1 - t0));

  lgt::SPIDual::end();
  SPI.endTransaction();
  Serial.println(F("done"));
}

void loop() { delay(1000); }
