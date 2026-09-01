// Example_USART — USART 9-bit + USART-SPI (lgt::Usart)
// Serial biasa tetap jalan; ini tambahan mode 9-bit & USPI.
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  // Mode 9-bit (sering dipakai buat parity/address bit)
  Usart.enable9Bit(true);
  Usart.write9(0x1FF);   // 9-bit value
  Serial.println("9-bit sent");

  // Kembalikan ke 8-bit kalau mau lanjut normal
  Usart.enable9Bit(false);

  // USART-SPI: pakai pin TX/RX sebagai SPI master (misal mode 0)
  lgt::Status s = Usart.spiBegin(4000000);   // 4 MHz
  Serial.print("uspi_status="); Serial.println((int)s);
  if (s == lgt::Ok) {
    uint8_t r = Usart.spiTransfer(0xAA);
    Serial.print("uspi_rx="); Serial.println(r, HEX);
    Usart.spiEnd();
  }
}

void loop() {}
