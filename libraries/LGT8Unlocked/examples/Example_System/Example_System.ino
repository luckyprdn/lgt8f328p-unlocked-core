// Example_System — chip info & reset cause (lgt::Sys)
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  // 32-bit chip ID / GUID
  uint32_t id = Sys.chipId();
  Serial.print("chipid=0x"); Serial.println(id, HEX);

  // Reset cause (bit flags: POR/WDR/BOR/EXRFD/SWR)
  uint8_t rc = Sys.lastResetCause();
  Serial.print("reset_cause=0x"); Serial.println(rc, HEX);

  // Batas program (alamat maksimum sketch)
  uint16_t limit = Sys.programLimit();
  Serial.print("program_limit=0x"); Serial.println(limit, HEX);

  // SWD disable state
  Serial.print("swd_disabled="); Serial.println(Sys.swdDisabled() ? 1 : 0);

  Sys.clearResetCause();
}

void loop() {}
