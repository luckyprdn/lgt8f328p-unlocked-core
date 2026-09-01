#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CXX=${CXX:-g++}
INC="-I$ROOT/tests/host_syntax -I$ROOT/cores/lgt8f -I$ROOT/variants/lgt8fx8p48"
FLAGS="-std=gnu++11 -Wall -Wextra -Werror -DLGT8_UNLOCKED_RECOVERY_SAFE=1 -DLGT8_UNLOCKED_ALLOW_HIGH_DRIVE=1"

$CXX $FLAGS -fsyntax-only $INC -I$ROOT/libraries/Wire -I$ROOT/libraries/Wire/utility "$ROOT/libraries/Wire/Wire.cpp"
$CXX $FLAGS -fsyntax-only $INC -I$ROOT/libraries/E2PROM "$ROOT/libraries/E2PROM/EEPROM.cpp"
EEPROM_BIN="$ROOT/tests/host_syntax/.eeprom-runtime-test"
BIN="$ROOT/tests/host_syntax/.wire-runtime-test"
trap 'rm -f "$BIN" "$EEPROM_BIN"' EXIT HUP INT TERM
$CXX $FLAGS $INC -I$ROOT/libraries/E2PROM \
  "$ROOT/tests/host_syntax/eeprom_runtime.cpp" "$ROOT/libraries/E2PROM/EEPROM.cpp" -o "$EEPROM_BIN"
"$EEPROM_BIN"
# twi.c is also parsed as C++ here because the lightweight host Arduino stub is
# shared with the C++ tests. This gate catches declarations/bounds edits; the
# real Arduino/AVR C compiler remains a separate qualification gate.
$CXX -x c++ $FLAGS -fsyntax-only $INC -I$ROOT/libraries/Wire/utility "$ROOT/libraries/Wire/utility/twi.c"

$CXX $FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer $INC -I$ROOT/libraries/Wire -I$ROOT/libraries/Wire/utility \
  "$ROOT/tests/host_syntax/wire_runtime.cpp" "$ROOT/libraries/Wire/Wire.cpp" -o "$BIN"
"$BIN"
printf 'PASS legacy syntax/runtime: Wire.cpp + EEPROM.cpp + twi.c + Wire + EEPROM safe-partition runtime\n'
