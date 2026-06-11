#!/bin/sh
#
# Enqpy(TM) reference build script.
# Compiles src/enqpy_reference_base_c1.c (the Base Cipher, the proof-complete
# profile) with both self-test and benchmark enabled, places the binary in
# ./build/enqpy_test, and runs it. To build the Extended Mixing Profile
# instead, substitute src/enqpy_reference.c below.
#
# Requires: any C11-capable compiler (gcc, clang, or equivalent).
# Usage:    ./build.sh
#
# For library-only builds, see README.md.

set -e

mkdir -p build

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--O3 -std=c11 -Wall}"

echo "Building Enqpy(TM) reference implementation (Base Cipher)..."
echo "  Compiler: $CC"
echo "  Flags:    $CFLAGS -DENQPY_SELFTEST -DENQPY_BENCHMARK"
echo ""

$CC $CFLAGS -DENQPY_SELFTEST -DENQPY_BENCHMARK \
    src/enqpy_reference_base_c1.c \
    -o build/enqpy_test

echo "Build complete: build/enqpy_test"
echo ""
echo "Running self-test and benchmark..."
echo "========================================================"
./build/enqpy_test
echo "========================================================"
echo ""
echo "For subsequent runs: ./build/enqpy_test"
