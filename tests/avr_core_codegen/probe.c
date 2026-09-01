#define F_CPU 32000000UL
#include <Arduino.h>

__attribute__((noinline)) void constant_gpio_probe(void) {
  digitalWrite(13, HIGH);
  digitalToggle(13);
}
__attribute__((noinline)) void pwm_semantics_probe(void) {
  digitalWrite(9, HIGH);
}
__attribute__((noinline)) void runtime_gpio_probe(uint8_t pin) {
  digitalWrite(pin, HIGH);
}
