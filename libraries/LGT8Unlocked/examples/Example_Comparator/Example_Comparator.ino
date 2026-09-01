// Example_Comparator — analog comparator (lgt::Comp)
// Bandingkan AC0P (pin D6/PB6) vs AC0N (pin D7/PB7) atau banding ACBG.
// Koneksi: sinyal ke PB6, referensi 1.1V bandgap di AC0P.
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  // Enable kedua comparator
  Comp.enable(0, true);
  Comp.enable(1, true);

  // Edge interrupt: rising
  Comp.setEdge(0, 1);          // 0=toggle, 1=rising, 2=falling
  Comp.setEdge(1, 1);

  // Filter/debounce 64us
  Comp.setFilter(0, lgt::Filter64us);

  // Interrupt enable + flag clear
  Comp.enableInterrupt(0, true);
  Comp.clearFlag(0);

  Serial.println("comparator ready");
}

void loop() {
  // Baca output langsung
  bool c0 = Comp.output(0);
  bool c1 = Comp.output(1);
  Serial.print("c0="); Serial.print(c0 ? 1 : 0);
  Serial.print(" c1="); Serial.println(c1 ? 1 : 0);
  delay(200);
}
