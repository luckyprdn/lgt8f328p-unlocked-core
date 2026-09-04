# Contributing

Thanks for considering a contribution. This repository documents hardware
behavior aggressively — please read `docs/datasheet-errata.md` before
submitting anything that touches silicon behavior.

## Ground rules

- **Hardware behavior wins over prose.** If you change behavior that depends
  on the die, either cite the DOC entry or add one. Unverifiable silicon
  claims do not get merged.
- **User-facing text is English.** No emoji, no marketing phrasing, no
  mixed-language comments in files users read.
- **Keep the example set compiling.** Every example must build on the three
  variants where the feature exists.

## Development workflow

1. Fork and create a branch.
2. Make the change. If it alters public API or examples, update the
   `Example_*` demos, `FeatureTour`, and `api-reference.md` in the same PR.
3. Compile check locally (Arduino CLI):
   ```bash
   arduino-cli compile -b lgt8funlocked:avr:328:variant=modelP  <sketch>
   arduino-cli compile -b lgt8funlocked:avr:328:variant=modelP48 <sketch>
   arduino-cli compile -b lgt8funlocked:avr:328:variant=modelD   <sketch>
   ```
4. If the change affects hardware behavior, run `silicon_verify` /
   `final_sweep` on your board and paste the summary into the PR.
5. Open the PR — CI compiles the full example set and runs the host tests.

## Hardware verification kit

- `silicon_verify` — core gates
- `final_sweep` — full byte-accurate sweep + benchmarks + WDT reset proof
- `danger_probe` — re-checks features flagged unreliable on LQFP48
- `HardwareSmokeSafePro` — production smoke test

## License

By contributing you agree that your changes are licensed under the same
terms as the file you modify (see `LICENSE` and `THIRD_PARTY_NOTICES.md`).
