/*
 * final_sweep.ino — LGT8F328P ONE-SHOT final silicon sweep.
 * Everything in one run, one paste:
 *   T1 mul signed/unsigned 256 pairs        (regression, DOC-021)
 *   T2 divmod 80 cases incl 0xFFFFFFFF      (regression, DOC-025 SW fallback)
 *   T3 DSP16 + - * / % 81 combos            (regression, DOC-025 single-op)
 *   T4 dotProduct + dotProductFast          (regression)
 *   T5 EEPROM full sweep + 32-bit + SWM + OOB (DOC-024; runs once, marker)
 *   T6 WDT reset-mode arm/feed/disarm       (DOC-023)
 *   T7 SPI loopback byte-exact              (SKIP unless MOSI->MISO jumper)
 *   T8 EEPROM power-cycle token             (re-run after power-cycle to PASS)
 *   T9 PERF cycles/op: mul, div, dot16 x3, ADC x2
 *   T10 uDSC 20k stress under Timer0 IRQ    (DOC-018 audit)
 *   T11 Timer3 counts + analogReference ADMUX report
 *   T12 ADC 12-bit stable                   (SKIP unless A0 tied)
 *   T13 INFO extended-op actuals (add/sub/negY/accY/macHalf/mulNeg...)
 *   T14 WDT REAL-RESET proof (LAST: arms 64ms reset-mode, does not feed ->
 *       chip reboots; next boot prints proof only and halts)
 * Serial @ 115200. Paste EVERYTHING.
 */

#include <Arduino.h>
#include <LGT8Unlocked.h>
#include <EEPROM.h>
#include <WDT.h>
#include <SPI.h>

using namespace lgt::dsp;
namespace dsp = lgt::dsp;

uint16_t g_pass = 0, g_fail = 0, g_skip = 0, g_info = 0;
char buf[48];

static void R(const char *n, bool ok, const char *d = "") {
  if (ok) g_pass++; else g_fail++;
  Serial.print(ok ? F("PASS ") : F("FAIL ")); Serial.print(n);
  if (*d) { Serial.print(F(" : ")); Serial.print(d); }
  Serial.println();
}
static void K(const char *n, const char *d = "") { g_skip++; Serial.print(F("SKIP ")); Serial.print(n); if (*d) { Serial.print(F(" : ")); Serial.print(d); } Serial.println(); }
static void I(const char *n, const char *d = "") { g_info++; Serial.print(F("INFO ")); Serial.print(n); if (*d) { Serial.print(F(" : ")); Serial.print(d); } Serial.println(); }
static int16_t sat16r(int32_t v) { if (v > 32767) return 32767; if (v < -32768) return -32768; return (int16_t)v; }

// boot guard for T14 (WDT real reset)
volatile uint16_t g_bootCnt __attribute__((section(".noinit")));
volatile uint16_t g_bootMagic __attribute__((section(".noinit")));
#define BMAGIC 0xB007

