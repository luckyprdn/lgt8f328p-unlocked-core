# Datasheet ambiguity / errata log

Basis: LGT8FX8P Programming Manual / Data Book v1.0.5. This log records
contradictions between the documentation and the implementation instead of
silently choosing one side. Status conventions:

- **Verified (LQFP48, 2026-09-04)** — confirmed by hardware test on the
  LGT8F328P-LQFP48 die used for this project.
- **Open — needs <instrument>** — resolution applied; confirmation requires
  equipment not available for this project (the header or library already
  follows the decision).
- **Retracted / Superseded** — entry no longer describes current behavior.

Add new entries whenever the implementation depends on ambiguous wording.
Measured hardware behavior wins over documentation prose.

| ID | Conflict | Resolution | Status |
|---|---|---|---|
| DOC-001 | Databook states 131 instructions (overview) vs 130 (core chapter). | No API depends on the instruction count. | Closed — not applicable. |
| DOC-002 | ADC described as 8-channel (overview) vs 12 single-ended (feature list / ADC chapter). | ADC chapter + package pin table are authoritative. | Open — needs voltage source for full channel sweep. |
| DOC-003 | uDSC accumulator described inconsistently; register tables define DX/DY 16-bit, DA 32-bit (DSAL/DSAH). | Register/opcode tables are authoritative. | Verified (LQFP48) — see DOC-021. |
| DOC-004 | WDT summary says max 8 s; WDP table extends to 32 s @ 32 kHz. | WDT library keeps the WDP table timing; no claim changed without measurement. | Open — needs timer measurement at 32 s. |
| DOC-005 | PMCR protected-write prose: six cycles vs four. | Shortest safe sequence: unlock then write, interrupts disabled. | Open — stress test requires instrumented setup. |
| DOC-006 | VDTCR protected-write: same six-vs-four inconsistency. | Immediate unlock/write, interrupts disabled. | Open — stress test requires instrumented setup. |
| DOC-007 | TCKCSR F2XEN described as both 1=enable and 1=disable. | Treated as enable (consistent with the 64 MHz activation text). | Open — needs scope on Timer0/1 high-speed output. |
| DOC-008 | Vector relocation alignment differs between early prose and the interrupt chapter. | 512-byte alignment per the IVBASE section. | Open — needs relocation firmware test. |
| DOC-009 | Translated register prose contains naming typos (e.g. DTR1L called high byte). | Functional sentence + register name win over the typo label. | Open — needs scope on PWM dead-time. |
| DOC-010 | TCCR3C summary puts DOC32 at bit 2; detailed bit description has DOC3C/DOC32 at bit 1. | Header follows the detailed description; Timer3::protect() ch C uses bit 1. | Closed — compile-checked; summary table wrong. |
| DOC-011 | PMCR narrative maps OSCMEN bit 2 / OSCKEN bit 3; summary table maps OSCKEN 3 / OSCMEN 2 (matches header). | Header follows the summary table. | Closed — compile-checked. |
| DOC-012 | MCUCR bit 5 named EXRFD (LogicGreen spec) vs SWR (summary table); true SWR is VDTCR bit 6. | Header keeps EXRFD at MCUCR bit 5; SWR only on VDTCR bit 6. | Closed — compile-checked. |
| DOC-013 | ADCSRB ACMExx vs datasheet CMExx naming (same bits 7:4). | Header uses AVR-convention ACMExx names; positions identical. | Closed — compile-checked. |
| DOC-014 | PRR1 bit 3 labelled PRTC3; logical name PRTIM3. | Header defines PRTIM3 with alias PRTC3. | Closed — compile-checked. |
| DOC-015 | CLKPR output-enable bits: CLKOE1/0 vs CKOE1/0. | Header defines CKOE1/CKOE0 with backward-compatible aliases. | Closed — compile-checked. |
| DOC-016 | TCKCSR bit 2 AFCKS present in v1.0.5 but missing from the header; unused. | AFCKS = 2 added for full datasheet coverage. | Closed — compile-checked. |
| DOC-017 | EEPROM partition >1KB overlaps the bootloader (2KB starts 0x7000 vs bootloader 0x7400-77FF). | Recovery-safe clamps to 1KB; change_size() above 1KB is a no-op. | Verified (LQFP48) — E2END = 1019. |
| DOC-018 | DSUEN left high while AVR code runs can mis-branch BRTS/SBRC (silicon quirk). | Every dsp op is a scoped, interrupt-masked transaction: enable -> op -> read -> disable. | Verified (LQFP48) — 20k stress checksum match. |
| DOC-019 | ~~X high byte written to the adjacent IO slot~~ — model wrong; IO 0x11 is DSDY itself and writing there clobbered Y. | Reverted to single pair-transfer out/in (see DOC-021). | Retracted — replaced by DOC-021. |
| DOC-020 | ~~WDT interrupt-only mode needs a `.init0` trampoline~~ — trampoline never intercepted the WDT vector. | Trampoline removed; interrupt-mode expectation dropped (see DOC-023). | Superseded — see DOC-023. |
| DOC-021 | uDSC 16-bit access is a register-pair transfer; uDSC samples the high byte of the pair first (pipeline hazard). | Pair-`out`/`in` with a `nop` between operand load and transfer. | Verified (LQFP48) — dot/divmod/saturate exact. |
| DOC-022 | DSSD (IO 0x02, "16-bit saturation result") is unstable — values differ per operation and boot; not a usable field. | DSP16 reads the 32-bit accumulator and clamps in software (`sat16`). | Verified (LQFP48) — 30000*2 saturates to 32767. |
| DOC-023 | WDT interrupt mode never fires on LQFP48 (WDIE armed, ISR installed, timeout always resets). | WDT library is reset-mode only; `wdt_ienable()` kept for API compatibility. | Verified (LQFP48) — arm/feed/disarm + real-reset proof. |
| DOC-024 | Native 32-bit EEPROM program mode only lands byte 0 on LQFP48. | write32/writeSWM use the byte-mode engine (4x byte writes). | Verified (LQFP48) — full 1020B sweep + SWM + OOB. |
| DOC-025 | Two uDSC quirks: DIVMOD is signed 32-bit (0xFFFFFFFF divides as -1); a second consecutive MAC with negative product double-adds. | divmod() falls back to SW for dividends >= 0x80000000; DSP16 +/- use single-op ADD/SUB; mac() = one signed mul + SW accumulate. | Verified (LQFP48) — 80-case divmod + 81-combo DSP16 matrix. |
| DOC-026 | Extended op variants: mulNegative (IR neg bit) returns 0; macHalf ignores the half bit. | mulNegative/mulNegativeHalf composed in SW from proven mul/mulHalf. | Verified (LQFP48) — mulNegative(200,3) = -600. |
| DOC-027 | DSP16 `/` and `%` wrapped at INT16_MIN and negating quotient 32768 was UB; also routed int16/int16 through the slower 32/16 uDSC divmod. | `/` and `%` compute in int32 and clamp with sat16; divmod stays for genuine uint32/uint16 divides. | Closed — saturating contract; -32768/-1 = 32767 (T3 matrix + D8). |
| DOC-028 | (perf attempt, reverted) EEPROM skip-identical-byte writes: read-before-program reset the chip mid-sweep on LQFP48. | Skip removed; the proven byte engine is restored. All program paths busy-wait EEPE self-clear before returning. | Verified (LQFP48) — sweep stable again; do not reintroduce without a dedicated probe. |
| DOC-029 | (perf) dsp::mul ran a full uDSC transaction (~63 cyc) even for byte-sized operands (~6 cyc native). | mul() takes a SW byte fast-path when both operands < 0x100. | Closed — byte-identical by construction; covered by the 256-pair matrix. |
| DOC-030 | Native wide EEPROM program modes (word 0x20, 32-bit 0x40) erase or corrupt cells and cost ~390 us/cell vs ~3 us byte-mode. | Byte engine stays the only write path. | Verified (LQFP48) — eeprom16 probe E3/E5 fail, E4 timing. |
| DOC-031 | (perf measurements) standalone uDSC mul: 102 cyc/op vs 77 SW; dotProductFast == dotProduct at n=16 and n=64. | Guidance: uDSC for divmod (158 vs 648) and dot/FIR batches (310 vs 722); SW for standalone 16-bit mul; dotProductFast kept with no perf claim. | Measured (LQFP48) — final_sweep BENCH rows. |
