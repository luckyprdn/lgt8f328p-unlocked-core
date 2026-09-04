#ifndef LGT_HOST_FAKE_SPI_H
#define LGT_HOST_FAKE_SPI_H

#include <stdint.h>
#include <stddef.h>

#define MSBFIRST 0
#define LSBFIRST 1
#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3
#define SPI_CLOCK_DIV4 4
#define SPI_CLOCK_DIV16 16
#define SPI_CLOCK_DIV64 64
#define SPI_CLOCK_DIV128 128

struct SPISettings {
  SPISettings(uint32_t = 0, uint8_t = 0, uint8_t = 0) {}
};

struct SPIClass {
  void begin() {}
  void end() {}
  void beginTransaction(SPISettings) {}
  void endTransaction() {}
  void setClockDivider(uint8_t) {}
  void setBitOrder(uint8_t) {}
  void setDataMode(uint8_t) {}
  uint8_t transfer(uint8_t v) { return v; }
  uint16_t transfer16(uint16_t v) { return v; }
  void transfer(void *, size_t) {}
};
static SPIClass SPI __attribute__((unused));

#endif
