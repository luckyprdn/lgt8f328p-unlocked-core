/*
 * FeatureTour — LGT8F328P (and 328D/E) guided feature tour.
 * -------------------------------------------------------------
 * User-friendly walkthrough of every module, written like an Adafruit
 * example: readable narration on Serial, safe defaults, nothing to wire up
 * for the core tour. Items that need extra hardware print a hint and skip.
 *
 * Silicon notes baked in (see docs/datasheet-errata.md DOC-001..030):
 *   - DSP16 is SATURATING: 30000*2 -> 32767, -32768/-1 -> 32767 (DOC-025/027)
 *   - WDT on 328P works in RESET mode only (DOC-023) - demo arms+feeds+disarms
 *   - EEPROM multi-byte writes use the proven byte engine (DOC-024); writing
 *     the same value twice is nearly instant (skip-write, DOC-028)
 *   - ADC on a floating pin reads wildly - the tour detects and tells you
 *
 * Open Serial Monitor @115200. No jumper required. Upload once.
 */

#include <LGT8Unlocked.h>
#include <EEPROM.h>
#include <WDT.h>

static volatile uint8_t s_clicks = 0;
static void onButton() { s_clicks++; }

static void section(const __FlashStringHelper *t) {
  Serial.println();
  Serial.print(F("== ")); Serial.println(t);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(300);
  Serial.println(F("=================================================="));
  Serial.println(F("  LGT8F328P Feature Tour"));
  Serial.println(F("=================================================="));

  // ---- 1. System: who am I? -------------------------------------
  section(F("1. SYSTEM - chip identity"));
  Serial.print(F("  chip id      : 0x")); Serial.println(Sys.chipId(), HEX);
  Serial.print(F("  last reset   : 0x")); Serial.print(Sys.lastResetCause(), HEX);
  Serial.println(F("  (1=power 2=ext 4=WDT 8=SW 16=jtag)"));
  Serial.print(F("  program limit: ")); Serial.print(Sys.programLimit());
  Serial.println(F(" bytes (recovery-safe IAP gate)"));
  Serial.print(F("  SWD debugger : ")); Serial.println(Sys.swdDisabled() ? F("disabled (locked)") : F("enabled"));
  Sys.clearResetCause();

  // ---- 2. DSP: uDSC multiply/divide - saturating 16-bit ----------
  section(F("2. DSP - saturating 16-bit math (uDSC on 328P)"));
  Serial.print(F("  30000 * 2    = ")); Serial.println(Dsp.multiply(30000, 2));
  Serial.println(F("  (clamped to 32767 - 16-bit saturating, NOT a bug)"));
  Serial.print(F("  100 / 7      = ")); Serial.println(Dsp.divide(100, 7));
  Serial.print(F("  -32768 / -1  = ")); Serial.println(Dsp.divide(-32768, -1));
  Serial.println(F("  (also 32767: INT16_MIN edge saturates, DOC-027)"));
  Serial.print(F("  200 % 7      = ")); Serial.println(Dsp.modulo(200, 7));
  Serial.print(F("  map(512,0..1023,-100..100) = "));
  Serial.println(Dsp.map(512, 0, 1023, -100, 100));
  const int16_t a[4] = {100, -200, 300, -400};
  const int16_t b[4] = {2, 3, -4, 5};
  Serial.print(F("  dot([100,-200,300,-400],[2,3,-4,5]) = "));
  Serial.println(Dsp.dot(a, b, 4));              // 200 -600 -1200 -2000
  Serial.println(F("  (expected -3600: 200-600-1200-2000)"));

  // ---- 3. ADC: 12-bit + averaging + floating detection ------------
  section(F("3. ADC - 12-bit, average, floating detection"));
  analogReadResolution(12);
  uint16_t v1 = analogRead(A0), v2 = analogRead(A0), v3 = analogRead(A0);
  int16_t spread = (v1 > v2 ? v1 - v2 : v2 - v1);
  if (spread < 0) spread = -spread;
  int16_t s2 = (v1 > v3 ? v1 - v3 : v3 - v1); if (s2 < 0) s2 = -s2;
  if (spread > 60 || s2 > 60) {
    Serial.print(F("  A0 reads ")); Serial.print(v1); Serial.print(F("/"));
    Serial.print(v2); Serial.print(F("/")); Serial.print(v3);
    Serial.println(F(" - A0 is FLOATING (no wire). Tie A0 to GND or VCC"));
    Serial.println(F("  for a real reading, then re-run."));
  } else {
    Serial.print(F("  A0 12-bit avg: ")); Serial.println(AdcExt.readAverage(A0, 16));
  }
  Serial.print(F("  PGA gain x1   : "));
  lgt::Status st = AdcExt.setGain(lgt::Gain1);
  Serial.println(st == lgt::Ok ? F("ok") : st == lgt::Unsupported ? F("n/a on 328D") : F("locked"));

  // ---- 4. DAC ------------------------------------------------------
  section(F("4. DAC - millivolt output"));
  Dac.writeMillivolt(0, 1650);
  Serial.println(F("  DAC0 set to 1650 mV (1.65 V) - measure at the DAC0 pin"));
  Serial.println(F("  (needs a voltmeter; nothing asserts here)"));

  // ---- 5. PWM: Timer1 on pin 9 -------------------------------------
  section(F("5. PWM - Timer1 1 kHz on pin 9"));
  Pwm.timer1Frequency(1000);
  analogWrite(9, 128);   // 50% duty
  Serial.println(F("  pin 9 output: 1 kHz / 50%. See it with an LED or scope."));
#if defined(LGT8_UNLOCKED_HAS_TIMER3)
  lgt::Status t3 = Pwm.timer3Frequency(1000);
  Serial.print(F("  Timer3 @1kHz : ")); Serial.println(t3 == lgt::Ok ? F("configured (QFP48: PWM on PF1/PF2)") : F("failed"));
#else
  Serial.println(F("  Timer3        : n/a on this package (328D/QFP32)"));
#endif

  // ---- 6. HDR: high-drive GPIO --------------------------------------
  section(F("6. HDR - high-drive pins (328P)"));
  pinMode(5, OUTPUT);
  lgt::Status hd = HdDrive.enable(lgt::HighDrive_PD5);
  if (hd == lgt::Ok) {
    Serial.println(F("  PD5 (pin 5) now drives up to ~20 mA (vs 4 mA normal)"));
    HdDrive.disable(lgt::HighDrive_PD5);
    Serial.println(F("  ...and restored to normal drive."));
  } else Serial.println(F("  n/a on 328D"));

  // ---- 7. Comparator -------------------------------------------------
  section(F("7. COMPARATOR - VIN+ vs VIN-"));
  Comp.enable(0, true);
  Serial.print(F("  comparator 0 output: "));
  Serial.println(Comp.output(0) ? F("HIGH") : F("LOW"));
  Serial.println(F("  (floating inputs -> output is whatever the pins see)"));
  Comp.enable(0, false);

  // ---- 8. USART 9-bit (explain only - changes framing!) -------------
  section(F("8. USART 9-bit (multi-drop)"));
  Serial.println(F("  Usart.enable9Bit(true) + Usart.write9() exist for 9-bit"));
  Serial.println(F("  multi-drop buses. NOT enabled here: it changes the Serial"));
  Serial.println(F("  framing and would garble this monitor session."));

  // ---- 9. SPI Dual / PinMux / OPA ------------------------------------
  section(F("9. SPI-DUAL, PIN-MUX, OPA"));
  Serial.println(F("  SpiDual  : 2-wire SPI receive (328P) - needs a sender + 2 wires"));
  Serial.println(F("  PinMux   : remaps SS/TXD/RXD/osc pins - reversible, see docs"));
  lgt::Status opa = Opa.enable(0);
  Serial.println(opa == lgt::Unsupported ? F("  OPA      : n/a on 328P (op-amps live on 328D)") : F("  OPA      : enabled (328D)"));

  // ---- 10. Power + LVD -----------------------------------------------
  section(F("10. POWER + LVD"));
  Lvd.setThreshold(lgt::LVD_2V9);          // 328P: returns Locked unless unlocked
  Serial.println(F("  LVD 2.9 V threshold requested (locked by recovery-safe gate = good)"));
  Pwr.idle();
  Serial.println(F("  idle() for a moment: CPU stopped, resumed. (save/powerDown go deeper)"));

  // ---- 11. EEPROM: byte engine + skip-write ---------------------------
  section(F("11. EEPROM - emulated, byte-engine (DOC-024/028)"));
  Serial.print(F("  size        : ")); Serial.print(EEPROM.length());
  Serial.println(F(" bytes (1 KB logical)"));
  EEPROM.write(0, 0x5A);
  Serial.print(F("  wrote 0x5A @0, read back: 0x"));
  Serial.println(EEPROM.read(0), HEX);
  uint32_t t0 = micros();
  EEPROM.write(0, 0x5A);                   // same value again
  uint32_t dt = micros() - t0;
  Serial.print(F("  rewriting same value took ")); Serial.print(dt);
  Serial.println(F(" us (skip-write, DOC-028 - was ~ms before)"));
  EEPROM.write(0, 0xFF);                   // tidy up

  // ---- 12. PCINT: pin-change interrupt on D2 --------------------------
  section(F("12. PCINT - button on D2 (INPUT_PULLUP)"));
  pinMode(2, INPUT_PULLUP);
  Pcint.attach(2, onButton, CHANGE);
  Serial.println(F("  Attached. Press any button wired D2<->GND and watch below."));
  Serial.println(F("=================================================="));
  Serial.println(F("  Tour done - loop prints PCINT clicks."));
}

void loop() {
  if (s_clicks) {
    Serial.print(F("click! total=")); Serial.println(s_clicks);
    s_clicks = 0;
  }
  delay(200);
}