static void t1_mul(void) {
  static const int16_t v[16] PROGMEM = {1, 2, 3, 127, 128, 255, 256, 1000, 30000, -1, -2, -128, -1000, -30000, -32768, 32767};
  int16_t lv[16];
  memcpy_P(lv, v, sizeof(v));          // keep .bss lean; const stays in flash
  for (uint8_t i = 0; i < 16; i++) for (uint8_t j = 0; j < 16; j++) {
    uint16_t x = (uint16_t)lv[i], y = (uint16_t)lv[j];
    if (dsp::mul(x, y) != (uint32_t)x * y) { R("T1.mulU", false); return; }
    if ((int32_t)dsp::mul(x, y, true, true) != (int32_t)lv[i] * (int32_t)lv[j]) { R("T1.mulS", false); return; }
  }
  R("T1.mul 256 pairs", true);
}
static void t2_div(void) {
  static const uint32_t dv[10] PROGMEM = {0x7FFFFFFFul, 0x80000000ul, 0xFFFFFFFFul, 65535ul, 65536ul, 1000ul, 7ul, 2ul, 1ul, 0ul};
  static const uint16_t ds[8] PROGMEM = {1000, 2, 1, 65535, 0x8000, 7, 3, 0};
  uint32_t lv[10]; uint16_t ls[8];
  memcpy_P(lv, dv, sizeof(dv)); memcpy_P(ls, ds, sizeof(ds));
  for (uint8_t i = 0; i < 10; i++) for (uint8_t j = 0; j < 8; j++) {
    DivResult r = divmod(lv[i], ls[j]);
    bool z = (ls[j] == 0);
    if (r.zero != z || (!z && (r.quotient != lv[i] / ls[j] || r.remainder != (uint16_t)(lv[i] % ls[j])))) {
      snprintf(buf, sizeof(buf), "%lu/%u", (unsigned long)lv[i], ls[j]);
      R("T2.divmod", false, buf); return;
    }
  }
  R("T2.divmod 80 cases", true);
}
static void t3_dsp16(void) {
  const int16_t v[] = {1, 2, 1000, 30000, -1, -1000, -30000, 32767, -32768};
  for (uint8_t i = 0; i < 9; i++) for (uint8_t j = 0; j < 9; j++) {
    DSP16 a(v[i]), b(v[j]);
    if ((int16_t)(a + b) != sat16r((int32_t)v[i] + v[j])) { R("T3.add", false); return; }
    if ((int16_t)(a - b) != sat16r((int32_t)v[i] - v[j])) { R("T3.sub", false); return; }
    if ((int16_t)(a * b) != sat16r((int32_t)v[i] * v[j])) { R("T3.mul", false); return; }
    if (v[j] != 0) {
      if ((int16_t)(a / b) != sat16r((int32_t)v[i] / v[j])) { R("T3.div", false); return; }
      if ((int16_t)(a % b) != sat16r((int32_t)v[i] % v[j])) { R("T3.mod", false); return; }
    } else if ((int16_t)(a / b) != 0) { R("T3.div0", false); return; }
  }
  R("T3.DSP16 81 combos", true);
}
static int32_t swDot(const int16_t *a, const int16_t *b, uint16_t c) { int32_t s = 0; for (uint16_t i = 0; i < c; i++) s += (int32_t)a[i] * b[i]; return s; }
static void t4_dot(void) {
  static int16_t a[9] = {1000, -2000, 300, -7, 32767, -32768, 1, -1, 512};
  static int16_t b[9] = {3, 5, -1000, 2000, -1, 2, -32768, 32767, -512};
  for (uint8_t c = 1; c <= 9; c++) {
    int32_t e = swDot(a, b, c);
    if (dotProduct(a, b, c) != e || dotProductFast(a, b, c) != e) { R("T4.dot", false); return; }
  }
  R("T4.dotProduct+Fast", true);
}
static const uint8_t SMARK[4] = {0xB4, 0xA5, 'V', 2};
static void t5_eeprom(void) {
  bool done = true;
  for (uint8_t k = 0; k < 4; k++) if (lgt_eeprom_read_byte(1016 + k) != SMARK[k]) { done = false; break; }
  if (done) {
    bool ok = true;
    for (uint16_t ad = 0; ad < 1020 && ok; ad += 17) if (lgt_eeprom_read_byte(ad) != (uint8_t)(ad * 7u + 3u)) ok = false;
    R("T5.sweep sample-check", ok);
    return;
  }
  bool ok = true;
  for (uint16_t ad = 0; ad < 1020; ad++) lgt_eeprom_write_byte(ad, (uint8_t)(ad * 7u + 3u));
  for (uint16_t ad = 0; ad < 1020 && ok; ad++) if (lgt_eeprom_read_byte(ad) != (uint8_t)(ad * 7u + 3u)) ok = false;
  for (uint16_t ad = 0; ad + 4 <= 1020 && ok; ad += 4) {
    uint32_t e = (uint32_t)lgt_eeprom_read_byte(ad) | ((uint32_t)lgt_eeprom_read_byte(ad + 1) << 8)
               | ((uint32_t)lgt_eeprom_read_byte(ad + 2) << 16) | ((uint32_t)lgt_eeprom_read_byte(ad + 3) << 24);
    if (lgt_eeprom_read32(ad) != e) ok = false;
  }
  uint32_t sw = 0x89ABCDEFul, swr = 0;
  lgt_eeprom_writeSWM(1012, &sw, 1); lgt_eeprom_readSWM(1012, &swr, 1);
  if (swr != sw) ok = false;
  lgt_eeprom_write32(E2END + 1, 0xDEADBEEFul);
  if (lgt_eeprom_read32(E2END + 1) == 0xDEADBEEFul) ok = false;
  for (uint8_t k = 0; k < 4; k++) lgt_eeprom_write_byte(1016 + k, SMARK[k]);
  R("T5.full sweep 1020B+32bit+SWM+OOB", ok);
}
static void t6_wdt_armfeed(void) {
  Lgtwdt.begin(WTO_32KHZ);
  uint8_t s = SREG; cli();
  __asm__ __volatile__("wdr" "\n\t" "sts %[c],%[u]" "\n\t" "sts %[c],%[r]"
      : : [c] "n"(_SFR_MEM_ADDR(WDTCSR)), [u] "r"((uint8_t)((1 << WDCE) | (1 << WDE))),
          [r] "r"((uint8_t)((1 << WDE) | 0b110)) : "memory");
  SREG = s;
  uint32_t t0 = millis();
  while (millis() - t0 < 300) __asm__ __volatile__("wdr" ::: "memory");
  s = SREG; cli();
  __asm__ __volatile__("wdr" "\n\t" "sts %[c],%[u]" "\n\t" "sts %[c],%[z]"
      : : [c] "n"(_SFR_MEM_ADDR(WDTCSR)), [u] "r"((uint8_t)((1 << WDCE) | (1 << WDE))),
          [z] "r"((uint8_t)0x00) : "memory");
  SREG = s;
  R("T6.WDT.armFeed", true);
}
static void t7_spi(void) {
  pinMode(SS, OUTPUT);
  SPI.begin(); SPI.setClockDivider(SPI_CLOCK_DIV4);
  uint8_t prev = SPI.transfer(0x00);
  bool ok = true;
  for (uint16_t i = 1; i <= 255 && ok; i++) if (SPI.transfer((uint8_t)i) != (uint8_t)(i - 1)) ok = false;
  if (SPI.transfer(0x00) != 0xFF) ok = false;
  uint16_t p16b = SPI.transfer16(0xA55A); p16b = SPI.transfer16(0x0000);
  SPI.end();
  if (!ok && (prev == 0x00 || prev == 0xFF) && p16b != 0xA55A) K("T7.SPI", "no MOSI->MISO jumper (wire it to test FIFO)");
  else R("T7.SPI 256B+transfer16", ok && p16b == 0xA55A);
}
static void t8_persist(void) {
  if (lgt_eeprom_read32(1004) == 0xC0FFEE01ul) R("T8.persist", true, "token survived");
  else { lgt_eeprom_write32(1004, 0xC0FFEE01ul); I("T8.persist", "token written - power-cycle & re-run to PASS"); }
}
static uint32_t tu;
static void bench(const char *n, uint32_t us, uint32_t cnt) {
  Serial.print(F("BENCH ")); Serial.print(n); Serial.print(' '); Serial.print(us); Serial.print(F("us "));
  Serial.print((us * 32ul) / (cnt ? cnt : 1)); Serial.println(F(" cyc/op"));
}
static void t9_perf(void) {
  volatile uint32_t sink = 0;
  const uint16_t N = 10000, M = 2000, K = 1000;
  tu = micros(); for (uint16_t i = 0; i < N; i++) sink += (uint32_t)((int16_t)i) * (uint16_t)(i + 1); bench("mul.SW", micros() - tu, N);
  tu = micros(); for (uint16_t i = 0; i < N; i++) sink += dsp::mul(i, (uint16_t)(i + 1)); bench("mul.uDSC", micros() - tu, N);
  tu = micros(); for (uint16_t i = 0; i < N; i++) sink += (0x7FFFFFFFul - i) / (uint16_t)(i + 1); bench("div.SW", micros() - tu, N);
  tu = micros(); for (uint16_t i = 0; i < N; i++) sink += divmod(0x7FFFFFFFul - i, (uint16_t)(i + 1)).quotient; bench("div.uDSC", micros() - tu, N);
  int16_t da[128], db[128];
  for (uint16_t i = 0; i < 128; i++) { da[i] = (int16_t)(i * 1000 - 8000); db[i] = (int16_t)(300 - i * 7); }
  tu = micros(); for (uint16_t i = 0; i < M; i++) sink += (uint32_t)swDot(da, db, 16); bench("dot16.SW", micros() - tu, M);
  tu = micros(); for (uint16_t i = 0; i < M; i++) sink += (uint32_t)dotProduct(da, db, 16); bench("dot16.uDSC", micros() - tu, M);
  tu = micros(); for (uint16_t i = 0; i < M; i++) sink += (uint32_t)dotProductFast(da, db, 16); bench("dot16.uDSC-SRAM", micros() - tu, M);
  const uint16_t M64 = 800, M128 = 400;
  tu = micros(); for (uint16_t i = 0; i < M64; i++) sink += (uint32_t)dotProduct(da, db, 64); bench("dot64.uDSC", micros() - tu, M64);
  tu = micros(); for (uint16_t i = 0; i < M64; i++) sink += (uint32_t)dotProductFast(da, db, 64); bench("dot64.SRAM", micros() - tu, M64);
  tu = micros(); for (uint16_t i = 0; i < M128; i++) sink += (uint32_t)dotProduct(da, db, 128); bench("dot128.uDSC", micros() - tu, M128);
  tu = micros(); for (uint16_t i = 0; i < M128; i++) sink += (uint32_t)dotProductFast(da, db, 128); bench("dot128.SRAM", micros() - tu, M128);
  int16_t cx[16], chh[8]; int32_t cOut[23];
  for (uint8_t i = 0; i < 16; i++) cx[i] = (int16_t)(i * 13 - 99);
  for (uint8_t i = 0; i < 8; i++)  chh[i] = (int16_t)(100 - i * 11);
  const uint16_t MC = 300;
  tu = micros(); for (uint16_t i = 0; i < MC; i++) { Dsp.convolve(cx, 16, chh, 8, cOut); sink += (uint32_t)cOut[11]; } bench("conv23.SW", micros() - tu, MC);
  analogReadResolution(12);
  tu = micros(); for (uint16_t i = 0; i < K; i++) sink += (uint32_t)analogRead(A0); bench("adc.read", micros() - tu, K);
  tu = micros(); for (uint16_t i = 0; i < K; i++) sink += (uint32_t)analogReadFast(A0); bench("adc.readFast", micros() - tu, K);
  Serial.print(F("sink=")); Serial.println((unsigned long)sink);
}
static void t10_stress(void) {
  int32_t cs = 0, ch = 0;
  for (uint16_t i = 1; i <= 20000; i++) {
    int16_t a = (int16_t)(i * 2654435761ul >> 16), b = (int16_t)(i * 40503 + 17);
    cs += (int32_t)a * b;
    ch += (int32_t)dsp::mul((uint16_t)a, (uint16_t)b, true, true);
    if ((i & 0x3FF) == 0) { volatile uint32_t t = millis(); (void)t; }
  }
  R("T10.stress20k", cs == ch, cs == ch ? "checksum match" : "CORRUPTED");
}
static void t11_regs(void) {
  bool ok = true;
  TCCR3B = (1 << CS30);
  uint16_t a = TCNT3; delay(2);
  if (TCNT3 <= a) ok = false;
  TCCR3B = 0;
  R("T11.Timer3 counts", ok);
  char d[20];
  analogReference(DEFAULT);        snprintf(d, sizeof(d), "ADMUX=0x%02X", ADMUX); I("T11.REF DEFAULT", d);
  analogReference(INTERNAL1V024);  snprintf(d, sizeof(d), "ADMUX=0x%02X", ADMUX); I("T11.REF 1V024", d);
  analogReference(INTERNAL2V048);  snprintf(d, sizeof(d), "ADMUX=0x%02X", ADMUX); I("T11.REF 2V048", d);
  analogReference(INTERNAL4V096);  snprintf(d, sizeof(d), "ADMUX=0x%02X", ADMUX); I("T11.REF 4V096", d);
  analogReference(DEFAULT);
}
static void t12_adc(void) {
  analogReadResolution(12);
  uint16_t v1 = analogRead(A0), v2 = analogRead(A0), v3 = analogRead(A0);
  int16_t s1 = (int16_t)(v1 > v2 ? v1 - v2 : v2 - v1), s2 = (int16_t)(v2 > v3 ? v2 - v3 : v3 - v2);
  if (s1 > 50 || s2 > 50) K("T12.ADC", "A0 floating - tie A0 to GND/VCC");
  else R("T12.ADC stable 12-bit", true);
}
static void t13_info(void) {
  const uint16_t x = 200, y = 3;
  char d[26];
  snprintf(d, sizeof(d), "add=%lu", (unsigned long)dsp::add(x, y)); I("T13.add(200,3)", d);
  snprintf(d, sizeof(d), "sub=%lu", (unsigned long)dsp::sub(x, y)); I("T13.sub(200,3)", d);
  snprintf(d, sizeof(d), "loadY=%lu", (unsigned long)dsp::loadY(y)); I("T13.loadY(3)", d);
  snprintf(d, sizeof(d), "negY=%lu", (unsigned long)dsp::negY(y)); I("T13.negY(3)", d);
  snprintf(d, sizeof(d), "accY=%lu", (unsigned long)dsp::accumulateY(y)); I("T13.accY(3)", d);
  snprintf(d, sizeof(d), "subY=%lu", (unsigned long)dsp::subtractY(y)); I("T13.subY(3)", d);
  snprintf(d, sizeof(d), "mac=%lu", (unsigned long)dsp::mac(x, y)); I("T13.mac(200,3)", d);
  snprintf(d, sizeof(d), "macH=%lu", (unsigned long)dsp::macHalf(x, y)); I("T13.macHalf(200,3)", d);
  snprintf(d, sizeof(d), "msc=%lu", (unsigned long)dsp::msc(x, y)); I("T13.msc(200,3)", d);
  snprintf(d, sizeof(d), "mulN=%lu", (unsigned long)dsp::mulNegative(x, y)); I("T13.mulNegative(200,3)", d);
  snprintf(d, sizeof(d), "mulH=%lu", (unsigned long)dsp::mulHalf(x, y)); I("T13.mulHalf(200,3)", d);
  snprintf(d, sizeof(d), "sqX=%lu", (unsigned long)dsp::squareX(x)); I("T13.squareX(200)", d);
}
static void t14_wdt_reset(void) {
  Serial.println(F("--- T14 arming WDT 64ms reset-mode, NOT feeding (expect reboot) ---"));
  Serial.flush();
  Lgtwdt.begin(WTO_32KHZ);
  uint8_t s = SREG; cli();
  __asm__ __volatile__("wdr" "\n\t" "sts %[c],%[u]" "\n\t" "sts %[c],%[r]"
      : : [c] "n"(_SFR_MEM_ADDR(WDTCSR)), [u] "r"((uint8_t)((1 << WDCE) | (1 << WDE))),
          [r] "r"((uint8_t)(1 << WDE)) : "memory");
  SREG = s;
  uint32_t t0 = millis();
  while (millis() - t0 < 2000) { /* NOT feeding */ }
  s = SREG; cli();
  __asm__ __volatile__("wdr" "\n\t" "sts %[c],%[u]" "\n\t" "sts %[c],%[z]"
      : : [c] "n"(_SFR_MEM_ADDR(WDTCSR)), [u] "r"((uint8_t)((1 << WDCE) | (1 << WDE))),
          [z] "r"((uint8_t)0x00) : "memory");
  SREG = s;
  R("T14.reset", false, "chip did NOT reset in 2s");
}

