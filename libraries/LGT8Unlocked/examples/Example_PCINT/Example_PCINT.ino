// Example_PCINT — pin-change interrupt dengan callback (lgt::Pcint)
// Semua pin bisa interrupt pin-change (bukan cuma INT0/INT1).
// Koneksi: tombol ke pin 2 (GND saat ditekan).
#include <LGT8Unlocked.h>

static volatile uint8_t falling = 0;
static volatile uint8_t rising = 0;

void onFalling() { falling++; }   // tekan
void onRising()  { rising++; }    // lepas

void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);

  // Dua callback beda mode di pin yang sama
  Pcint.attach(2, onFalling, PCINT_FALLING);
  Pcint.attach(2, onRising, PCINT_RISING);

  Serial.println("press button on pin 2");
}

void loop() {
  if (falling || rising) {
    Serial.print("down="); Serial.print((int)falling);
    Serial.print(" up="); Serial.println((int)rising);
  }
  delay(50);
}
