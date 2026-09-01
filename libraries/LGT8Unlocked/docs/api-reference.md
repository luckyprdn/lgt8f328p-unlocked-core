# LGT8Unlocked Library — API Reference

Library header: `#include <LGT8Unlocked.h>` — compile untuk **LGT8F328P** (`__LGT8FX8P__`) dan **LGT8F328D/E** (`__LGT8FX8E__`).
Fitur yang tidak ada di silikon tertentu → method `return Unsupported` / stub (tidak crash, API tetap kompatibel).

**Status enum** (dipakai banyak method):
`lgt::Ok=0, InvalidArgument, OutOfRange, NotReady, Unsupported, Locked`

---

## 1. lgt::System — sistem & reset

```cpp
uint8_t  resetCause();                    // MCUSR & 0x3F
void     clearResetCause();               // preserve SWDD bit
void     softwareReset();                 // via VDTCR SWR
DeviceId guid();                          // 4-byte chip ID (GUID0-3)
bool     configurationLoadFailed();       // IFAIL (328P only; D→false)
Status   reloadConfiguration();           // IRLD (recovery-safe→Locked)
bool     swdDisabled();                   // MCUSR.SWDD
Status   disableSWD();                    // recovery-safe→Locked
uint16_t programLimit();                  // 0x7800/0x7000/0x6000/0x4000
Status   relocateVectors(uint16_t addr);  // 512-byte align (P only; D→Unsupported)
void     restoreVectors();                // P only; D→no-op
```

## 2. lgt::Clock — clock source & calibration

```cpp
Status   enable(ClockSource src, bool yes=true);   // Internal32M/ExternalHigh/Internal32K/ExternalLow
Status   select(ClockSource src);                  // recovery-safe→Locked
Status   switchTo(ClockSource src, uint32_t startupDelayUs=0); // recovery-safe→Locked
Status   prescaler(uint8_t code);                  // CLKPR code 0-8
uint8_t  hfCalibration();                          // RCMCAL (P only; D→0)
Status   hfCalibration(uint8_t v);                 // recovery-safe→Locked (D→Unsupported)
uint8_t  lfCalibration();                          // RCKCAL
Status   lfCalibration(uint8_t v);                 // recovery-safe→Locked
void     outputPB0(bool yes=true);                 // CLKOE0
void     outputPE5(bool yes=true);                 // CLKOE1
```

## 3. lgt::Power — power management & sleep

```cpp
// PRR peripheral gating
Status disable(Peripheral p);  Status enable(Peripheral p);
// Peripheral: ADC=0, USART=1, SPI=2, Timer1=3, Timer0=5, Timer2=6, TWI=7
void timer3(bool on);          // P only; D→no-op
void pinChange(bool on);       // PRR1.PRPCI
void flashController(bool on); // PRR1.PREFL
void watchdogClock(bool on);   // PRR1.PRWDT

// Sleep modes (SMCR)
Status sleepMode(uint8_t mode); // 0=idle 1=adc-nr 2=save 3=dps1 6=dps0 7=dps2(Locked/Unsupported)
void idle();  void adcNoiseReduction();  void save();  void dps1();  void dps0();

// DPS2 (328P only; D→Unsupported)
Status dps2();  Status dps2Enable(bool yes=true);
Status dps2Timer(uint8_t periodCode, bool yes=true);
Status dps2WakePins(uint8_t portDMask);
```

## 4. lgt::LVD — low-voltage detect

```cpp
// P: LVDThreshold 0-7 (1V8..4V4) via VTH[2:0]
// D: LVDThreshold 0-2 (1.8V / 2.7V / 4.3V) via VDTS[1:0]; >2 → InvalidArgument
Status configure(LVDThreshold threshold, bool resetEnable=true, bool enable=true);
// recovery-safe → Locked
```

## 5. lgt::ADCAdvanced — ADC 12-bit

```cpp
int      readFast(uint8_t pin);                 // single conversion (skip SPN)
Status   channel(uint8_t rawChannel);           // 0-31
Status   channelFromPin(uint8_t pin);           // A0=14 → ch0; D: max ch7

// Trigger
void     trigger(ADCTrigger src, ADCComparatorTrigger comp=TriggerComparator0);
void     disableTrigger();
void     start();  bool busy();  uint16_t result();
void     clearInterruptFlag();  void interrupt(bool yes=true);

// P-only (D → Unsupported / no-op)
void     highSpeed(bool yes=true);              // ADCSRC.SPD
void     fixedOffsetCompensation(bool yes=true); // ADCSRC.OFEN
void     offsetRegisters(int8_t pos, int8_t neg); // OFR0/1
Status   monitor(uint16_t low, uint16_t high, uint8_t consecutive=1);
void     stopMonitor();  bool monitorOverflow();  bool monitorTriggered();
void     differentialEnable(bool yes=true);
Status   configurePGA(ADCPGAPositive pos, ADCPGANegative neg, ADCPGAGain gain, bool enable=true);
void     pgaRaw(uint8_t dapcr);
Status   calibrateOffset(uint8_t samples=8);
```

## 6. lgt::DAC0Advanced / DAC1Advanced — 8-bit DAC

