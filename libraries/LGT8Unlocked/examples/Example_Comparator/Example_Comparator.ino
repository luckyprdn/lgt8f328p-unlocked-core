/*
 * Example_Comparator — analog comparator, the Arduino way
 * --------------------------------------------------------
 *   Connect : (optional) a signal on the comparator input pins,
 *             e.g. AC0+/AC0- per your board pinout, or leave floating.
 *   Watch   : Serial Monitor @115200 - it prints c0/c1 HIGH-LOW.
 *   Silicon : 328P has two comparators with edge, hysteresis & filter;
 *             328D/E maps the same calls (see docs).
 *
 * Note: interrupt mode is NOT enabled here - polling shows the output
 * without needing an ISR. See the library docs to attach an ISR.
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== Comparator demo ==="));

  Comp.enable(0, true);
  Comp.enable(1, true);
  Comp.setEdge(0, 1);                 // 0=toggle 1=rising 2=falling
  Comp.setEdge(1, 1);
  Comp.setFilter(0, lgt::Filter64us); // debounce 64 us
  Comp.clearFlag(0);

  Serial.println(F("  comparators enabled (edge=rising, 64us filter)."));
  Serial.println(F("  Polling outputs below - feed a signal to see it switch."));
}

void loop() {
  bool c0 = Comp.output(0);
  bool c1 = Comp.output(1);
  Serial.print(F("  c0=")); Serial.print(c0 ? F("HIGH") : F("LOW"));
  Serial.print(F("  c1=")); Serial.println(c1 ? F("HIGH") : F("LOW"));
  delay(200);
}
