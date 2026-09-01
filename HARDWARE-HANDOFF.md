# LGT8F328P Unlocked — Hardware Handoff

The source package has passed every software-side gate available in the build environment. Your job on real silicon is to turn the remaining `VERIFY-HW` cells into measured results.

## Start here

1. Install/use the real LogicGreen LGT8F Arduino package and its AVR-GCC toolchain.
2. Select the exact board/package and **Default / compatibility** RAM profile first.
3. Compile and upload stock Blink + Serial echo before touching clock, SWD, reset-pin repurpose, IAP or DPS2.
4. Run the examples under `libraries/LGT8Unlocked/examples/` one subsystem at a time.
5. Record board/package, VCC, oscillator, compiler version and source candidate (`0.9.0-source-rc2-safe-hdr-hwtest`) with every result.

## Do not start with destructive features

Leave these until recovery is proven:

- `FlashIAP`
- `disableSWD`
- reset-pin/PC6 repurpose
- clock-source switching
- `IVBASE` relocation
- DPS2
- high-drive GPIO

Use a current-limited supply and a sacrificial/known-unused Flash region for IAP.

## Pass/fail recording

Use `docs/hardware-verification.md` as the checklist. For each test record:

- PASS / FAIL
- exact sketch/example
- expected behavior
- measured behavior
- scope/logic-analyzer capture if timing-related
- current if power-related
- compiler warnings/errors
- minimal repro for any failure

A hardware failure is more valuable than a guessed workaround: preserve the failing configuration so the source can be corrected against real silicon.
