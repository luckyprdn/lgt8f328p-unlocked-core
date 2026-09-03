/*
 * LGT8F328P Integrated Silicon Verification Sketch
 * Covers the three P0 fixes: uDSC (DOC-018/019), WDT ISR (DOC-020), ADC 12-bit.
 * Upload when the board is available. Serial @ 115200.
 * Each section prints PASS/FAIL with the measured value; a final summary line
 * lists how many passed. Designed to run unattended and report over UART.
 */
#include <Arduino.h>
#include <LGT8Unlocked.h>
#include <WDT.h>

using namespace lgt::dsp;

#define SERIAL_BAUD 115200
uint8_t g_pass = 0, g_fail = 0;

void report(const char* name, bool ok, const char* detail = "") {
  if (ok) g_pass++; else g_fail++;
  Serial.print(ok ? "PASS " : "FAIL ");
  Serial.print(name);
  if (*detail) { Serial.print(" : "); Serial.print(detail); }
  Serial.println();
}

// ---- uDSC: 16-bit operand path (DOC-019) ----
void test_udsc_16bit() {
  // Dot product of two int16 vectors whose values exceed 255 must use the
  // high byte of X/Y.  a = {1000, -2000}, b = {3, 5}
  // golden = 1000*3 + (-2000)*5 = 3000 - 10000 = -7000
  static const int16_t a[2] = {1000, -2000};
  static const int16_t b[2] = {3, 5};
  int32_t r = dotProductFast(a, b, 2);
  char buf[24]; snprintf(buf, sizeof(buf), "dp=%ld", (long)r);
  report("uDSC.dotProduct16", r == -7000, buf);

  // DIVMOD with a divisor > 255 and a large dividend (high-byte path)
  // 0x7FFFFFFF / 1000 = 2147483 rem 647
  DivResult d = divmod(0x7FFFFFFFul, 1000u);
  snprintf(buf, sizeof(buf), "q=%lu rem=%u z=%d", (unsigned long)d.quotient, d.remainder, d.zero);
  report("uDSC.divmod", d.quotient == 2147483ul && d.remainder == 647u && !d.zero, buf);

  // divide-by-zero must flag zero and not hang
  DivResult z = divmod(12345u, 0u);
  report("uDSC.divByZero", z.zero == true, z.zero ? "flagged" : "NOT flagged");

  // MAC signed saturation sanity: 30000 * 2 should saturate near int16 max
  DSP16 x(30000), y(2);
  DSP16 s = x * y; // 60000 saturates to 32767
  snprintf(buf, sizeof(buf), "sat=%d", (int)s);
  report("uDSC.saturate", (int)s == 32767, buf);
}

// ---- WDT: interrupt-only must fire repeatedly, NOT reset (DOC-020) ----
volatile uint8_t g_wdtTicks = 0;
ISR(WDT_vect) { g_wdtTicks++; }

void test_wdt_interrupt() {
  g_wdtTicks = 0;
  // Enable 32kHz RC and arm interrupt-only mode at the shortest timeout
  // (WTO_64MS = 0 -> ~64ms period at 32kHz).  wdt_ienable sets WDIE + timeout,
  // NOT WDE, so the trampoline should fire the ISR repeatedly without reset.
  Lgtwdt.begin(WTO_32KHZ);
  wdt_ienable(WTO_64MS);
  // If the trampoline is broken, the MCU resets on the 2nd timeout and this
  // counter never reaches >=2.  Wait ~400ms (no reset expected).
  uint32_t t0 = millis();
  while (millis() - t0 < 400) { /* spin */ }
  wdt_disable(); // avr-libc: stop the watchdog
  char buf[16]; snprintf(buf, sizeof(buf), "ticks=%u", g_wdtTicks);
  // Expect at least 2 interrupts in 400ms at 64ms period (no reset).
  report("WDT.isrRepeats", g_wdtTicks >= 2, buf);
}

// ---- ADC: 12-bit read on a known divider (e.g. 1/2 VCC via AREF) ----
void test_adc_12bit() {
  analogReadResolution(12);
  // Read A0 a few times; just confirm range 0..4095 and stability.
  uint16_t v = analogRead(A0);
  uint16_t v2 = analogRead(A0);
  char buf[24]; snprintf(buf, sizeof(buf), "v=%u v2=%u", v, v2);
  bool ok = (v <= 4095) && (v2 <= 4095) && (v == v2);
  report("ADC.12bitRange", ok, buf);
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial) { /* wait for USB */ }
  Serial.println("=== LGT8F328P Silicon Verification ===");
  test_udsc_16bit();
  test_wdt_interrupt();
  test_adc_12bit();
  Serial.print("=== SUMMARY pass=");
  Serial.print(g_pass);
  Serial.print(" fail=");
  Serial.print(g_fail);
  Serial.println(" ===");
}

void loop() {
  // Blink to show it survived (WDT test did not reset the chip).
  digitalWrite(LED_BUILTIN, HIGH); delay(200);
  digitalWrite(LED_BUILTIN, LOW);  delay(200);
}
