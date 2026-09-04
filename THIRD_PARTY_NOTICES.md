# Third-party notices

This project builds on the work of several upstream projects. Their
contributions are acknowledged here; license texts ship with the respective
upstream sources (see the file headers and the `bootloaders/`, `libraries/`
trees in this repository).

| Component | Origin | License | Note |
|-----------|--------|---------|------|
| LGT8FX platform & libraries (E2PROM, variants, register maps) | [dbuezas/lgt8fx](https://github.com/dbuezas/lgt8fx) | LGPL-2.1 | This core was forked from and is API-compatible with `lgt8fx`; large parts of `libraries/E2PROM` and the variant/register headers derive from it |
| Board definitions base | [LGTMCU/Larduino_HSP](https://github.com/LGTMCU/Larduino_HSP) | — | `boards.txt` states "Based on https://github.com/LGTMCU/Larduino_HSP" |
| Bootloaders (`bootloaders/*/optiboot.c`) | Optiboot project (P. Knight et al.) | GPL-2.0 | Vendor optiboot builds for the LGT8F chips |
| `libraries/usbdrv`, `VUsb*` (software USB) | [V-USB — Objective Development](https://www.obdev.at/products/vusb/index.html), C. Starkjohann | GPL-2.0 with linking exception | `usbdrv.h`: "(c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH" |
| Arduino AVR core heritage (cores, Wire, SPI, SoftwareSerial, String/Print, tone, ...) | Arduino / arduino/Arduino AVR Boards | LGPL-2.1 | Standard Arduino core code under the usual Arduino licensing |
| Datasheets / register documentation | LogicGreen (LGT) | — | Register names and behavior documented against the LGT8FX8P databook; see `docs/datasheet-errata.md` for where silicon behavior differs from the documentation |

If you believe an attribution is missing or incorrect, open an issue.
