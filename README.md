# Enqpy™ Stream Cipher — Reference Implementation

> **Enqpy™** (pronounced "En-Q-P") is a symmetric stream cipher formally proved
> to satisfy Shannon's Ideal System criterion. This repository is the canonical
> software reference implementation, released for evaluation alongside the
> formal proof paper.

**Paper (IACR ePrint):** *Enqpy™ Stream Cipher: Constructive Proof of Shannon's
Ideal System for a Finite-Key Cipher* — link forthcoming on publication.

**Project site:** [enqpy.com](https://enqpy.com)

**Acquisition inquiries:** [RPM@enqpy.com](mailto:RPM@enqpy.com)

---

## What this is

This repository contains:

- **`src/enqpy_reference.c`** — the C11 reference implementation of the Ideal
  Enqpy™ Configuration (Key Role Separation: nonce-only OffsetKey derivation).
  Single file, no external dependencies, portable from 8-bit microcontrollers
  to 64-bit servers.
- **78 embedded self-test assertions** covering PDAF Mode 0, PDAF Mode 1,
  PDAF_SEC round-trip, and the `[+8]` coset invariants that drive the Shannon
  Ideal System result.
- **Built-in benchmark harness** reproducing the performance numbers reported
  in the paper.

## What this is *not*

- **Not open source** in the OSI sense. The implementation is released under
  the **NQP LLC Evaluation License** (see [`LICENSE`](./LICENSE)): you may
  clone, build, test, and benchmark for evaluation purposes. Redistribution,
  sublicensing, or production deployment require a separate written agreement
  with NQP LLC.
- **Not production-ready** without the additional operational mechanisms
  described in §16 of the paper: integrity (a MAC over ciphertext and
  `eff_or`), nonce uniqueness infrastructure, secure key storage, constant-time
  implementation of the CS permutation lookup, and secure erase of key
  material after use. See the implementation comments in `src/enqpy_reference.c`
  for specifics.

---

## Quick verify (5-minute check)

```bash
# Clone the repo
git clone https://github.com/<USERNAME>/enqpy.git
cd enqpy

# Build with self-test + benchmark
./build.sh

# Run
./build/enqpy_test
```

Expected output begins with:

```
Enqpy(tm) Stream Cipher -- Reference Implementation Rev 2.0
Ideal Enqpy(tm) Configuration (nonce-only OffsetKey derivation)
Copyright (c) 2026 NQP LLC
Platform: n=64, tile_len=144, W_bytes=6144

Self-test: 78 PASS  0 FAIL
Self-test PASSED.
```

followed by benchmark tables. Total wall-clock time is well under a second on
any modern machine.

If you see `78 PASS  0 FAIL`, the implementation matches the test vectors in
the Formal Cryptographic Description (FCD §13) and the paper (§15) byte-exact.

---

## What the self-tests verify

The 78 assertions map directly onto claims made in the paper:

| Assertion group | Count | Verifies |
|---|---:|---|
| TV1 — PDAF Mode 0 (n=10, 30 nibbles) | 30 | Core PDAF primitive, additive mode — matches FCD §13.2 |
| TV2 — PDAF Mode 1 (n=10, 30 nibbles) | 30 | Core PDAF primitive, one-way gate mode — matches FCD §13.2 |
| TV3 — PDAF_SEC W-output (n=64, Ideal Config) | 8 | Full cipher output under Ideal Enqpy™ Configuration |
| TV3 RT — Round-trip decryption | 8 | W ⊕ (W ⊕ PT) = PT; confirms XOR-streaming correctness |
| TV4 — `[+8]` coset invariance on EK axis | 1 | EK+8·**1** produces identical ciphertext (Remark 1, Theorem 2) |
| TV5 — `[+8]` coset invariance on QK axis | 1 | QK+8·**1** produces identical ciphertext (Remark 1, Theorem 2) |
| **Total** | **78** | |

TV4 and TV5 are the empirical confirmation of the `[+8]` global shift invariant
that drives the 4-element ciphertext-equivalent coset
{EK, EK+8} × {QK, QK+8} — the algebraic foundation of the
H(EK, QK | T^∞) = log₂(4) = 2 bits exact equivocation result proved in the
paper (Theorem 2, Section 6.1; Appendix A, Layer 4 six-step proof).

See [`tests/README.md`](./tests/README.md) for a more detailed map of what
each assertion verifies and how to extend the test suite.

---

## Security profiles

The reference supports three mandatory key-length profiles per the FCD:

| Profile | Key length | PDAF output | Target deployment |
|---|---|---|---|
| LOW    | 32 nibbles (128 bits) | 1,024 nibbles | IoT, embedded |
| MEDIUM | 48 nibbles (192 bits) | 2,304 nibbles | Enterprise |
| **HIGH** | **64 nibbles (256 bits)** | **4,096 nibbles** | **Government, defense (default)** |

The paper's numerical bounds are all stated for the HIGH profile (n=64):
plaintext equivocation floor ≥ 2^128, key equivocation floor = 2 bits exact
under the Ideal Configuration.

---

## Building from source

### Requirements

- A C11-capable compiler (`gcc` ≥ 4.9, `clang` ≥ 3.3, MSVC ≥ 2015)
- POSIX-compatible shell for `build.sh`, or run the commands manually
- No external libraries; no `make`, `cmake`, or build system required

### One-line build (library only)

```bash
cc -O2 -std=c11 -c src/enqpy_reference.c -o enqpy.o
```

### With self-test

```bash
cc -O3 -std=c11 -DENQPY_SELFTEST src/enqpy_reference.c -o enqpy_test
./enqpy_test
```

### With benchmark

```bash
cc -O3 -std=c11 -DENQPY_BENCHMARK src/enqpy_reference.c -o enqpy_bench
./enqpy_bench
```

### With both (recommended for first check)

```bash
cc -O3 -std=c11 -DENQPY_SELFTEST -DENQPY_BENCHMARK src/enqpy_reference.c -o enqpy_all
./enqpy_all
```

The included `build.sh` does the combined build and places the binary in
`./build/enqpy_test`.

---

## Paper

See [`paper/README.md`](./paper/README.md) for the full citation and links.
The paper establishes, unconditionally and with no computational hardness
assumption, that Enqpy™ is the first finite-key cipher formally proved to
satisfy Shannon's Ideal System definition.

Principal results:

- **Theorem 1 (§5)** — PDAF Mode 1 inversion lower bound: |P(O*)| ≥ 16ⁿ.
- **Theorem 2 (§6)** — Shannon Ideal System property:
  H(EK, QK | T^∞) = log₂(4) = 2 bits exact for the Ideal Configuration;
  T_{>t} ⊥ (EK, QK) | T_{≤t}.
- **Corollary 2 (§7)** — All bounds invariant under quantum computation
  (no computational hardness assumption invoked in the proof).
- **Theorem 3 (§12)** — Plaintext equivocation:
  H(PT | CT, OR) ≥ 128 bits for HIGH profile; at least 2^128 consistent
  plaintexts for any ciphertext.

---

## Acquisition and licensing

NQP LLC is actively engaged in commercialization discussions. For evaluation
requests, technical discussions, or acquisition inquiries:

**[RPM@enqpy.com](mailto:RPM@enqpy.com)**

The reference implementation under this repository is released solely for
evaluation under the [NQP LLC Evaluation License](./LICENSE). Production
licensing, FPGA/VHDL reference, extended test vector suites, and commercial
support terms are available through direct inquiry.

---

## Citation

A `CITATION.cff` file in the repository root enables GitHub's native citation
export. Preferred citation format once the ePrint paper is published:

```bibtex
@techreport{mcgough2026enqpy,
  author      = {Paul McGough},
  title       = {{E}nqpy\texttrademark{} Stream Cipher: Constructive Proof of
                 Shannon's Ideal System for a Finite-Key Cipher},
  institution = {NQP LLC},
  year        = {2026},
  number      = {Rev 1.0},
  note        = {IACR ePrint entry forthcoming}
}
```

---

## Repository status

| Artifact | Status |
|---|---|
| C reference implementation | ✅ Rev 2.0 (Ideal Configuration) |
| Test vectors (78 assertions) | ✅ 78/78 PASS |
| Benchmark harness | ✅ Included |
| IACR ePrint paper | 🟡 Pre-publication |
| VHDL reference | 🔒 Available via direct licensing |
| NIST SP 800-22 full test suite | 🟡 Summary in FCD §14; full run available via direct inquiry |

---

*Copyright © 2026 NQP LLC. All rights reserved.*
*Enqpy™ and EnqpyADS™ are trademarks of NQP LLC.*
