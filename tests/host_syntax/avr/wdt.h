/* Host stub for <avr/wdt.h>: the real header is AVR-libc; the host syntax
 * harness replaces it with register-level stubs. Bodies are empty because
 * the harness only parses (fsyntax-only) or exercises fake registers. */
#ifndef _AVR_WDT_H_
#define _AVR_WDT_H_

#include <stdint.h>

#define WDTOE 4
#define WDCE  WDTOE
#define WDE   3
#define WDIE  6
#define WDIF  7
#define WDP0  0
#define WDP1  1
#define WDP2  2
#define WDP3  5
#define WDTCSR _SFR_IO8(0x60)

static inline void wdt_reset(void) {}
static inline void wdt_enable(uint8_t) {}
static inline void wdt_disable(void) {}

#endif
