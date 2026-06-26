# ============================================================================
# Enqpy(tm) Rev 5.0 -- reference core
#
#   make            build + run the core 84/84 KAT self-test
#   make test       build + run the core's self-test (84 PASS / 0 FAIL)
#   make bench      build + run the core benchmark
#   make aead       build + run the cross-cipher AEAD benchmark
#   make clean      remove build artifacts
#
# Layout assumed:  src/enqpy_reference.c  src/aead_bench.c
# ============================================================================

CC      ?= cc
CFLAGS  ?= -O2 -std=c11 -Wall -Wextra
SRC     := src

.PHONY: all test bench aead clean help

all: test

# ---- core's own KAT binary (main via -DENQPY_SELFTEST) ----------------------
core_selftest: $(SRC)/enqpy_reference.c
	$(CC) $(CFLAGS) -DENQPY_SELFTEST $< -o $@

# ---- core benchmark --------------------------------------------------------
enqpy_bench: $(SRC)/enqpy_reference.c
	$(CC) $(CFLAGS) -DENQPY_BENCHMARK $< -o $@

# ---- cross-cipher AEAD benchmark (links the core; no SELFTEST/BENCHMARK) ----
aead_bench: $(SRC)/aead_bench.c $(SRC)/enqpy_reference.c
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L $(SRC)/aead_bench.c $(SRC)/enqpy_reference.c -o $@

# ---- test targets (nonzero exit on failure -> CI-friendly) -----------------
test: core_selftest
	./core_selftest

bench: enqpy_bench
	./enqpy_bench

aead: aead_bench
	./aead_bench

clean:
	rm -f core_selftest enqpy_bench aead_bench

help:
	@echo "targets: all test bench aead clean"
