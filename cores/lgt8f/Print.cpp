/*
 Print.cpp - Base class that provides print() and println()
 Copyright (c) 2008 David A. Mellis.  All right reserved.
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
 Modified 23 November 2006 by David A. Mellis
 Modified 03 August 2015 by Chuck Todd
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Arduino.h"

#include "Print.h"

// Public Methods //////////////////////////////////////////////////////////////

/* default implementation: may be overridden */
size_t Print::write(const uint8_t *buffer, size_t size)
{
  size_t n = 0;
  while (size--) {
    if (write(*buffer++)) n++;
    else break;
  }
  return n;
}

size_t Print::print(const __FlashStringHelper *ifsh)
{
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);
  size_t n = 0;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    if (write(c)) n++;
    else break;
  }
  return n;
}

size_t Print::print(const String &s)
{
  return write(s.c_str(), s.length());
}

size_t Print::print(const char str[])
{
  return write(str);
}

size_t Print::print(char c)
{
  return write(c);
}

size_t Print::print(unsigned char b, int base)
{
  return print((unsigned long) b, base);
}

size_t Print::print(int n, int base)
{
  return print((long) n, base);
}

size_t Print::print(unsigned int n, int base)
{
  return print((unsigned long) n, base);
}

size_t Print::print(long n, int base)
{
  if (base == 0) {
    return write(n);
  } else if (base == 10) {
    if (n < 0) {
      int t = print('-');
      n = -n;
      return printNumber(n, 10) + t;
    }
    return printNumber(n, 10);
  } else {
    return printNumber(n, base);
  }
}

size_t Print::print(unsigned long n, int base)
{
  if (base == 0) return write(n);
  else return printNumber(n, base);
}

size_t Print::print(double n, int digits)
{
  return printFloat(n, digits);
}

size_t Print::println(const __FlashStringHelper *ifsh)
{
  size_t n = print(ifsh);
  n += println();
  return n;
}

size_t Print::print(const Printable& x)
{
  return x.printTo(*this);
}

size_t Print::println(void)
{
  return write("\r\n");
}

size_t Print::println(const String &s)
{
  size_t n = print(s);
  n += println();
  return n;
}

size_t Print::println(const char c[])
{
  size_t n = print(c);
  n += println();
  return n;
}

size_t Print::println(char c)
{
  size_t n = print(c);
  n += println();
  return n;
}

size_t Print::println(unsigned char b, int base)
{
  size_t n = print(b, base);
  n += println();
  return n;
}

size_t Print::println(int num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(unsigned int num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(long num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(unsigned long num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(double num, int digits)
{
  size_t n = print(num, digits);
  n += println();
  return n;
}

size_t Print::println(const Printable& x)
{
  size_t n = print(x);
  n += println();
  return n;
}

// Private Methods /////////////////////////////////////////////////////////////

// uDSC-accelerated 32/16 division for number formatting.
//
// LGT8F328P carries a uDSC co-processor whose DIVMOD op (IR 0xB1) computes a
// full 32/16 division in ~158 cycles vs several hundred for the libgcc
// software routine used by n/base below (this die has no DIV/MUL instruction).
// Every decimal/hex/octal digit printed by printNumber used to cost two
// software long divisions; routing them through the uDSC makes Serial/FILE
// number output roughly 5-10x cheaper in CPU cycles.
//
// This block is a byte-for-byte mirror of the silicon-verified 32/16 divmod
// in LGT8Unlocked/src/lgt/udsc.h (DOC-021 register-pair access, DOC-025
// signed-divider quirk + fallback). Keep the two in sync. Guarded on DSCR so
// the uDSC-less LGT8F328D/E/88A variants compile the plain path unchanged.
#if defined(DSCR)
namespace {
// DOC-018: keep the DSU transaction atomic w.r.t. interrupts - an IRQ that
// lands between DSUEN set and clear could corrupt AVR control flow. The
// library divmod relies on the short window; the core print path additionally
// masks IRQs for the ~20-cycle transaction because print() runs everywhere.
static inline void print_fastDivMod(uint32_t d, uint16_t v, uint32_t *q, uint16_t *r) {
  if (v == 0u) { *q = 0ul; *r = 0u; return; }              // never happens (base>=2)
  if (d >= 0x80000000ul) { *q = d / v; *r = (uint16_t)(d % v); return; } // DOC-025 fallback
  uint8_t s = SREG; cli();
  DSCR |= _BV(DSUEN);                                       // enable
  { uint16_t lo = (uint16_t)d, hi = (uint16_t)(d >> 16);    // setAccumulator
    __asm__ __volatile__("nop" "\n\t" "out %0,%A1"::"I"(_SFR_IO_ADDR(DSAL)),"w"(lo):"memory");
    __asm__ __volatile__("nop" "\n\t" "out %0,%A1"::"I"(_SFR_IO_ADDR(DSAH)),"w"(hi):"memory"); }
  __asm__ __volatile__("nop" "\n\t" "out %0,%A1"::"I"(_SFR_IO_ADDR(DSDY)),"w"(v):"memory"); // setY
  DSIR = 0xB1u;                                             // OP_DIVMOD
  while (!(DSCR & _BV(DSD1))) {}                            // waitDivision
  { uint16_t lw, hw;                                        // accumulator()
    __asm__ __volatile__("in %A0,%1":"=w"(lw):"I"(_SFR_IO_ADDR(DSAL)):"memory");
    __asm__ __volatile__("in %A0,%1":"=w"(hw):"I"(_SFR_IO_ADDR(DSAH)):"memory");
    *q = (uint32_t)lw | ((uint32_t)hw << 16); }
  __asm__ __volatile__("in %A0,%1":"=w"(*r):"I"(_SFR_IO_ADDR(DSDY)):"memory"); // remainder
  DSCR &= (uint8_t)~_BV(DSUEN);                             // disable
  SREG = s;
}
}
#endif

size_t Print::printNumber(unsigned long n, uint8_t base)
{
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];

  *str = '\0';

  // prevent crash if called with base == 1
  if (base < 2) base = 10;

#if defined(DSCR)
  do {
    uint32_t nq; uint16_t nr;
    print_fastDivMod(n, (uint16_t)base, &nq, &nr);          // uDSC 32/16 DIVMOD
    char c = (char)nr;
    n = nq;
    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while (n);
#else
  do {
    char c = n % base;
    n /= base;

    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);
#endif

  return write(str);
}

size_t Print::printFloat(double number, uint8_t digits) 
{ 
  size_t n = 0;
  
  if (isnan(number)) return print("nan");
  if (isinf(number)) return print("inf");
  if (number > 4294967040.0) return print ("ovf");  // constant determined empirically
  if (number <-4294967040.0) return print ("ovf");  // constant determined empirically
  
  // Handle negative numbers
  if (number < 0.0)
  {
     n += print('-');
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i)
    rounding /= 10.0;
  
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  n += print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0) {
    n += print('.'); 
  }

  // Extract digits from the remainder one at a time
  while (digits-- > 0)
  {
    remainder *= 10.0;
    unsigned int toPrint = (unsigned int)(remainder);
    n += print(toPrint);
    remainder -= toPrint; 
  } 
  
  return n;
}
