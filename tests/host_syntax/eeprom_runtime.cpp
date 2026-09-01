#include "Arduino.h"
#include "EEPROM.h"
#include <assert.h>

volatile uint8_t __fake_mem8[512] = {};

int main() {
  assert((ECCR & 0x40u) != 0u);
  assert((ECCR & 0x03u) == 0u);
  assert(lgt_eeprom_size(false) == 1020);

  lgt_eeprom_init(2);
  assert((ECCR & 0x03u) == 0u);
  assert(lgt_eeprom_size(false) == 1020);

  lgt_eeprom_init(8);
  assert((ECCR & 0x03u) == 0u);
  assert(lgt_eeprom_size(false) == 1020);

  assert(EEPROM.change_size(4) == 1020);
  assert((ECCR & 0x03u) == 0u);

  assert(lgt_eeprom_continuous_address_to_real_address(1019) == 1019);
  assert(lgt_eeprom_continuous_address_to_real_address(1020) == 1024);
  return 0;
}
