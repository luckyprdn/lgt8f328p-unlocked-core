/*
 * danger_probe.ino — DEBUG SKETCH for every feature flagged unsafe/unreliable
 * on LGT8F328P LQFP48. Safe: read-only + guarded writes, single run, NO reset.
 * Serial @115200. Paste output.
 *
 * D1  DA register at boot: garbage? (why accumulate ops MUST be initialized)
 * D2  DSSD (IO 0x02) dead register check
 * D3  EEPROM DOC-024 fallback correctness (write32 roundtrip)
 * D4  DOC-026 SW-composed mulNegative / mulHalf / chain semantics
 * D5  WDT: reset-mode arm/feed/disarm + WDTCSR state (interrupt mode = absent)
 * D6  DOC-018 guard: DSCR DSUEN cleared after every API op
 * D7  Lock/config register snapshot (ECCR/CLKPR/PMCR/MCUCR)
 * D8  DSP16 DOC-027 saturation edges (-32768/-1, + overflow, neg overflow)
 * D9  EEPROM OOB guard (write past E2END must be refused)
 * D10 DSU discipline demo: scoped op vs raw DSUEN-leak readback
 */

#include <Arduino.h>
#include <LGT8Unlocked.h>
#include <EEPROM.h>
#include <WDT.h>

using namespace lgt::dsp;
namespace dsp = lgt::dsp;

uint16_t g_pass = 0, g_fail = 0, g_info = 0;
char buf[40];

