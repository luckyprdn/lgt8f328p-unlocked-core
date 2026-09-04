# Hardware verification

This project only ships silicon behavior claims that have been exercised on
real hardware. Hardware runs happened on an LGT8F328P-LQFP48 module
(2026-09-04). This page maps every verification sketch to what it proves and
which DOC entries it covers.

## Verification sketches (File > Examples > LGT8Unlocked)

| Sketch | What it proves | DOC coverage |
|--------|----------------|--------------|
| `silicon_verify` | Core gates: uDSC dot/divmod/saturate, WDT reset arm/feed, ADC sanity | 021, 022, 023, 025 |
| `final_sweep` | Full byte-accurate sweep: mul 256 pairs, divmod 80 cases, DSP16 81 combos, dot + convolve, EEPROM 1020B sweep + 32-bit + SWM + OOB, WDT real-reset proof (reboot counted via noinit), RTC 1 Hz, PRR domain roundtrip, cycle benchmarks, 20k uDSC stress under interrupts | 018, 021, 022, 023, 024, 025, 026, 027, 028, 029, 030, 031 |
| `danger_probe` | Re-checks each feature flagged unreliable on this die (DA-at-boot, DSSD, EEPROM fallback, SW-composed variants, WDT state, DSUEN guard, config snapshot, DSP16 saturation, OOB refusal, DSU discipline, RTC, PRR) | 022, 023, 024, 025, 026, 027, 028, 029, 030 |
| `HardwareSmokeSafePro` | Production smoke: uDSC, EEPROM incl. partition lock, WString edges, HDR, timer3 N/A guard, recovery-safe locks | 017, 024 |

## Result of the certified run (2026-09-04, LQFP48)

```
final_sweep: pass=11  fail=0  skip=2 (T7 SPI loopback, T12 ADC - need jumpers)
danger_probe: all checks PASS
```

## Remaining open hardware items

These need equipment, not code; see `datasheet-errata.md` status column:

- DOC-002 channel sweep, DOC-004 WDT 32 s timing, DOC-005/006 protected-write
  stress, DOC-007/009 scope checks, DOC-008 relocation test — require a
  voltage source, scope, or stress rig.
- SPI FIFO loopback (T7) and ADC on a tied pin (T12): two jumper wires.
- INL/DNL and PGA gain accuracy: precision voltage source.
- Sleep-current per mode: ammeter.
