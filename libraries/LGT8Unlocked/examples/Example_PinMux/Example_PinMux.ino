// Example_PinMux — pin reassignment (lgt::PinMux)
// 328P pakai PMX0/PMX1/PMX2; 328D pakai PMXCR + IOCR (method disesuaikan).
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  // USART TX pindah dari PD1 → PD6
  PinMux.txdToPD6(true);

  // USART RX pindah dari PD0 → PD5
  PinMux.rxdFromPD5(true);

  // SPI SS pindah dari PB2 → PB1 (biar PB2 bebas)
  PinMux.ssToPB1(true);

  // PC6/PE6 jadi GPIO biasa (QFP48) — buat pin ekstra
  PinMux.pc6AsGPIO(true);
  PinMux.pe6AsGPIO(true);

  Serial.println("pinmux applied");

  // CATATAN: setelah ini Serial tetap di pin baru (TX=PD6, RX=PD5).
  // Kalau mau balik ke default: panggil method lagi dengan false.
}

void loop() {}
