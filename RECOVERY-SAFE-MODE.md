# Recovery-Safe Mode — RC2 hardware-test profile

`LGT8_UNLOCKED_RECOVERY_SAFE=1` remains enabled.

`LGT8_UNLOCKED_ALLOW_HIGH_DRIVE=1` is the only deliberate exception: HDR0..HDR5 high-drive control is enabled.

Still locked: Flash IAP erase/write, SWD disable/takeover, IVBASE relocation, main-clock source switching, RC calibration writes, DPS2, LVD mutation, protected configuration reload, legacy PMU, and runtime EEPROM partition resizing.

EEPROM byte/block/32-bit/SWM access remains enabled inside the default 1KB logical partition (1020 user bytes). `lgt_eeprom_init()` and all `EEPROM.change_size()` API variants are clamped to 1KB in recovery-safe builds.

## Electrical warning
Datasheet labels HDR pins as 80 mA drive versus 12 mA normal drive. This is not permission to continuously source/sink 80 mA into arbitrary loads. Test unloaded or through a safe load/series resistor first; never short an HDR output to GND/VCC or another actively-driven pin.