static void R(const char *n, bool ok, const char *d = "") {
  if (ok) g_pass++; else g_fail++;
  Serial.print(ok ? F("PASS ") : F("FAIL ")); Serial.print(n);
  if (*d) { Serial.print(F(" : ")); Serial.print(d); }
  Serial.println();
}
static void I(const char *n, const char *d = "") {
  g_info++; Serial.print(F("INFO ")); Serial.print(n);
  if (*d) { Serial.print(F(" : ")); Serial.print(d); }
  Serial.println();
}
static void dbg1(void) {
  // first touch of DA: accumulateY(0) adds zero -> returns raw DA content
  uint32_t d0 = dsp::accumulateY(0);
  uint32_t d1 = dsp::accumulateY(1);      // d0 + 1 if accumulate arithmetic is pure
  snprintf(buf, sizeof(buf), "DA@boot=0x%08lX raw", (unsigned long)d0);
  I("D1.DA-at-boot", d0 ? buf : "DA=0 (clean boot)");
  R("D1.accumulate-pure", d1 == d0 + 1, d0 ? "true but DA uninit -> init DA first!" : "true");
}
static void dbg2(void) {
  dsp::add(0, 0);
  volatile uint32_t p = dsp::mul(30000, 2, false, false);  // 60000 = 0xEA60
  uint8_t s1 = _SFR_IO8(0x02);
  dsp::add(30000, 30000);                                    // saturating add
  uint8_t s2 = _SFR_IO8(0x02);
  dsp::mul(255, 255);
  uint8_t s3 = _SFR_IO8(0x02);
  snprintf(buf, sizeof(buf), "mul=60000:0x%02X sat-add:0x%02X mul255:0x%02X (unstable)", s1, s2, s3);
  I("D2.DSSD-unstable", buf);
}
static void dbg3(void) {
  const uint16_t ad = 500;
  uint32_t old = lgt_eeprom_read32(ad);
  uint32_t w = 0x89ABCDEFul;
  lgt_eeprom_write32(ad, w);
  uint32_t r = lgt_eeprom_read32(ad);
  lgt_eeprom_write32(ad, old);             // restore scratch
  snprintf(buf, sizeof(buf), "w=89ABCDEF r=0x%08lX", (unsigned long)r);
  R("D3.eeprom-write32-fallback", r == w, buf);
}
static void dbg4(void) {
  uint32_t mn = dsp::mulNegative(200, 3);          // expect -(600) = 0xFFFFFDA8
  uint32_t mh = dsp::mulHalf(200, 3);              // expect 300
  snprintf(buf, sizeof(buf), "mulNeg=0x%08lX mulHalf=%lu", (unsigned long)mn, (unsigned long)mh);
  I("D4.SW-composed-variants", buf);
  R("D4.mulNegative=-600", mn == (uint32_t)(-(int32_t)600), "");
  R("D4.mulHalf=300", mh == 300, "");
  dsp::add(0, 0);                                  // DA := 0
  uint32_t m1 = dsp::mac(200, 3);                  // 0 + 600
  uint32_t m2 = dsp::mac(200, 3);                  // 600 + 600  (chain OK when DA tracked)
  snprintf(buf, sizeof(buf), "mac=%lu mac2=%lu", (unsigned long)m1, (unsigned long)m2);
  I("D4.mac-chain-DA-tracked", buf);
  R("D4.mac-chain-correct", m1 == 600 && m2 == 1200, "");
}
static void dbg5(void) {
  Lgtwdt.begin(WTO_32KHZ);
  uint8_t s = SREG; cli();
  __asm__ __volatile__("wdr" "\n\t" "sts %[c],%[u]" "\n\t" "sts %[c],%[r]"
      : : [c] "n"(_SFR_MEM_ADDR(WDTCSR)), [u] "r"((uint8_t)((1 << WDCE) | (1 << WDE))),
          [r] "r"((uint8_t)((1 << WDE) | 0b110)) : "memory");
  SREG = s;
  uint8_t armed = WDTCSR;
  uint32_t t0 = millis();
  while (millis() - t0 < 300) __asm__ __volatile__("wdr" ::: "memory");
  s = SREG; cli();
  __asm__ __volatile__("wdr" "\n\t" "sts %[c],%[u]" "\n\t" "sts %[c],%[z]"
      : : [c] "n"(_SFR_MEM_ADDR(WDTCSR)), [u] "r"((uint8_t)((1 << WDCE) | (1 << WDE))),
          [z] "r"((uint8_t)0x00) : "memory");
  SREG = s;
  snprintf(buf, sizeof(buf), "WDTCSR armed=0x%02X disarmed=0x%02X", armed, (int)WDTCSR);
  I("D5.WDT-reset-mode", buf);
  R("D5.fed-300ms-no-reset", true, "(DOC-023: interrupt mode absent - reset-mode ONLY)");
}
static void dbg6(void) {
  dsp::mul(30000, 2); dsp::divmod(0x7FFFFFFFul, 1000); dsp::dotProduct;
  uint8_t dscr = DSCR;
  snprintf(buf, sizeof(buf), "DSCR=0x%02X after ops", dscr);
  R("D6.DSUEN-cleared", (dscr & _BV(DSUEN)) == 0, buf);
}
static void dbg7(void) {
  snprintf(buf, sizeof(buf), "ECCR=0x%02X CLKPR=0x%02X PMCR=0x%02X MCUCR=0x%02X",
           ECCR, CLKPR, PMCR, MCUCR);
  I("D7.config", buf);
  I("D7.fuse-note", "fuses read-only via ISP (avrdude -U ...:r:-:h). Shipment default unlocked 0xFF - set lock bits for IP protection");
}
static void dbg8(void) {
  DSP16 a(-32768), b(-1), c(30000), d(2), e(-30000);
  int16_t r1 = (int16_t)(a / b);   // -32768/-1 -> sat 32767 (DOC-027)
  int16_t r2 = (int16_t)(c + c);   // 60000 -> sat 32767
  int16_t r3 = (int16_t)(e + e);   // -60000 -> sat -32768
  snprintf(buf, sizeof(buf), "-32768/-1=%d 30000+30000=%d -30000-30000=%d", r1, r2, r3);
  R("D8.DSP16-saturation", r1 == 32767 && r2 == 32767 && r3 == -32768, buf);
}
static void dbg9(void) {
  uint16_t oob = E2END + 1;
  uint8_t old = lgt_eeprom_read_byte(oob);
  lgt_eeprom_write_byte(oob, 0x5A);
  uint8_t now = lgt_eeprom_read_byte(oob);
  lgt_eeprom_write_byte(oob, old);        // restore (was refused anyway)
  snprintf(buf, sizeof(buf), "write@%u -> read=0x%02X", (int)oob, now);
  R("D9.OOB-refused", now != 0x5A, buf);
}
static void dbg10(void) {
  dsp::add(0, 0);
  uint8_t before = DSCR;
  volatile uint32_t p = dsp::mul(1000, 3);
  uint8_t after = DSCR;
  dsp::add(0, 0);
  uint8_t again = DSCR;
  snprintf(buf, sizeof(buf), "p=%lu DSCR=%02X->%02X->%02X (DSUEN bit7)",
           (unsigned long)p, before, after, again);
  R("D10.scoped-DSU", (after & 0x80) == 0 && (again & 0x80) == 0, buf);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(200);
  Serial.println(F("=== UNRELIABLE-FEATURE PROBE ==="));
  dbg1(); dbg2(); dbg3(); dbg4(); dbg5(); dbg6(); dbg7(); dbg8(); dbg9(); dbg10();
  Serial.print(F("=== pass=")); Serial.print(g_pass);
  Serial.print(F(" fail=")); Serial.print(g_fail);
  Serial.print(F(" info=")); Serial.println(g_info);
  Serial.println(F("DEBUG_DONE"));
}
void loop() { delay(1000); }
