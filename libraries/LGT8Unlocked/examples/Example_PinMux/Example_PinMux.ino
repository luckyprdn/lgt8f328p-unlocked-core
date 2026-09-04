/*
 * Example_PinMux — reassign peripheral pins
 * -------------------------------------------
 *   Connect : nothing needed (round-trip demo).
 *   Watch   : Serial Monitor @115200.
 *   Silicon : 328P muxes via PMX0/1/2; 328D via PMXCR+IOCR.
 *
 * Console safety: TX/RX moves are done as enable -> disable so the USB
 * serial you are reading stays alive. For a REAL remap, call with `true`
 * and do not call the matching `false`.
 */
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== PinMux demo (round-trip, console-safe) ==="));

  Serial.println(F("  SS  PB2 -> PB1 ..."));
  PinMux.ssToPB1(true);
  Serial.println(F("    moved. (real use: frees PB2)"));

  Serial.println(F("  TX  PD1 -> PD6, RX PD0 -> PD5 ..."));
  PinMux.txdToPD6(true);
  PinMux.rxdFromPD5(true);
  Serial.println(F("    moved. restoring so this console still works..."));
  PinMux.txdToPD6(false);
  PinMux.rxdFromPD5(false);
  PinMux.ssToPB1(false);
  Serial.println(F("    restored to defaults."));

  Serial.println(F("  PC6/PE6 as extra GPIO (QFP48) ..."));
  PinMux.pc6AsGPIO(true);
  PinMux.pe6AsGPIO(true);
  PinMux.pc6AsGPIO(false);
  PinMux.pe6AsGPIO(false);
  Serial.println(F("    toggled back."));

  Serial.println(F("=== done. ==="));
}

void loop() {}
