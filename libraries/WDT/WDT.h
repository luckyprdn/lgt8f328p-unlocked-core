/**************************************************************************
 * WDT.h - Watchdog library for LogicGreen LGT8F328x microcontrollers
 *
 * SILICON NOTE (DOC-023, verified 2026-09-04 on LGT8F328P-LQFP48):
 * WDT INTERRUPT MODE (WDIE=1, WDE=0) DOES NOT WORK on tested silicon.
 * Extensive probe evidence: WDTCSR reads back 0x40 (WDE clean, interrupt
 * mode armed), ISR properly installed on __vector_6, yet the timeout
 * RESETS the chip and the ISR never fires - regardless of windowed
 * re-arm/disarm inside the ISR. The databook "interrupt mode" row does
 * not match this die. USE RESET MODE ONLY: wdt_enable(timeout) + wdr
 * feeding, or wdt_disable() to stop. wdt_ienable() is kept for API
 * compatibility but on this silicon it behaves as a reset-mode arm.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 **************************************************************************/

#ifndef WDT_h
#define WDT_h

#include "Arduino.h"
#include <avr/wdt.h>

#define WTOH_32MHZ 0
#define WTO_32KHZ 1

#define WTOH_1MS 0
#define WTOH_2MS 1
#define WTOH_4MS 2
#define WTOH_8MS 3
#define WTOH_16MS 4
#define WTOH_32MS 5
#define WTOH_64MS 6
#define WTOH_128MS 7
#define WTOH_256MS 8
#define WTOH_512MS 9

#define WTO_64MS 0
#define WTO_128MS 1
#define WTO_256MS 2
#define WTO_512MS 3
#define WTO_1S 4
#define WTO_2S 5
#define WTO_4S 6
#define WTO_8S 7
#define WTO_16S 8
#define WTO_32S 9

class LGTWDT {
public:
  static void begin(uint8_t clockSource = WTO_32KHZ)
      __attribute__((always_inline)) {
    uint8_t btmp = 0;

    if (clockSource == WTOH_32MHZ) {
      // enable 32MHz RC for WDT
      btmp = PMCR | (1 << RCMEN);  // 0000 0001 (enable RCMEN)
#ifdef WCLKS
      btmp = btmp & ~(1 << WCLKS); // 1110 1111 (select HFRC) - 328P only
#else
      // 328D/E: WDT clock source is fixed (no PMCR.WCLKS select); PMCR
      // RCMEN/RCKEN lines below still apply where the bits exist.
#endif
      uint8_t sreg=SREG; cli();
      PMCR = (1 << PMCE);          // 1000 0000
      PMCR = btmp;
      SREG=sreg;
    } else {
      // enable 32KHz RC for WDT
      btmp = PMCR | (1 << RCKEN); // 0000 0010 (enable RCKEN)
#ifdef WCLKS
      btmp = btmp | (1 << WCLKS); // 0001 0000 (select LFRC) - 328P only
#endif
      uint8_t sreg=SREG; cli();
      PMCR = (1 << PMCE);         // 1000 0000
      PMCR = btmp;
      SREG=sreg;
    }
  }
};

// DOC-023: on LGT8F328P-LQFP48 silicon this arms WDIE-only mode, but the
// timeout RESETS instead of interrupting (interrupt mode unsupported).
// Both stores are in ONE asm block so the WDTOE 4-cycle window is respected.
static void __inline__ __attribute__((__always_inline__))
wdt_ienable(const uint8_t value) {
  __asm__ __volatile__(
      "in     __tmp_reg__,__SREG__    \n\t"
      "cli                            \n\t"
      "wdr                            \n\t"
      "sts    %[wdtcsr], %[wdce_wde]  \n\t"
      "sts    %[wdtcsr], %[wdie_val]  \n\t"
      "out    __SREG__,__tmp_reg__    \n\t"
      : /* no outputs */
      : [wdtcsr] "n"(_SFR_MEM_ADDR(WDTCSR)),
        [wdce_wde] "r"((uint8_t)((1 << WDCE) | (1 << WDE))),
        [wdie_val] "r"((uint8_t)((1 << WDIE) | (value & 0x08 ? 0x20 : 0x00) |
                                 (value & 0x07)))
      : "r0");
}

extern LGTWDT Lgtwdt;

#endif
