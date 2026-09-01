# AVR-Clang qualification gate

`run.sh` compiles the complete LGT8Unlocked public header surface and every
example with Clang 17's AVR backend (`atmega328p`) under `-Wall -Wextra -Werror`.
It also disassembles the uDSC example and checks that AVR `IN`/`OUT` operations
were actually emitted.

This is stronger than a host syntax check, but intentionally **does not claim**
compatibility with the LogicGreen-custom avr-gcc/assembler or validate the
LGT8XM 16-bit `IN/OUT` extension. Those remain real-toolchain/hardware gates.
