# Enqpy™ Stream Cipher — Reference Implementation

[![build](https://github.com/nqp-llc/enqpy/actions/workflows/ci.yml/badge.svg)](https://github.com/nqp-llc/enqpy/actions/workflows/ci.yml)

> **Enqpy™** (pronounced "En-Q-P") is a symmetric stream cipher formally proved
> to satisfy Shannon's Ideal System criterion. This repository is the canonical
> software reference implementation, released alongside the formal proof paper.

**Paper:** *Enqpy™ Stream Cipher: Constructive Proof of Shannon's
Ideal System for a Finite-Key Cipher.* The canonical citation and the current
paper link are maintained on the website — see
[enqpy.com/technical.html](https://enqpy.com/technical.html).

**Project site:** [enqpy.com](https://enqpy.com)

**Inquiries:** [RPM@enqpy.com](mailto:RPM@enqpy.com) — see
[enqpy.com/use.html](https://enqpy.com/use.html) for the four commercial paths.

---

## Quick verify

Clone, build, and confirm the implementation in a few minutes — no signup,
no permission.

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

followed by the benchmark tables. Total wall-clock time is well under a
second on any modern machine.

If you see `78 PASS  0 FAIL`, the implementation matches the test vectors in
the [Formal Cryptographic Description](./FCD.md) (FCD §13) and the paper (§15) byte-exact
— including the round-trip assertions (TV3 RT) that encrypt and decrypt end
to end.

**Benchmark figures are portable C, no hardware acceleration**, on a
conventional desktop CPU — a like-for-like portable-C comparison. See
[enqpy.com/speed_bench.html](https://enqpy.com/speed_bench.html) for full
methodology, including how hardware-accelerated AES compares.

### What a clean run shows — and what it doesn't

A clean run shows the **artifact** is real, builds with no dependencies, and
reproduces the published test vectors (including end-to-end encrypt/decrypt
round-trips). It does **not**, by itself, establish the **proof** — that lives
in the paper and is for cryptographic review. Two different trust acts: run the
code here; check the math there.

---

## Stewardship and licensing

The Enqpy™ reference implementation is stewarded by the **Enqpy™ Foundation
Inc.** (a Delaware nonstock nonprofit corporation, formed April 2026), which
owns the Enqpy™ and EnqpyADS™ trademarks. Commercial relationships are
operated by **NQP LLC** (the inventor's Virginia commercial entity,
incorporated March 19, 2026), under a perpetual, non-exclusive license from
the Foundation (the Foundation–NQP IP License Agreement). The Foundation makes
the work durable; NQP makes it commercially serviceable.
See [enqpy.com/foundation.html](https://enqpy.com/foundation.html).

**Patent-safe for everyone.** Under the **Open-Infrastructure Patent
Non-Assertion Covenant** ([enqpy.com/covenant](https://enqpy.com/covenant),
and in this repository), NQP LLC irrevocably commits not to assert the NQP
Patents against any conforming implementation of the Enqpy™ cipher — by
anyone, for any purpose, at any scale, including commercial deployment. No
fee, no signup, no NDA, and no license is required to use, implement, or
deploy the cipher itself. Source-code rights in NQP's reference
implementation are covered separately by the Evaluation License in the
[`LICENSE`](./LICENSE) file.

**Commercial relationships (optional — the earned edge).** None of these is a
condition of using the cipher; they fund the work and provide assurance for
organizations that want them:

- **Foundation conformance certification** — for third-party implementations
  seeking the Enqpy™-Certified mark and registry listing
- **NQP high-performance proprietary implementations** — the optimized
  VHDL/FPGA pipeline and related materials, licensed separately as NQP
  trade-secret/copyright works (not the public reference, not reached by the
  Covenant)
- **NQP commercial services** — architecture review, audit, training, custom
  implementation, integration, advisory, support, SLAs, indemnity
- **Partner Program** — five categories (Enqpy-Certified Product, Enqpy Inside
  Trademark, Custom Implementation, Premier Technical, Hardware Integration)
- **Optional signed patent license and indemnity** — for organizations that
  prefer executed paper notwithstanding the Covenant's protection of their use

See [enqpy.com/use.html](https://enqpy.com/use.html) for the framework, or
email RPM@enqpy.com with `[Commercial]` in the subject line.

---

## What this is

This repository contains:

- **`src/enqpy_reference.c`** — the C11 reference implementation of the Ideal
  Enqpy™ Configuration (Key Role Separation: nonce-only OffsetKey derivation).
  Single file, no external dependencies, portable from 8-bit microcontrollers
  to 64-bit servers.
- **[`FCD.md`](./FCD.md)** — the Formal Cryptographic Description: the
  prose-and-mathematics specification of the cipher (OWC, PDAF, PDAF_SEC, the
  five phases, key management, security analysis, and the §13 test vectors the
  self-tests check against). The authoritative statement of *what* the cipher
  does; the C reference shows *how* one implementation does it.
- **78 embedded self-test assertions** covering PDAF Mode 0, PDAF Mode 1,
  PDAF_SEC round-trip, and the `[+8]` coset invariants that drive the Shannon
  Ideal System result.
- **Built-in benchmark harness** reproducing the performance numbers reported
  in the paper.
- **`src/aead_bench.c`** — a separate cross-cipher AEAD benchmark
  (Enqpy™ + Poly1305 vs ChaCha20-Poly1305, AES-256-CTR, AES-256-GCM),
  KAT-verified before timing; the apples-to-apples comparison behind the
  Performance page.

## What this is *not*

- **Not open source** in the OSI sense. The implementation is released under
  the **NQP LLC Public License for Enqpy™** (see [`LICENSE`](./LICENSE)),
  which contains an Evaluation License covering source-code rights for all
  users. Patent rights in the cipher are addressed separately, and more
  broadly, by the [Patent Non-Assertion Covenant](https://enqpy.com/covenant),
  under which the cipher is patent-safe for all conforming use.
- **Not production-ready** without the additional operational mechanisms
  described in §16 of the paper: integrity (a MAC over ciphertext and
  `eff_or`), nonce uniqueness infrastructure, secure key storage, constant-time
  implementation of the CS permutation lookup, and secure erase of key
  material after use. See the implementation comments in `src/enqpy_reference.c`
  for specifics.

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

### Cross-cipher benchmark (apples-to-apples AEAD)

`src/aead_bench.c` is a separate harness that compares **Enqpy™ + Poly1305**
against **ChaCha20-Poly1305**, **AES-256-CTR**, and **AES-256-GCM** on
identical terms — every cipher authenticated, and every implementation
verified against its known-answer test vectors before any timing. It is the
apples-to-apples comparison behind
[enqpy.com/speed_bench.html](https://enqpy.com/speed_bench.html). The other
ciphers are implemented inline (no external libraries); only the `PDAF_SEC`
primitive is linked from the reference, so build the two files together. Do
**not** pass `-DENQPY_SELFTEST` / `-DENQPY_BENCHMARK` here — those enable the
reference's own `main` and would collide with the harness's.

```bash
cc -O3 -march=native -std=c11 -D_POSIX_C_SOURCE=200809L \
    src/aead_bench.c src/enqpy_reference.c -o aead_bench
./aead_bench
```

Software ranking is CPU-dependent — run it on your own hardware. (`-march=native`
is optional; it lets the compiler use your CPU's full instruction set.)

---

## Ports & bindings

Enqpy™ is meant to be used, in any language or runtime. Ports and bindings are
welcome — you don't need permission to write one.

- **Canonical test vectors:** [`tests/vectors/enqpy-vectors.json`](./tests/vectors/enqpy-vectors.json),
  documented in [`TEST_VECTORS.md`](./TEST_VECTORS.md) — the bar your port must
  reproduce exactly (UPPERCASE hex). Start with the zero-plaintext vector, where
  the ciphertext equals the keystream and isolates W generation from the XOR step.
- **How to port, verify, and get listed:** [`PORTING.md`](./PORTING.md)
- **Existing ports:** [`PORTS.md`](./PORTS.md)

A port that reproduces the vectors is **Reference-Compatible** by
self-attestation — give it its own name and describe it as *"Reference-Compatible
with the Enqpy™ reference implementation"* (the mark can't sit in a project name
like "Enqpy-Rust" or "PyEnqpy"). Use of the cipher in a port is patent-safe
under the [Covenant](https://enqpy.com/covenant) on the same terms as the
reference — no fee or license required for any conforming use (source-code
rights in NQP's reference are separate; see [`LICENSE`](./LICENSE)). Putting
the **Enqpy™** mark in a product name,
and **Enqpy™-Certified** or **Enqpy™-Compatible** claims, require Foundation
conformance certification — see [`CONFORMANCE.md`](./CONFORMANCE.md) for the three
levels.

---

## Paper

See [enqpy.com/technical.html](https://enqpy.com/technical.html) for the
canonical citation and the current paper link.
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
  (Evaluation License + Patent Non-Assertion Covenant summary)
- **Patent Non-Assertion Covenant** —
  [enqpy.com/covenant](https://enqpy.com/covenant) (and in this repository);
  the controlling instrument making the cipher patent-safe for all conforming use
- [`CODE_OF_CONDUCT.md`](./CODE_OF_CONDUCT.md) — Code of Conduct and
  Acceptable Use Policy
- [`CONTRIBUTING.md`](./CONTRIBUTING.md) — How to contribute (accepted
  scope, DCO sign-off, review process)
- [`SECURITY.md`](./SECURITY.md) — Vulnerability disclosure and
  coordinated-disclosure policy

Conformance and porting:

- [`CONFORMANCE.md`](./CONFORMANCE.md) — conformance levels (Reference-Compatible,
  Conformant Implementation, Enqpy™-Certified), canonical sources, and the
  submission process, stewarded by the Enqpy™ Foundation
- [`PORTING.md`](./PORTING.md) — how to port Enqpy™ to another language, verify
  against the vectors, and self-attest Reference-Compatible
- [`PORTS.md`](./PORTS.md) — registry of community ports and bindings
- [`TEST_VECTORS.md`](./TEST_VECTORS.md) — the canonical test-vector format and
  the verification procedure

This is an inventor-stewarded reference implementation, not a community-
developed project. Contributions to documentation, build, portability,
and test infrastructure are welcome; modifications to the core
cryptographic algorithm are not — the implementation must remain in exact
correspondence with the formal proof. Independent ports and bindings in other
languages are welcome and tracked in [`PORTS.md`](./PORTS.md) — see
[`PORTING.md`](./PORTING.md) to add one.

---

## Citation

The canonical citation and the current paper link are maintained in one
place — [enqpy.com/technical.html](https://enqpy.com/technical.html). A
`CITATION.cff` file in the repository root also enables GitHub's native
citation export. BibTeX form:

```bibtex
@techreport{mcgough2026enqpy,
  author      = {Paul McGough},
  title       = {{E}nqpy\texttrademark{} Stream Cipher: Constructive Proof of
                 Shannon's Ideal System for a Finite-Key Cipher},
  institution = {NQP LLC},
  year        = {2026},
  number      = {Rev 1.0},
  note        = {Canonical citation and paper link: https://enqpy.com/technical.html}
}
```

---

## Repository status

| Artifact | Status |
|---|---|
| C reference implementation | ✅ Rev 2.0 (Ideal Configuration) |
| Formal Cryptographic Description | ✅ `FCD.md` |
| Test vectors (78 assertions) | ✅ 78/78 PASS |
| Benchmark harness | ✅ Included |
| Formal proof paper | ✅ June 1, 2026 (canonical link on enqpy.com/technical.html) |
| Repository governance (LICENSE, COC, CONTRIBUTING, SECURITY) | ✅ Effective June 1, 2026 |
| Conformance specification | ✅ `CONFORMANCE.md` — Rev 0.1 framework (Rev 1.0 Months 4–6) |
| Porting guide & vector format | ✅ `PORTING.md`, `TEST_VECTORS.md` |
| Community ports registry | ✅ `PORTS.md` — open for submissions |
| VHDL reference | 🔒 Available via direct licensing |
| NIST SP 800-22 full test suite | 🟡 Summary in FCD §14; full run available via direct inquiry |

---

*Copyright © 2026 NQP LLC. All rights reserved.*
*Enqpy™, EnqpyADS™, Enqpy™-Certified, and Security, Settled.™ are trademarks
of Enqpy™ Foundation Inc., licensed to NQP LLC.*
