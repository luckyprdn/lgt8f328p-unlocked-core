# Installing LGT8F328 unlocked-core

Three ways to install, from the most convenient to the most manual.

## 1. Boards Manager (recommended)

1. Arduino IDE: **File → Preferences → Settings**, *Additional Boards
   Manager URLs* → add:

   ```
   https://raw.githubusercontent.com/luckyprdn/lgt8f328p-unlocked-core/main/package_lgt8funlocked_index.json
   ```

2. **Tools → Board → Boards Manager**, search `LGT8F328`, install
   **LGT8Fx Boards** (the Arduino AVR toolchain is pulled in automatically).

3. Select **Board → LGT8F328**, then the matching **Variant**
   (328P / 328P LQFP48 / 328D). Upload speed is fixed at 57600.

CLI:

```bash
arduino-cli core update-index \
  --additional-urls https://raw.githubusercontent.com/luckyprdn/lgt8f328p-unlocked-core/main/package_lgt8funlocked_index.json
arduino-cli core install lgt8funlocked:avr
arduino-cli core list   # lgt8funlocked:avr 1.0.3
```

Updates arrive as new releases: run `core update-index` + `core upgrade`.

## 2. Release zip (manual, no internet at IDE time)

Download `lgt8f-unlocked-v1.0.3.zip` from the
[latest release](https://github.com/luckyprdn/lgt8f328p-unlocked-core/releases),
then extract so the layout is:

```
<sketchbook>/hardware/
  └── lgt8funlocked/
      └── avr/       ← boards.txt, cores/, libraries/, ...
```

Restart the IDE.

## 3. Git clone (for development)

```bash
cd ~/Arduino/hardware
git clone https://github.com/luckyprdn/lgt8f328p-unlocked-core.git lgt8funlocked
```

Uninstall is just deleting the `lgt8funlocked` folder (Boards Manager) or
`<sketchbook>/hardware/lgt8funlocked` (manual).

## First run

File → Examples → **LGT8Unlocked → FeatureTour** is a guided tour of every
feature; `silicon_verify` is the fast self-test that exercises the DSP core,
EEPROM and watchdog.
