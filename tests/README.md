# Enqpy™ Test Vector Reference

The self-test harness in `src/enqpy_reference.c` verifies **78 assertions**
against the canonical test vectors published in the Formal Cryptographic
Description (FCD §13) and the ePrint paper (§15). All 78 must pass for the
implementation to be considered correct.

---

## Running the tests

```bash
./build.sh
```

Output begins:

```
Enqpy(tm) Stream Cipher -- Reference Implementation Rev 2.0
Ideal Enqpy(tm) Configuration (nonce-only OffsetKey derivation)
...
Self-test: 78 PASS  0 FAIL
Self-test PASSED.
```

A single failing assertion means the implementation disagrees with the
published test vectors and should not be trusted.

---

## Test vector inventory

### TV1 — PDAF Mode 0 (additive mode, n=10)

**Source:** FCD §13.2, Mode 0 row

| Parameter | Value |
|---|---|
| VK | `FB382C001A` |
| OK | `CC69100AB4` |
| nDigits | 30 |
| Expected output | `B7913C0ACE7FEBD00B53F4851014AF` |
| Assertions | 30 (one per output nibble) |

Verifies: the core PDAF primitive in its additive mode, used in OWC and
various cipher-internal operations.

---

### TV2 — PDAF Mode 1 (one-way gate mode, n=10)

**Source:** FCD §13.2, Mode 1 row

| Parameter | Value |
|---|---|
| VK | `FB382C001A` |
| OK | `CC69100AB4` |
| nDigits | 30 |
| Expected output | `7DD02C010CDF74C01B5BF8D811B92B` |
| Assertions | 30 (one per output nibble) |

Verifies: the primary cipher primitive. Mode 1 is the many-to-one gate whose
preimage multiplicity underlies Theorem 1 and the Shannon Ideal System result
(Theorem 2).

---

### TV3 — PDAF_SEC W-output (Ideal Configuration, n=64, 8 bytes)

**Source:** Test vector block embedded in `src/enqpy_reference.c` header
(Ideal Configuration output, produced from the Rev 1.0 TV3 keys and nonce).

| Parameter | Value |
|---|---|
| EK (hex) | `cb1e1203c479f30c1c356f12362fe43b47e8b5906c992013468395489a17d957` |
| QK (hex) | `0e2eab25a9f78620abb6726cf81a012776511b3988431d427da911bdc2130680` |
| OR (hex) | `3667a507e1109ee32cd50718fa511065900eb422ac187ac5cd47ef5b18d86e0c` |
| OR_CTR | `0x0000000000000001` |
| Plaintext | 8 bytes, all zero |
| Expected W[0..7] | `24 C4 3F 3E 94 9B BC 35` |
| Assertions | 8 |

Verifies: the full PDAF_SEC construction under the Ideal Enqpy™ Configuration
(Key Role Separation: `VKP = PDAF₁(OR, OR)[:n]`, `OKP = (VKP + DS_SEP) mod 16`,
`VKC = PDAF₁(EK, VKP)`, `OKC = PDAF₁(QK, OKP)`).

**Important:** Rev 1.0 of the paper/FCD published a different TV3 output
(`F4 38 41 E4 C2 B0 A0 6B`) for the HMIX-based standard configuration. The
Ideal Configuration uses nonce-only OffsetKey derivation and therefore
produces different VKP/OKP, hence different VKC/OKC and different W. The 8
bytes above are the Ideal Configuration output for the same keys and nonce.

---

### TV3 RT — Round-trip decryption (n=64, 8 bytes)

**Source:** Correctness property.

Applies `PDAF_SEC(EK, QK, OR, CT) = PT` with the ciphertext from TV3 as input.

| Assertion | Expected |
|---|---|
| Decrypted byte[0..7] | all zero (matches original plaintext) |
| Assertions | 8 |

Verifies: XOR streaming correctness. The round-trip confirms that the W
keystream is the same on encrypt and decrypt given the same (EK, QK, OR,
OR_CTR) — a necessary property for any stream cipher.

---

### TV4 — `[+8]` coset invariance on EK axis

**Source:** Theorem 2, Step 1 (Remark 1 in paper §6).

Computes PDAF_SEC with EK replaced by `EK + 8·1` (elementwise, mod 16), same
QK, same OR, same OR_CTR, same plaintext.

| Assertion | Expected |
|---|---|
| `CT(EK+8, QK, OR, PT) == CT(EK, QK, OR, PT)` byte-exact | TRUE |
| Assertions | 1 |

Verifies: the `[+8]` global shift invariant for the EK axis. This is the
algebraic mechanism that produces the 2-element ambiguity
{EK, EK+8·1} on the encryption-key axis. EK and EK+8·1 produce *literally
identical* ciphertext for every plaintext, every nonce, and every session.

---

### TV5 — `[+8]` coset invariance on QK axis

**Source:** Theorem 2, Step 1 (symmetric argument on QK).

Same as TV4 but shifts QK instead of EK.

| Assertion | Expected |
|---|---|
| `CT(EK, QK+8, OR, PT) == CT(EK, QK, OR, PT)` byte-exact | TRUE |
| Assertions | 1 |

Verifies: the `[+8]` global shift invariant for the QK axis. Together with
TV4, TV5 confirms the full 4-element ciphertext-equivalent coset
{EK, EK+8} × {QK, QK+8} empirically, matching the algebraic proof in the
paper.

---

## Total coverage

| Group | Count |
|---|---:|
| TV1 (PDAF Mode 0) | 30 |
| TV2 (PDAF Mode 1) | 30 |
| TV3 (PDAF_SEC W) | 8 |
| TV3 RT (round-trip) | 8 |
| TV4 (EK+8 coset) | 1 |
| TV5 (QK+8 coset) | 1 |
| **Total** | **78** |

---

## What is *not* covered in the embedded self-test

The following additional vectors from FCD §13 are not in the self-test but
can be verified independently:

- **FCD §13.1 — OWC test vector** (`FCB578` with nSkip=1 → `B0F`). OWC is
  exercised indirectly inside PDAF_SEC but not as a standalone assertion.
- **FCD §13.2a — PDAF Mode 1 self-referential** (VK = OK = OR_CTR_nibs for
  Phase 1 OR_EXP). Exercised inside PDAF_SEC via Phase 1 but not as a
  standalone assertion.
- **FCD §13.3 — PDAF_SEC full round-trip** (1,449,544-byte plaintext +
  byte-exact match after decrypt ∘ encrypt). Requires a test file; run with
  the `ENQPY_BENCHMARK` flag or via direct inquiry for full-size test data.
- **FCD §14 — NIST SP 800-22 statistical randomness** (15 / 15 tests PASS;
  p-values 0.121 – 0.939). Requires the NIST SP 800-22 test harness; full
  run available via direct inquiry.

These additional verifications are summarized in FCD §13 – §14 and are
available in extended form from NQP LLC on request.

---

## Adding your own tests

The self-test block is in `src/enqpy_reference.c` between the
`#ifdef ENQPY_SELFTEST` marker (around line 554) and the corresponding
`#endif`. Each TV uses a simple pattern:

```c
/* Set up keys, nonce, expected output */
/* Call the cipher */
/* Compare byte-for-byte */
/* Increment pass / fail counters */
```

Feel free to add additional vectors or edge cases. Reference vectors against
FCD §13 or derive new ones from the algebraic specification.

---

*Questions about the test vectors or requests for extended verification
data: [RPM@enqpy.com](mailto:RPM@enqpy.com).*
