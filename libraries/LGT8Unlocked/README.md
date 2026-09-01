# LGT8Unlocked

`LGT8Unlocked` is the extension layer for the LGT8F328P core. It keeps the normal Arduino API intact and exposes LGT-only silicon features without heap allocation or virtual dispatch.

## Scope

- safe GPIO/pin-change/high-drive helpers
- PMX0/1/2 pin routing
- Timer0/1/3 dead-time, hardware fault shutdown, Timer1/3 capture, Timer0/1 high-speed clock, Timer2 asynchronous clock
- ADC fast path, trigger selection, threshold monitor, fixed-offset calibration, differential PGA
- DAC0 and AC0/AC1 comparator configuration
- uDSC complete documented opcode families plus 16-bit SRAM access helpers
- SPI master dual-receive
- USART 9-bit, multiprocessor, synchronous and USART-SPI modes
- clock calibration/source/prescaler/output, peripheral gating, sleep modes, LVD/reset/GUID/vector relocation/SWD controls
- guarded program-Flash IAP

Include it with:

```cpp
#include <LGT8Unlocked.h>
```

The library targets `__LGT8FX8P__` only. Existing Arduino APIs remain available and are preferred for portable code.

## Safety boundaries

High-drive GPIO is a silicon capability, not permission to drive arbitrary loads. Respect package current, thermal and voltage-drop limits and use an external driver for motors, relays and other power loads.

`FlashIAP` refuses erase/write operations until a writable region is explicitly armed. The caller must still choose a region that does not contain executing code, vectors, bootloader data or emulated EEPROM.

DPS2 loses SRAM and peripheral/register state. Wake from DPS2 is reset-like, not a normal function return in the application model.

## Verification badges

Documentation uses: `[D]` datasheet checked, `[S]` source checked, `[C]` compiled with AVR toolchain, `[T]` automated test, `[H]` hardware verified, `[B]` benchmarked. This tree ships source/static validation; hardware badges are intentionally not asserted without a real board.

See `../../docs/coverage.md`, `../../docs/hardware-verification.md` and `../../docs/datasheet-errata.md`.

## Recovery-safe development build

This handoff defaults to `LGT8_UNLOCKED_RECOVERY_SAFE=1`. Recovery-risk operations are intentionally locked until a hardware recovery path is verified. This RC2 hardware-test build explicitly allows HDR/high-drive GPIO while clamping emulated EEPROM partitioning to the default 1KB logical size. See the repository-root `RECOVERY-SAFE-MODE.md`.