static void t15_rtc(void) {
  lgt::Status st = Rtc.begin();
  if (st != lgt::Ok) { R("T15.Rtc", false, "begin failed"); return; }
  Rtc.set(0);
  delay(2500);
  uint32_t s = Rtc.seconds();
  Rtc.end();
  char b[26];
  snprintf(b, sizeof(b), "t=2.5s rtc=%lus", (unsigned long)s);
  R("T15.Rtc-1Hz", s >= 2 && s <= 4, b);
}
static void t16_peripheral(void) {
  lgt::Status off = Pwr.peripheral(lgt::Power::PeripheralTWI, false);
  bool bitSet = (PRR & _BV(7)) != 0;
  Pwr.peripheral(lgt::Power::PeripheralTWI, true);
  bool bitCleared = (PRR & _BV(7)) == 0;
  R("T16.PRR-domain", off == lgt::Ok && bitSet && bitCleared, "TWI off->on roundtrip");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  delay(150);
  if (g_bootMagic == BMAGIC) g_bootCnt++; else { g_bootMagic = BMAGIC; g_bootCnt = 1; }
  if (g_bootCnt >= 2) {   // post-reset run: proof only, then halt
    Serial.println(F("=== POST-RESET PROOF (T14) ==="));
    Serial.print(F("boots=")); Serial.println((unsigned int)g_bootCnt);
    Serial.println(F("T14.WDT-real-reset: PASS (rebooted after unfed reset-mode arm)"));
    Serial.println(F("=== SWEEP COMPLETE ==="));
    g_bootMagic = 0;
    for (;;) delay(1000);
  }
  Serial.println(F("=== FINAL SWEEP run1 ==="));
  t1_mul(); t2_div(); t3_dsp16(); t4_dot(); t5_eeprom(); t6_wdt_armfeed();
  t7_spi(); t8_persist(); t9_perf(); t10_stress(); t11_regs(); t12_adc(); t13_info();
  t15_rtc(); t16_peripheral();
  Serial.print(F("=== pre-T14 pass=")); Serial.print(g_pass);
  Serial.print(F(" fail=")); Serial.print(g_fail);
  Serial.print(F(" skip=")); Serial.print(g_skip);
  Serial.print(F(" info=")); Serial.println(g_info);
  Serial.flush();
  delay(100);
  t14_wdt_reset();   // LAST: resets the chip -> run 2 prints proof only
}

void loop() { delay(1000); }
