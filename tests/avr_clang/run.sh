#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CLANGXX=${CLANGXX:-clang++}
OBJDUMP=${OBJDUMP:-/usr/local/swift/usr/bin/llvm-objdump}
OUT=${TMPDIR:-/tmp}/lgt8-unlocked-avr-clang
rm -rf "$OUT"
mkdir -p "$OUT/examples"
trap 'rm -rf "$OUT"' EXIT HUP INT TERM

COMMON="--target=avr -mmcu=atmega328p -std=gnu++11 -Os -Wall -Wextra -Werror -ffreestanding -fno-exceptions -fno-rtti"
INCLUDES="-I$ROOT/tests/host_syntax -I$ROOT/tests/host_syntax/avr -I$ROOT/cores/lgt8f -I$ROOT/libraries/LGT8Unlocked/src -I$ROOT/libraries/SPI"

# This uses Clang's real AVR backend, so register-width/inline-assembly syntax
# gets lowered to AVR machine code rather than merely parsed by a host C++
# compiler. It is still not a replacement for the LogicGreen avr-gcc package:
# the LGT core has ISA/register extensions that stock AVR tools do not know.
$CLANGXX $COMMON $INCLUDES -c "$ROOT/tests/host_syntax/test.cpp" -o "$OUT/aggregate.o"

count=0
for ino in "$ROOT"/libraries/LGT8Unlocked/examples/*/*.ino; do
  name=$(basename "$(dirname "$ino")")
  $CLANGXX --target=avr -mmcu=atmega328p -x c++ -std=gnu++11 -Os -Wall -Wextra -Werror -ffreestanding -fno-exceptions -fno-rtti $INCLUDES -c "$ino" -o "$OUT/examples/$name.o"
  count=$((count + 1))
done

# The uDSC implementation intentionally contains explicit AVR IN/OUT. Confirm
# that the AVR backend really emitted those instructions. Their custom LGT
# 16-bit semantics still require the real LogicGreen assembler/hardware test.
$OBJDUMP -d "$OUT/examples/uDSC.o" > "$OUT/udsc.dis"
grep -Eq '[[:space:]]out[[:space:]]' "$OUT/udsc.dis"
grep -Eq '[[:space:]]in[[:space:]]' "$OUT/udsc.dis"

printf 'PASS AVR-Clang codegen: aggregate + %d examples; uDSC IN/OUT emitted\n' "$count"
