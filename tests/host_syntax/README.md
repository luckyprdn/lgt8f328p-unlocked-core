# Host C++ syntax harness

This harness uses fake AVR registers and host `g++` to catch C++ API/header mistakes when the real AVR toolchain is unavailable.

```sh
tests/host_syntax/run.sh
```

It is intentionally only a **syntax gate**. It cannot validate AVR inline-assembly constraints/opcodes, register timing, binary size, generated instructions, ISR behavior, or hardware semantics. The real AVR compile matrix remains a v1.0 release gate.
