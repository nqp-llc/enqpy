# Enqpy™ Stream Cipher — Reference Implementation

> **Enqpy™** (pronounced "En-Q-P") is a symmetric stream cipher formally proved
> to satisfy Shannon's Ideal System criterion. This repository is the canonical
> software reference implementation, released alongside the formal proof paper.

**Paper (IACR ePrint):** *Enqpy™ Stream Cipher: Constructive Proof of Shannon's
Ideal System for a Finite-Key Cipher* — [link will be inserted on June 1, 2026
publication; until then, paper is hosted directly on enqpy.com].

**Project site:** [enqpy.com](https://enqpy.com)

**Inquiries:** [RPM@enqpy.com](mailto:RPM@enqpy.com) — see
[enqpy.com/use.html](https://enqpy.com/use.html) for the four commercial paths.

---

## Stewardship and licensing

The Enqpy™ reference implementation is stewarded by the **Enqpy™ Foundation
Inc.** (a Delaware nonstock nonprofit corporation, formed April 2026), which
owns the Enqpy™ and EnqpyADS™ trademarks. Commercial relationships are
operated by **NQP LLC** (the inventor's Virginia commercial entity,
incorporated March 19, 2026), under a perpetual non-exclusive trademark
license from the Foundation. The Foundation makes the work durable; NQP makes
it commercially serviceable.
See [enqpy.com/foundation.html](https://enqpy.com/foundation.html).

**Free use** for research, education, individual developers, small entities
(under 25 employees and under $1M annual revenue), and public-interest
projects is granted by the Public Patent Grant in the [`LICENSE`](./LICENSE)
file — no fee, no signup, no NDA.

**Four commercial paths** are available for use beyond Free-Tier criteria:

- **NQP Standard Commercial License** — clean annual patent license for
  above-threshold organizations ($5K–$75K range, scaled by organization size
  and deployment scope)
- **NQP commercial services** — architecture review, audit, training, custom
  implementation, advisory
- **Partner Program** — five categories (Enqpy-Certified Product, Enqpy Inside
  Trademark, Custom Implementation, Premier Technical, Hardware Integration)
- **Foundation conformance certification** — for third-party implementations
  seeking the Enqpy™-Certified mark

See [enqpy.com/use.html](https://enqpy.com/use.html) for the framework, or
email RPM@enqpy.com with `[Commercial]` in the subject line.

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
  the **NQP LLC Public License for Enqpy™** (see [`LICENSE`](./LICENSE)),
  which contains an Evaluation License covering source-code rights for all
  users plus a Public Patent Grant covering free use of the cipher for the
  Free-Tier audience above.
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
git clone https://github.com/nqp-llc/enqpy.git
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

A companion technical note, [*Adversarial Outcome Equivalence — proved under
the DSMV*](https://enqpy.com/papers/enqpy-dsmv.pdf), proves that Enqpy™ and
the One-Time Pad share the same operational secrecy outcome under a binary
success metric, with a structural observation that Enqpy™ maintains two
algebraically independent uncertainty axes (the OTP maintains only one).

---

## Repository governance

This repository follows community-profile conventions for governance,
contribution, and security disclosure:

- [`LICENSE`](./LICENSE) — NQP LLC Public License for Enqpy™
  (Evaluation License + Public Patent Grant for Free-Tier Use)
- [`CODE_OF_CONDUCT.md`](./CODE_OF_CONDUCT.md) — Code of Conduct and
  Acceptable Use Policy
- [`CONTRIBUTING.md`](./CONTRIBUTING.md) — How to contribute (accepted
  scope, DCO sign-off, review process)
- [`SECURITY.md`](./SECURITY.md) — Vulnerability disclosure and
  coordinated-disclosure policy

This is an inventor-stewarded reference implementation, not a community-
developed project. Contributions to documentation, build, portability,
and test infrastructure are welcome; modifications to the core
cryptographic algorithm are not — the implementation must remain in exact
correspondence with the formal proof.

---

## Citation

A `CITATION.cff` file in the repository root enables GitHub's native citation
export. Preferred citation format:

```bibtex
@techreport{mcgough2026enqpy,
  author      = {Paul McGough},
  title       = {{E}nqpy\texttrademark{} Stream Cipher: Constructive Proof of
                 Shannon's Ideal System for a Finite-Key Cipher},
  institution = {NQP LLC},
  year        = {2026},
  number      = {Rev 1.0},
  note        = {IACR ePrint entry: link inserted June 1, 2026}
}
```

---

## Repository status

| Artifact | Status |
|---|---|
| C reference implementation | ✅ Rev 2.0 (Ideal Configuration) |
| Test vectors (78 assertions) | ✅ 78/78 PASS |
| Benchmark harness | ✅ Included |
| IACR ePrint paper | ✅ June 1, 2026 (link inserted on publication) |
| Repository governance (LICENSE, COC, CONTRIBUTING, SECURITY) | ✅ Effective June 1, 2026 |
| VHDL reference | 🔒 Available via direct licensing |
| NIST SP 800-22 full test suite | 🟡 Summary in FCD §14; full run available via direct inquiry |

---

*Copyright © 2026 NQP LLC. All rights reserved.*
*Enqpy™, EnqpyADS™, Enqpy™-Certified, and Security, Settled.™ are trademarks
of Enqpy™ Foundation Inc., licensed to NQP LLC.*
