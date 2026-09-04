# Security policy

This project runs on microcontrollers; report security issues the same way as
for any other embedded project.

## Reporting a vulnerability

Please do **not** open a public issue for a vulnerability. Report privately:

- GitHub private vulnerability reporting (preferred, if enabled on this repo),
  or
- email the maintainer (see the repository owner profile) with subject
  prefix `[lgt8f328p-unlocked-core security]`.

Include: affected release/commit, silicon package if relevant, a minimal
sketch or register sequence that reproduces the issue, and impact.

## Scope

- Core bootloader / IAP boundaries and the recovery-safe gates
  (`docs/datasheet-errata.md`, DOC-017/018)
- Library APIs with memory-safety or interrupt-context implications
- Anything that could brick a board or bypass the recovery-safe locks

Out of scope: documented silicon behavior (the errata log), electrical
damage, physical attacks.