```cpp
// P : DAC0 pada PD4 (DACON/DALR)
// D : DAC0 (IOCR.DACEN0 + DAL0), DAC1 (IOCR.DACEN1 + DAL1) — NEW
void     enable(bool yes=true);
void     output(bool yes=true);          // D: alias enable
void     reference(DACReference r);      // P: DACVcc/External/Internal/Off; D: fixed internal
void     write(uint8_t value);
uint8_t  read();
void     begin(DACReference r=DACVcc, bool externalOutput=true);
void     end();
```

## 7. lgt::Comparator0 / Comparator1 — analog comparator

```cpp
void     enable(bool yes=true);
void     positive(AC0Positive src);      // P: AC0P/ACXP0/AC0DAC/Off; D: ACBG only
void     negative(AC0Negative src);      // via ADCSRB ACME
bool     output();
void     edge(ComparatorEdge e);         // Toggle/Falling/Rising
void     interrupt(bool yes=true);
void     clearFlag();
void     routeToTimer1Capture(bool yes=true);
// P only (D → no-op / filter via OPA):
void     outputPin(bool yes=true);
void     hysteresis(bool yes=true);
void     wake(bool yes=true);
void     filter(ComparatorFilter f);     // P: C0XR.C0FEN; D: OP0CRA.ACFEN + AFTCNT
```

## 8. lgt::Timer0Advanced / Timer1Advanced / Timer2Async / Timer3

```cpp
// Timer0/1 — shared (DTR0/DTR1 ada di kedua silikon)
Status deadTime(uint8_t a, uint8_t b);    // 0-15 ticks each
void   disableDeadTime();
void   faultSources(uint8_t mask);        // Timer0Fault / Timer13Fault bitmask
void   protectA(bool yes=true);  void protectB(bool yes=true);
void   highSpeed(bool yes=true);          // TCKCSR F2XEN (P: pin 64MHz; D: sama)

// Timer1 extra
void   captureEdge(bool rising, bool noiseCancel=false);
uint16_t captured();  void captureInterrupt(bool yes=true);
void   independentPrescaler(bool yes=true);  void resetPrescaler();

// Timer3 — P only (D → stub, duty/output/protect return Unsupported)
void   stop();  void clock(uint8_t csBits);  void mode(uint8_t wgm);
void   fastPwmICR(uint16_t top, uint8_t csBits=1);
Status duty(Channel ch, uint16_t value);   // A/B/C
Status output(Channel ch, bool enable=true, bool inverted=false);
void   deadTime(uint8_t a, uint8_t b);  void faultSources(uint8_t mask);
Status protect(Channel ch, bool yes=true);
void   captureEdge(bool rising, bool noiseCancel=false);
uint16_t captured();  void captureInterrupt(bool yes=true);
void   independentPrescaler(bool yes=true);  void resetPrescaler();

// Timer2Async — shared
Status begin(Source src, uint8_t tccr2a, uint8_t tccr2b, uint8_t counter=0,
             uint8_t compareA=0xff, uint8_t compareB=0xff, uint32_t maxSyncSpins=0);
Status beginExternal32768Hz(...);  Status beginInternal32KHz(...);
bool   updating();  uint8_t updateMask();  Status waitSync(uint32_t maxSpins=0);
void   enable32768Hz();  void enableInternal32KHz();  void disable();
```

## 9. lgt::dsp — uDSC 16-bit DSP accelerator (P only; D → stubs return 0)

```cpp
// Operand/accumulator
void     setX(uint16_t v);  void setY(uint16_t v);
uint16_t getX();  uint16_t getY();
void     setAccumulator(uint32_t v);  uint32_t accumulator();
uint16_t low();  uint16_t high();  uint16_t saturated();

// Flag
uint8_t flags();  bool negativeFlag();  bool zeroFlag();  bool carryFlag();
bool divideDone();  bool divideByZero();

// ALU
void clear();
uint32_t add(uint16_t x, uint16_t y, bool sign=false);
uint32_t sub(uint16_t x, uint16_t y, bool sign=false);
uint32_t loadY(uint16_t y);  uint32_t negY(uint16_t y, bool sign=false);
uint32_t accumulateY(uint16_t y, bool sign=false);
uint32_t subtractY(uint16_t y, bool sign=false);

// Multiply / MAC / MSC
uint32_t mul(uint16_t x, uint16_t y, bool xSigned=false, bool ySigned=false);
uint32_t mulNegative(...);  uint32_t mulHalf(...);  uint32_t mulNegativeHalf(...);
uint32_t mac(uint16_t x, uint16_t y, bool xSigned=false, bool ySigned=false, bool signedAcc=false);
uint32_t macHalf(...);  uint32_t msc(...);  uint32_t mscHalf(...);
uint32_t squareX(uint16_t x, bool sign=false);  uint32_t squareY(uint16_t y, bool sign=false);
uint32_t abs32(bool sign=true);  uint32_t neg32(bool sign=true);

// Shift (raw opcode: 0xC0|n left, 0xD0|n right)
void shiftLeft(uint8_t n);  void shiftRight(uint8_t n, bool arithmetic=true);

// Divide (8 cycles)
struct DivResult { uint32_t quotient; uint16_t remainder; bool zero; };
DivResult divmod(uint32_t dividend, uint16_t divisor);

// 16-bit SRAM alias (R0 <-> DX fast map)
bool     validSram16(const void *p);      // 0x0100-0x08FE even
uint16_t load16(const uint16_t *p);
Status   store16(uint16_t *p, uint16_t value);

// Vector
int32_t  dotProduct(const int16_t *a, const int16_t *b, uint16_t count);
```

