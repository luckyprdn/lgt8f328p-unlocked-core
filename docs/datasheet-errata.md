# Datasheet ambiguity / errata log

Basis: LGT8FX8P Programming Manual / Data Book v1.0.5. This file records contradictions instead of silently choosing one statement.

| ID | Conflict | Implementation decision | Verification |
|---|---|---|---|
| DOC-001 | Overview says 131 instructions; core chapter says 130. | No API depends on the count. | Count instruction table if needed. |
| DOC-002 | System overview text labels ADC as 8-channel while feature list/ADC chapter documents 12 single-ended channels on LGT8F328P. | Use ADC chapter + package pin table. | Hardware channel sweep. |
| DOC-003 | uDSC overview wording labels the accumulator inconsistently; detailed register table defines DX/DY as 16-bit and DA as 32-bit (DSAL/DSAH). | Follow detailed uDSC register/opcode tables. | uDSC functional test. |
| DOC-004 | WDT feature summary says maximum 8 s, but WDP table extends to 32 s on 32 kHz source. | Existing WDT library is preserved; do not change timing claims without measurement. | Timer measurement. |
| DOC-005 | PMCR protected-write prose mentions six cycles while a register paragraph mentions four. | Use the shortest safe sequence: unlock immediately followed by write with interrupts disabled. | Hardware stress test. |
| DOC-006 | VDTCR protected-write paragraph contains the same 6-vs-4-cycle inconsistency. | Immediate unlock/write, interrupts disabled. | Hardware stress test. |
| DOC-007 | TCKCSR F2XEN description says both `1=enable` and `1=disable` in adjacent lines. | Treat 1 as enable because surrounding text/status behavior describes 64 MHz activation. | Scope Timer0/1 high-speed output. |
| DOC-008 | Early core prose mentions vector relocation alignment differently from the dedicated interrupt chapter. | Require 512-byte alignment per dedicated IVBASE interrupt section. | Relocation firmware test. |
| DOC-009 | Some translated register descriptions contain naming typos (for example DTR1L called high byte in prose). | Follow address/register name and functional sentence, not typo label. | PWM dead-time scope test. |
| DOC-010 | TCCR3C register summary table places DOC32 at bit 2, while the detailed bit description (authoritative) places DOC3C/DOC32 at bit 1 with FOC3C at bit 0. | Header follows the detailed description: DOC32 = bit 1 (used by Timer3::protect() ch C). Summary table is wrong. | Compile + register check. |
| DOC-011 | PMCR narrative paragraph assigns OSCMEN to bit 2 and OSCKEN to bit 3, but the register summary table (authoritative layout) assigns OSCKEN = bit 3 / OSCMEN = bit 2. | Header follows the register summary table. Narrative is wrong. | Register check. |
| DOC-012 | MCUCR bit 5: header names it EXRFD (External Reset Force Disable, per LogicGreen spec) while the register summary table labels it SWR. The true SWR (software reset) is VDTCR bit 6. | Header keeps EXRFD at bit 5; SWR is only defined on VDTCR (bit 6). Summary-table label is a typo. | Register check. |
| DOC-013 | ADCSRB ACME01/ACME00/ACME11/ACME10 vs datasheet CME01/CME00/CME11/CME10 naming. Same bit positions 7:4. | Header uses AVR-convention ACMExx names (matching header ADCSRB convention); positions verified identical. | Register check. |
| DOC-014 | PRR1 register: datasheet bit 3 labelled PRTC3 (Timer3 power reduction); the logical name for that function is PRTIM3. | Header defines PRTIM3 (bit 3) with backward-compatible alias PRTC3 = PRTIM3. | Register check. |
| DOC-015 | CLKPR clock-output-enable bits: summary uses CLKOE1/CLKOE0 while detailed description uses CKOE1/CKOE0. | Header defines CKOE1/CKOE0 (bits 6/5) with backward-compatible aliases CLKOE1/CLKOE0/CLKOE. | Register check. |
| DOC-016 | TCKCSR bit 2 AFCKS (auto frequency check) present in datasheet v1.0.5 but was missing from the header; unused by any code path. | Added AFCKS = 2 to lgtx8p.h for full datasheet coverage. | Register check. |
| DOC-017 | EEPROM partition growth above 1KB overwrites the bootloader: wiki + programLimit() agree that 2KB EEPROM starts at 0x7000 which overlaps bootloader 0x7400-0x77FF. Only the 1KB default (0x7800) is bootloader-safe. | Recovery-safe clamps partition to 1KB (lgt_eeprom_init forces pages=1); change_size() above 1KB is a no-op under recovery-safe. | E2END + partition test. |

Add new entries whenever source implementation depends on ambiguous wording. Hardware behavior wins over prose after reproducible measurement.
