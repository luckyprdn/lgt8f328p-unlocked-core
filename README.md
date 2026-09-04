# LGT8F328P Unlocked Core

Arduino-compatible core untuk **LGT8F328P** dan **LGT8F328D/E** — MCU LQFP32/QFP48 buatan LogicGreen, kompatibel pin dengan ATmega328P tapi lebih cepat (32MHz, 12-bit ADC, DSP uDSC, 8KB EEPROM emulasi).

## Fitur

| Fitur | LGT8F328P | LGT8F328D/E |
|-------|-----------|-------------|
| uDSC 16-bit DSP | ✅ Full | 🚫 Stub |
| Timer3 PWM + dead-time | ✅ | 🚫 Stub |
| PGA + ADC monitor | ✅ | 🚫 Stub |
| HDR high-drive GPIO | ✅ 6 pin | 🚫 Stub |
| OPA0/OPA1 op-amp | 🚫 | ✅ **NEW** |
| DAC0 + DAC1 | ✅ | ✅ **DAC1 baru** |
| LGT8Unlocked library | ✅ 33 contoh | ✅ 31 contoh |

## Board Support

- **328P-LQFP32** — Nano style, WAVGAT (chip 32 pin)
- **328P-LQFP48 MiniEVB** — QFP48 (chip 48 pin)
- **328P-LQFP32 wemos-TTGO-XI** — ESP32-like form factor
- **328D** — LGT8F328D/E (chip 32 pin)

## Instalasi

```
Arduino/
  └── hardware/
      └── lgt8funlocked/     ← clone atau extract ZIP di sini
          └── avr/
              ├── boards.txt
              ├── cores/
              └── ...
```

Clone dari GitHub:
```bash
cd ~/Arduino/hardware
git clone https://github.com/luckyprdn/lgt8f328p-unlocked-core.git lgt8funlocked
```

Restart Arduino IDE → Tools → Board → **LGT8F328**.

## LGT8Unlocked Library

Mulai dari **`FeatureTour`** (panduan semua modul, gaya Adafruit), lalu 38 contoh lain: `File → Examples → LGT8Unlocked`.

### High-Level API (Arduino-style)

```cpp
#include <LGT8Unlocked.h>

void setup() {
  AdcExt.readAverage(A0, 8);            // ADC rata-rata
  Dsp.multiply(30000, 2);               // uDSC multiply
  HdDrive.enable(lgt::HighDrive_PD5);  // high-drive GPIO
  Dac.writeMillivolt(0, 1650);          // DAC millivolt
  Pwm.timer1Frequency(1000);            // PWM frekuensi
  Pcint.attach(2, onButton, PCINT_CHANGE); // pin-change interrupt
}
```

Lengkap: 15 objek — `AdcExt`, `Dsp`, `Dac`, `HdDrive`, `Pwm`, `Timer`, `Comp`, `Usart`, `SpiDual`, `PinMux`, `Sys`, `Pwr`, `Opa`, `Pcint`, `Lvd`.

## Safety

Recovery-safe mode aktif secara default. Operasi berbahaya (Flash IAP, clock switch, EEPROM resize, LVD config) akan return `Locked` tanpa programmer. Aman untuk upload via bootloader USB.

## Silicon Verification (LQFP48, 2026-09-04)

Seluruh klaim silicon di bawah ini **verified di hardware asli** (LGT8F328P-LQFP48, test 2026-09-04) — bukan asumsi datasheet. Temuan tiap fitur dicatat di `docs/datasheet-errata.md` (DOC-001..027).

Sketch verifikasi (compile 328P, variant LQFP32 atau LQFP48):

- `File → Examples → LGT8Unlocked → silicon_verify` — 6 test inti uDSC + WDT
- `File → Examples → LGT8Unlocked → final_sweep` — satu-run penuh: byte-accurate (mul/divmod/DSP16/dot), EEPROM 1020B, WDT reset-real, benchmark cycles/op, stress 20k
- `File → Examples → LGT8Unlocked → danger_probe` — verifikasi fitur yang di-flag unsafe/unreliable di silicon ini

Rangkuman errata silicon LQFP48: WDT interrupt mode **tidak ada** (reset-mode saja); EEPROM native 32-bit write **rusak** (byte-engine); DSSD **unstable**; DSP16/extended-op di-SW-compose di atas jalur accumulator 32-bit yang proven (register-pair + nop, DOC-021).

## Dokumentasi

- `docs/api-reference.md` — 15 modul API lengkap
- `docs/datasheet-errata.md` — 27 entri perbedaan datasheet vs implementasi (DOC-001..027, silicon-verified)

## License

GNU Lesser General Public License v2.1