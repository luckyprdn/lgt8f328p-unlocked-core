#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CXX=${CXX:-g++}
FLAGS="-std=gnu++11 -Wall -Wextra -Werror -fsyntax-only -DLGT8_UNLOCKED_ALLOW_HIGH_DRIVE=1"
INCLUDES="-I$ROOT/tests/host_syntax -I$ROOT/cores/lgt8f -I$ROOT/libraries/LGT8Unlocked/src"

# Aggregate header/API syntax gate.
$CXX $FLAGS $INCLUDES "$ROOT/tests/host_syntax/test.cpp"

# Runtime-check the register-only helpers that do not execute AVR-specific
# inline assembly. Sanitizers catch accidental fake-register OOB/UB on host.
RUNTIME_BIN="$ROOT/tests/host_syntax/.runtime-test"
trap 'rm -f "$RUNTIME_BIN"' EXIT HUP INT TERM
$CXX -std=gnu++11 -Wall -Wextra -Werror -DLGT8_UNLOCKED_ALLOW_HIGH_DRIVE=1 -fsanitize=address,undefined -fno-omit-frame-pointer $INCLUDES "$ROOT/tests/host_syntax/runtime.cpp" -o "$RUNTIME_BIN"
"$RUNTIME_BIN"

# Every public example must parse against the same API surface.  This is not an
# AVR codegen test; inline AVR assembly is only parsed as C++ strings here.
count=0
for ino in "$ROOT"/libraries/LGT8Unlocked/examples/*/*.ino; do
  $CXX -x c++ $FLAGS $INCLUDES "$ino"
  count=$((count + 1))
done
printf 'PASS host C++ syntax/runtime: aggregate + register runtime + %d examples\n' "$count"
