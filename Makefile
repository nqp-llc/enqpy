# ============================================================================
# Enqpy(tm) Rev 5.1 -- reference core
#
#   make            build + run the core 84/84 KAT self-test
#   make test       build + run the core's self-test (84 PASS / 0 FAIL)
#   make vectors    verify every published vector against the reference (34)
#   make check      test + vectors -- what CI runs
#   make bench      build + run the core benchmark
#   make aead       build + run the cross-cipher AEAD benchmark
#   make clean      remove build artifacts
#
# Layout assumed:  src/enqpy_reference.c        src/aead_bench.c
#                  tests/vectors/enqpy-vectors.json
#                  tests/vectors/vectors_check.c
# ============================================================================

CC      ?= cc
CFLAGS  ?= -O2 -std=c11 -Wall -Wextra
SRC     := src
VECDIR  := tests/vectors

.PHONY: all test vectors check bench aead clean help

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

# ---- vector verifier (includes the core; -I points at it) ------------------
vectors_check: $(VECDIR)/vectors_check.c $(SRC)/enqpy_reference.c
	$(CC) $(CFLAGS) -I $(SRC) $(VECDIR)/vectors_check.c -o $@

# ---- test targets (nonzero exit on failure -> CI-friendly) -----------------
test: core_selftest
	./core_selftest

# Verifies the reference against the PUBLISHED vectors -- all three profiles,
# the Rev 5.1 key domain, the cosets, empty plaintext, the window boundary,
# the Phase-1 OR_EXP expansion and the NIL key-update policy.
vectors: vectors_check
	./vectors_check

# What CI runs: the code agrees with itself, and with the published data.
check: test vectors

bench: enqpy_bench
	./enqpy_bench

aead: aead_bench
	./aead_bench

clean:
	rm -f core_selftest enqpy_bench aead_bench vectors_check

help:
	@echo "targets: all test vectors check bench aead clean"
