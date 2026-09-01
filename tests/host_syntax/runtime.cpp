#include <assert.h>
#include <string.h>
#include <LGT8Unlocked.h>
#include "../../libraries/LGT8Unlocked/src/lgt/flash_iap.h"

volatile uint8_t __fake_mem8[512];

static void reset_regs() {
  for (size_t i = 0; i < sizeof(__fake_mem8); ++i) __fake_mem8[i] = 0;
}

int main() {
  reset_regs();

  // Protected-write helpers must not leave their write-enable bit asserted.
  lgt::PinMux::oc1aToPF5(true);
  assert((PMX0 & _BV(5)) != 0);
  assert((PMX0 & _BV(7)) == 0);
  lgt::PinMux::oc1aToPF5(false);
  assert((PMX0 & _BV(5)) == 0);

  lgt::PinMux::pc6AsGPIO(true);
  assert((PMX2 & _BV(0)) != 0);
  assert((PMX2 & _BV(7)) == 0);

  lgt::Clock::enable(lgt::Internal32M, true);
  assert((PMCR & _BV(RCMEN)) != 0);
  assert((PMCR & _BV(7)) == 0);
  uint8_t pmcr_before = PMCR;
  assert(lgt::Clock::switchTo(lgt::ExternalHigh, 1000) == lgt::Locked);
  assert(lgt::Clock::select(lgt::Internal32K) == lgt::Locked);
  assert(PMCR == pmcr_before);

  // Recovery-safe build must block reset-generating LVD writes.
  VDTCR = 0;
  assert(lgt::LVD::configure(lgt::LVD_2V9, true, true) == lgt::Locked);
  assert(VDTCR == 0);

  // ADTS=0 is continuous/free-running but still requires ADATE.
  ADCSRA = 0;
  ADCSRB = 0xF0;
  lgt::ADCAdvanced::trigger(lgt::ADCContinuous);
  assert((ADCSRA & _BV(ADATE)) != 0);
  assert((ADCSRB & 0x07u) == 0);
  lgt::ADCAdvanced::trigger(lgt::ADCTimer1Capture, lgt::TriggerComparator1);
  assert((ADCSRB & 0x07u) == 7u);
  assert((ADCSRB & _BV(ACTS)) != 0);
  lgt::ADCAdvanced::disableTrigger();
  assert((ADCSRA & _BV(ADATE)) == 0);

  // Timer3 WGM=14 (fast PWM, ICR3 TOP) register encoding.
  TCCR3A = TCCR3B = 0;
  lgt::Timer3::mode(14);
  assert((TCCR3A & 0x03u) == 0x02u);
  assert((TCCR3B & (_BV(WGM33) | _BV(WGM32))) == (_BV(WGM33) | _BV(WGM32)));

  HDR = 0;
  assert(lgt::DriveStrength::high(lgt::HighDrive_PF5) == lgt::Ok);
  assert((HDR & _BV(5)) != 0);
  assert(lgt::DriveStrength::normal(lgt::HighDrive_PF5) == lgt::Ok);
  assert((HDR & _BV(5)) == 0);


  // Timer2 asynchronous setup must select the requested source, preserve the
  // user's interrupt mask and reject invalid/busy transitions predictably.
  TIMSK2 = (uint8_t)(_BV(OCIE2A) | _BV(TOIE2));
  ASSR = 0;
  assert(lgt::Timer2Async::beginInternal32KHz(0x12, 0x03, 7, 9, 11, 100) == lgt::Ok);
  assert((ASSR & (_BV(INTCK) | _BV(AS2))) == (_BV(INTCK) | _BV(AS2)));
  assert(TCCR2A == 0x12 && TCCR2B == 0x03 && TCNT2 == 7 && OCR2A == 9 && OCR2B == 11);
  assert(TIMSK2 == (uint8_t)(_BV(OCIE2A) | _BV(TOIE2)));

  // Dual-SPI setup clears FIFOs once and restores normal master pin direction
  // on exit.  Data transfer itself is hardware/AVR gated because fake SPIF
  // cannot model the serial engine.
  SPCR = (uint8_t)(_BV(MSTR) | _BV(SPE));
  DDRB = 0xff; SPSR = 0; SPFR = 0;
  assert(lgt::SPIDual::begin() == lgt::Ok);
  assert((SPSR & _BV(DUAL)) != 0);
  assert((DDRB & (_BV(DDB3) | _BV(DDB4))) == 0);
  assert((SPFR & (_BV(RDEMPT) | _BV(WREMPT))) == (_BV(RDEMPT) | _BV(WREMPT)));
  lgt::SPIDual::end();
  assert((SPSR & _BV(DUAL)) == 0);
  assert((DDRB & _BV(DDB3)) != 0 && (DDRB & _BV(DDB4)) == 0);

  // USART-SPI cleanup must not leak DORD/UCPHA bits into UART frame size.
  UCSR0C = 0xFF;
  UCSR0B = 0xFF;
  lgt::USARTSPI0::end();
  assert(UCSR0B == 0);
  assert(UCSR0C == (uint8_t)(_BV(UCSZ01) | _BV(UCSZ00)));

  // IAP partition math must follow ECCR partition selection without touching HW.
  ECCR = 0;
  assert(lgt::FlashIAP::programLimit() == 32768u);
  ECCR = _BV(EEN) | 0u;
  assert(lgt::FlashIAP::programLimit() == 30720u);
  ECCR = _BV(EEN) | 1u;
  assert(lgt::FlashIAP::programLimit() == 28672u);
  ECCR = _BV(EEN) | 2u;
  assert(lgt::FlashIAP::programLimit() == 24576u);
  ECCR = _BV(EEN) | 3u;
  assert(lgt::FlashIAP::programLimit() == 16384u);
  lgt::FlashIAP flash;
  assert(flash.writableRegion(0x7000u, 0x7400u) == lgt::Locked);
  assert(flash.erasePage(0x7000u) == lgt::Locked);
  assert(flash.write32(0x7000u, 0x12345678UL) == lgt::Locked);

  MCUCR = 0;
  IVBASE = 0;
  assert(lgt::System::relocateVectors(0x0100u) == lgt::Locked);
  assert(lgt::System::relocateVectors(0x0200u) == lgt::Locked);
  assert(IVBASE == 0u && MCUCR == 0u);
  assert(lgt::System::reloadConfiguration() == lgt::Locked);
  MCUSR = 0;
  assert(lgt::System::disableSWD() == lgt::Locked);
  assert(MCUSR == 0);

  DPS2R = 0;
  assert(lgt::Power::dps2Timer(4) == lgt::InvalidArgument);
  assert(DPS2R == 0);
  assert(lgt::Power::dps2Timer(2) == lgt::Locked);
  assert(DPS2R == 0);
  assert(lgt::Power::dps2Enable(true) == lgt::Locked);
  assert(lgt::Power::dps2() == lgt::Locked);

  PRR = 0;
  lgt::Power::disable(lgt::Power::PeripheralSPI);
  assert((PRR & _BV(2)) != 0);
  lgt::Power::enable(lgt::Power::PeripheralSPI);
  assert((PRR & _BV(2)) == 0);

  return 0;
}