## 10. lgt::DriveStrength — HDR high-drive (P only; D → Unsupported)

```cpp
enum HighDrivePin { HighDrive_PD5, HighDrive_PD6, HighDrive_PF1,
                    HighDrive_PF2, HighDrive_PF4, HighDrive_PF5 }; // PF1+ hanya QFP48
Status high(HighDrivePin pin);    // recovery-safe tanpa ALLOW_HIGH_DRIVE → Locked
Status normal(HighDrivePin pin);
bool   isHigh(HighDrivePin pin);
```

## 11. lgt::PinChange — pin-change interrupt

```cpp
Status enable(uint8_t pin);      // set PCMSK + PCICR
Status disable(uint8_t pin);     // clear mask, auto-disable group when empty
Status clearFlag(uint8_t pin);   // PCIFR group
```

## 12. lgt::PinMux — pin function routing

```cpp
// P (PMX0/PMX1/PMX2):
Status oc1bToPF4(bool yes=true);  Status oc1aToPF5(bool yes=true);
Status oc0bToPF3(bool yes=true);
void   spiSsToPB1(bool yes=true);  void txdToPD6(bool yes=true);  void rxdFromPD5(bool yes=true);
Status oc0a(OC0ARoute route);      // PD6/PE4/PC0 combos
Status oc3aToAC0P(bool yes=true);  Status oc2bToPF7(bool yes=true);  Status oc2aToPF6(bool yes=true);
void   pc6AsGPIO(bool yes=true);   void pe6AsGPIO(bool yes=true);  void externalOscillatorPins(bool yes=true);

// D (PMXCR + IOCR):
Status oc0aToPC0(bool yes=true);   // PMXCR.OC0C0
void   spiSsToPB1(bool yes=true);  void txdToPD6(bool yes=true);  void rxdFromPD5(bool yes=true);
void   pc6AsGPIO(bool yes=true);   void pe6AsGPIO(bool yes=true);  void externalOscillatorPins(bool yes=true);
// OC3A/OC2 routes → Unsupported
```

## 13. lgt::SPIDual — dual SPI receive (P only; D → Unsupported)

```cpp
struct SPIDualResult { uint8_t first; uint8_t second; };
Status begin();                       // master receive-only mode
void   end(bool restoreMasterMosi=true);
Status receive2(SPIDualResult &r);
SPIDualResult receive2();
Status receive(void *buffer, size_t bytes);
```

## 14. lgt::USART0Advanced / USARTSPI0 — USART 9-bit + USPI

```cpp
// USART0Advanced
void     modeAsync();  void modeSync(bool master, bool polarity=false);
void     nineBit(bool yes=true);  void multiprocessor(bool yes=true);
void     write9(uint16_t v);  uint16_t read9();  bool available();

// USARTSPI0 (USART-SPI mode)
Status   begin(uint32_t clock, Mode mode=Mode0, bool lsbFirst=false, bool master=true);
uint8_t  transfer(uint8_t v);  void end();
```

## 15. lgt::OPA0 / OPA1 — op-amp (NEW, 328D/E only)

```cpp
enum OPAChannel { OPACh0=0, OPACh1=1, OPABoth=3 };

void     enable(bool yes=true);  bool enabled();
void     channel(OPAChannel ch);            // CH0EN/CH1EN
void     invertCh0(bool yes=true);  void invertCh1(bool yes=true);
uint8_t  activeChannel();                   // ACCH read
void     switchChannel();                   // ACCH write-1
void     enableTimer(bool yes=true);        // OPxCRB.OPTEN
void     setPeriod(uint8_t cycles);         // OPxTCNT (TCSV)
void     setHoldTime(uint8_t t);            // OPxCRB.TCSH (7-bit)
void     comparatorFilter(bool yes=true);   // OPxCRA.ACFEN (+ AFTCNTx)
```

---

## Perbedaan Silicon — Ringkasan

| Fitur | LGT8F328P | LGT8F328D/E |
|---|---|---|
| uDSC, Timer3, PGA, ADC monitor/cal, SPI dual, HDR, IVBASE, DPS2, RCMCAL | ✅ | stub/Unsupported |
| OPA0/OPA1, DAC1, ADTMR differential | ❌ | ✅ NEW |
| DAC0 | DACON/DALR | IOCR.DACEN0 + DAL0 |
| Comparator filter | C0XR/C1XR | OP0CRA/1.ACFEN + AFTCNT |
| Pin mux | PMX0/1/2 | PMXCR + IOCR |
| LVD threshold | 8 level (VTH) | 3 level (VDTS) |
| ADC channels | 12 (PC+PF) | 8 (PC+PE) |
