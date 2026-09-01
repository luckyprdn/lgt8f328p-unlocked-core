#define F_CPU 32000000UL
#include <Arduino.h>

// Non-PWM D13 must collapse to direct SBI and direct PIN toggle. PWM D9 must
// retain a call to the compatibility function so it can disconnect PWM first.
__attribute__((noinline)) void constant_gpio_probe() {
  digitalWrite(13, HIGH);
  digitalToggle(13);
}
__attribute__((noinline)) void pwm_semantics_probe() {
  digitalWrite(9, HIGH);
}
__attribute__((noinline)) void runtime_gpio_probe(uint8_t pin) {
  digitalWrite(pin, HIGH);
}
