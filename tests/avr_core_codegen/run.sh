#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CLANG=${CLANG:-clang}
OBJDUMP=${OBJDUMP:-/usr/local/swift/usr/bin/llvm-objdump}
OUT=${TMPDIR:-/tmp}/lgt8-core-codegen
rm -rf "$OUT"; mkdir -p "$OUT"
trap 'rm -rf "$OUT"' EXIT HUP INT TERM

COMMON_CFLAGS="--target=avr -mmcu=atmega328p -std=gnu11 -Os -Wall -Wextra -Werror -ffreestanding -DF_CPU=32000000UL -DLGT8_UNLOCKED_RECOVERY_SAFE=1"
INCLUDES="-I$ROOT/tests/avr_core_codegen/stubs -I$ROOT/cores/lgt8f -I$ROOT/variants/lgt8fx8p48"

# Compile a probe against the actual core Arduino.h and actual QFP48 variant.
# This validates the optimized macros through a real AVR backend rather than a
# host-only surrogate.
# shellcheck disable=SC2086
$CLANG $COMMON_CFLAGS $INCLUDES -c "$ROOT/tests/avr_core_codegen/probe.c" -o "$OUT/probe.o"
$OBJDUMP -dr "$OUT/probe.o" > "$OUT/probe.dis"

# D13 is PB5. A constant digitalWrite must collapse to SBI PORTB,5 and a
# constant toggle to one write to PINB. No digitalWrite relocation is allowed
# inside constant_gpio_probe.
awk '/<constant_gpio_probe>:/,/^$/' "$OUT/probe.dis" > "$OUT/constant.dis"
grep -Eq '[[:space:]]sbi[[:space:]]+0x5, 0x5' "$OUT/constant.dis"
grep -Eq '[[:space:]]out[[:space:]]+0x3,' "$OUT/constant.dis"
! grep -q 'R_AVR_CALL.*digitalWrite' "$OUT/constant.dis"

# PWM constants and runtime pins must keep the compatibility function call so
# PWM disconnection and dynamic lookup semantics are preserved.
awk '/<pwm_semantics_probe>:/,/^$/' "$OUT/probe.dis" | grep -q 'R_AVR_CALL.*digitalWrite'
awk '/<runtime_gpio_probe>:/,/^$/' "$OUT/probe.dis" | grep -q 'R_AVR_CALL.*digitalWrite'

# Compile the two modified legacy C translation units with the actual core and
# variant headers.  The lightweight avr-libc headers in stubs/ exist only
# because this environment lacks avr-libc; register definitions from lgtx8p.h
# and the production source files remain real.
# shellcheck disable=SC2086
$CLANG $COMMON_CFLAGS $INCLUDES -c "$ROOT/cores/lgt8f/wiring_digital.c" -o "$OUT/wiring_digital.o"
# shellcheck disable=SC2086
$CLANG $COMMON_CFLAGS $INCLUDES -c "$ROOT/cores/lgt8f/wiring_analog.c" -o "$OUT/wiring_analog.o"

# Force ARDUINO_MAIN so all production pin tables are instantiated and their
# compile-time size assertions fire for the 40-pin QFP48 variant.
# shellcheck disable=SC2086
$CLANG $COMMON_CFLAGS $INCLUDES -DARDUINO_MAIN -c "$ROOT/tests/avr_core_codegen/pin_tables.c" -o "$OUT/pin_tables.o"

# Production TWI low-level C source against the real core/variant headers.
# compat/twi.h is a constants-only avr-libc surrogate in stubs/.
# shellcheck disable=SC2086
$CLANG $COMMON_CFLAGS $INCLUDES -I"$ROOT/libraries/Wire" -I"$ROOT/libraries/Wire/utility" \
  -c "$ROOT/libraries/Wire/utility/twi.c" -o "$OUT/twi.o"

# Production Wire C++ source. Stream.h is a tiny ABI-compatible compile stub;
# the Wire implementation and twi declarations themselves are production code.
$CLANG --target=avr -mmcu=atmega328p -x c++ -std=gnu++11 -Os -Wall -Wextra -Werror -ffreestanding \
  -DF_CPU=32000000UL $INCLUDES -I"$ROOT/libraries/Wire" -I"$ROOT/libraries/Wire/utility" \
  -c "$ROOT/libraries/Wire/Wire.cpp" -o "$OUT/Wire.o"

# Production EEPROM source with the real Arduino core header. Clang diagnoses a
# legacy Arduino EERef implicit-copy pattern that avr-g++ historically accepts;
# suppress only that unrelated compatibility warning, keeping all other
# warnings fatal.
$CLANG --target=avr -mmcu=atmega328p -x c++ -std=gnu++11 -Os -Wall -Wextra -Werror \
  -Wno-deprecated-copy-with-user-provided-copy -ffreestanding -DF_CPU=32000000UL \
  $INCLUDES -I"$ROOT/libraries/E2PROM" -c "$ROOT/libraries/E2PROM/EEPROM.cpp" -o "$OUT/EEPROM.o"

# RAM profile compile contracts.  These are board-menu-facing macros, so test
# all public profiles plus the historical ArduinoISP RX=250 override path.
for spec in '64 64 standard' '32 32 lean' '16 16 tiny'; do
  set -- $spec; rx=$1; tx=$2; name=$3
  $CLANG --target=avr -mmcu=atmega328p -x c++ -std=gnu++11 -Os -Wall -Wextra -Werror -ffreestanding \
    -I"$ROOT/tests/avr_core_codegen/stubs" -I"$ROOT/cores/lgt8f" \
    -DLGT_SERIAL_RX_BUFFER_SIZE=$rx -DLGT_SERIAL_TX_BUFFER_SIZE=$tx \
    -DEXPECT_RX=$rx -DEXPECT_TX=$tx \
    -c "$ROOT/tests/avr_core_codegen/serial_profile.cpp" -o "$OUT/serial_$name.o"
done
$CLANG --target=avr -mmcu=atmega328p -x c++ -std=gnu++11 -Os -Wall -Wextra -Werror -ffreestanding \
  -I"$ROOT/tests/avr_core_codegen/stubs" -I"$ROOT/cores/lgt8f" \
  -DSERIAL_RX_BUFFER_SIZE=250 -DRAMSTART=0x100 -DRAMEND=0x8ff -DEXPECT_RX=250 -DEXPECT_TX=64 \
  -c "$ROOT/tests/avr_core_codegen/serial_profile.cpp" -o "$OUT/serial_isp.o"

printf 'PASS actual core AVR codegen: D13 fast path + legacy wiring/Wire/TWI/EEPROM + RAM profiles + QFP48 table assertions\n'
