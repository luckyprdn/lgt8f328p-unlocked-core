#ifndef LGT_HOST_FAKE_SPI_H
#define LGT_HOST_FAKE_SPI_H
struct SPIClass { void begin(){} };
static SPIClass SPI __attribute__((unused));
#endif
