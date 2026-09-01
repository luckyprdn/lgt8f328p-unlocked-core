#define F_CPU 32000000UL
#include <Arduino.h>

/*
 * Instantiating pins_arduino.h under ARDUINO_MAIN is the test: the production
 * header owns static assertions for port/bit/timer table lengths.  These tiny
 * references also ensure the linker-visible arrays are emitted in the object.
 */
uint8_t qfp48_pin_table_probe(uint8_t pin) {
  if (pin >= NUM_PIN_TABLE_ENTRIES) return NOT_A_PIN;
  return (uint8_t)(digitalPinToPort(pin) ^ digitalPinToTimer(pin));
}
