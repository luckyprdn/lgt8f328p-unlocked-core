/*
 * Example_System — chip ID, reset cause, safety info
 * ----------------------------------------------------
 *   Connect : nothing.
 *   Watch   : Serial Monitor @115200.
 *   Silicon : prints the per-chip GUID, why the chip last reset, the
 *             recovery-safe program limit and whether SWD is locked.
 */
#include <LGT8Unlocked.h>

static const char *resetName(uint8_t bits) {
  // bit0=power,1=external,2=WDT,3=soft/jtag,4=BrownOut(LVD) per LGT core
  switch (bits) {
    case 0x01: return "power-on";
    case 0x02: return "external pin";
    case 0x04: return "watchdog";
    case 0x08: return "software / debugger";
    case 0x10: return "brown-out (LVD)";
    default:   return "multiple or unknown";
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== System info ==="));

  Serial.print(F("  chip id     : 0x")); Serial.println(Sys.chipId(), HEX);
  Serial.print(F("  reset cause : 0x")); Serial.print(Sys.lastResetCause(), HEX);
  Serial.print(F("  (")); Serial.print(resetName(Sys.lastResetCause()));
  Serial.println(F(")"));
  Serial.print(F("  program lim.: ")); Serial.print(Sys.programLimit());
  Serial.println(F(" bytes (sketches above this are refused)"));
  Serial.print(F("  SWD debugger: "));
  Serial.println(Sys.swdDisabled() ? F("disabled (locked)") : F("enabled"));

  Sys.clearResetCause();
  Serial.println(F("  reset cause cleared - next boot reports the new cause."));
  Serial.println(F("=== done. ==="));
}

void loop() {}
