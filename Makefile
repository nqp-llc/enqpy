# ============================================================================
# Enqpy(tm) Rev 4.0 -- reference core + deployable HKU wrapper
#
#   make            build the static library and both test binaries
#   make test       build + run the core 84/84 KAT and the wrapper self-test
#   make core-test  build + run the core's own self-test (84 PASS / 0 FAIL)
#   make hku-test   build + run the deployable wrapper self-test (8 passed)
#   make bench      build + run the core benchmark
#   make lib        build libenqpy_hku.a for linking into applications
#   make clean      remove build artifacts
#
# Layout assumed:  src/enqpy_reference.c  src/enqpy_hku.c  src/enqpy_hku.h
# ============================================================================

CC      ?= cc
AR      ?= ar
CFLAGS  ?= -O2 -std=c11 -Wall -Wextra
SRC     := src

# Core object for DEPLOYMENT: compiled WITHOUT ENQPY_SELFTEST/ENQPY_BENCHMARK,
# so it exposes no main() and exports PDAF_SEC / ENQPY_NIL_COMM_UPDATE / etc.
CORE_OBJ := enqpy_reference.o
# Wrapper object for LINKING into apps: compiled WITHOUT ENQPY_HKU_DEMO (no main()).
HKU_OBJ  := enqpy_hku.o
LIB      := libenqpy_hku.a

.PHONY: all test core-test hku-test bench lib clean help

all: lib core_selftest enqpy_hku_demo

# ---- objects ---------------------------------------------------------------
$(CORE_OBJ): $(SRC)/enqpy_reference.c
	$(CC) $(CFLAGS) -c $< -o $@

$(HKU_OBJ): $(SRC)/enqpy_hku.c $(SRC)/enqpy_hku.h
	$(CC) $(CFLAGS) -c $(SRC)/enqpy_hku.c -o $@

# ---- static library apps link against (with src/enqpy_hku.h) ---------------
lib: $(LIB)
$(LIB): $(CORE_OBJ) $(HKU_OBJ)
	$(AR) rcs $@ $(CORE_OBJ) $(HKU_OBJ)

# ---- core's own KAT binary (has main via -DENQPY_SELFTEST) ------------------
core_selftest: $(SRC)/enqpy_reference.c
	$(CC) $(CFLAGS) -DENQPY_SELFTEST $< -o $@

# ---- deployable wrapper self-test/demo (has main via -DENQPY_HKU_DEMO) ------
enqpy_hku_demo: $(SRC)/enqpy_hku.c $(SRC)/enqpy_hku.h $(CORE_OBJ)
	$(CC) $(CFLAGS) -DENQPY_HKU_DEMO $(SRC)/enqpy_hku.c $(CORE_OBJ) -o $@

# ---- core benchmark --------------------------------------------------------
enqpy_bench: $(SRC)/enqpy_reference.c
	$(CC) $(CFLAGS) -DENQPY_BENCHMARK $< -o $@

# ---- test targets (nonzero exit on failure -> CI-friendly) -----------------
core-test: core_selftest
	./core_selftest

hku-test: enqpy_hku_demo
	./enqpy_hku_demo

test: core-test hku-test
	@echo "== all Enqpy tests passed =="

bench: enqpy_bench
	./enqpy_bench

clean:
	rm -f $(CORE_OBJ) $(HKU_OBJ) $(LIB) core_selftest enqpy_hku_demo enqpy_bench

help:
	@echo "targets: all test core-test hku-test bench lib clean"
