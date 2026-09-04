/*
 * Example_USART — 9-bit frames + USART-as-SPI
 * ---------------------------------------------
 *   Connect : nothing required (loopback optional).
 *   Watch   : Serial Monitor @115200.
 *   Note    : 9-bit mode changes the framing of THE SAME UART that
 *             Serial uses, so it is enabled only for the write and
 *             disabled again before any println() - otherwise this
 *             console garbles.
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== USART demo ==="));

  Serial.println(F("[1] 9-bit frame write ..."));
  Usart.enable9Bit(true);
  Usart.write9(0x155);      // 9-bit value 0b101010101
  Usart.enable9Bit(false);  // back to 8-bit BEFORE printing
  Serial.println(F("    sent 0x155 as a 9-bit frame (loopback to see it)."));

  Serial.println(F("[2] USART pins as SPI master, 4 MHz ..."));
  lgt::Status s = Usart.spiBegin(4000000);
  if (s == lgt::Ok) {
    uint8_t r = Usart.spiTransfer(0xAA);
    Serial.print(F("    SPI byte in = 0x")); Serial.println(r, HEX);
    Usart.spiEnd();
    Serial.println(F("    USPI session closed."));
  } else {
    Serial.println(F("    USART-SPI unavailable here."));
  }

  Serial.println(F("=== done. ==="));
}

void loop() {}
