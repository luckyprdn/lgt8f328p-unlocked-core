from pathlib import Path
root = Path(__file__).resolve().parents[2]
common = (root/'libraries/LGT8Unlocked/src/lgt/safety.h').read_text()
agg = (root/'libraries/LGT8Unlocked/src/LGT8Unlocked.h').read_text()
system = (root/'libraries/LGT8Unlocked/src/lgt/system.h').read_text()
flash = (root/'libraries/LGT8Unlocked/src/lgt/flash_iap.h').read_text()
gpio = (root/'libraries/LGT8Unlocked/src/lgt/gpio.h').read_text()
assert '#define LGT8_UNLOCKED_RECOVERY_SAFE 1' in common
assert '#if !LGT8_UNLOCKED_RECOVERY_SAFE\n#include "lgt/flash_iap.h"' in agg
for needle in ['disableSWD()', 'relocateVectors(', 'switchTo(', 'dps2()', 'reloadConfiguration()', 'LVD']:
    assert needle in system
assert system.count('return Locked;') >= 7
assert flash.count('LGT8_UNLOCKED_RECOVERY_SAFE') >= 3
assert 'LGT8_UNLOCKED_ALLOW_HIGH_DRIVE' in gpio and 'DriveStrength' in gpio
assert not (root/'libraries/LGT8Unlocked/examples/FlashIAP').exists()
print('recovery-safe static interlocks PASS')
platform = (root/'platform.txt').read_text()
wiring = (root/'cores/lgt8f/wiring_digital.c').read_text()
lowp = (root/'libraries/lgt_LowPower/src/lgt_LowPower.h').read_text()
assert 'build.extra_flags=-DLGT8_UNLOCKED_RECOVERY_SAFE=1 -DLGT8_UNLOCKED_ALLOW_HIGH_DRIVE=1' in platform
assert 'Recovery-safe build: E0/E2 remain owned by SWD/SWC' in wiring
assert '#if !defined(LGT8_UNLOCKED_RECOVERY_SAFE) || !LGT8_UNLOCKED_RECOVERY_SAFE' in lowp
assert not (root/'libraries/PMU').exists()
assert not (root/'libraries/lgt_LowPower/examples/powerDown-Dps2').exists()
print('core/legacy recovery interlocks PASS')
eeprom_cpp = (root/'libraries/E2PROM/EEPROM.cpp').read_text()
assert 'number_of_1KB_pages = 1u;' in eeprom_cpp
assert 'runtime API calls cannot grow the EEPROM into' in eeprom_cpp
print('EEPROM partition lock PASS')

