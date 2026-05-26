<!--
BEFORE PUBLISHING — replace bracketed placeholders:
  [EPRINT_URL]      IACR ePrint paper (companion proof [12])
  [CONF_SPEC_URL]   Conformance Specification
  [TUG_URL]         Trademark Use Guidelines
Repo, license, and commercial links are already filled in from the LICENSE and FCD.
-->

# Enqpy™ Canonical Test Vectors

These vectors **are** the technical conformance test. A port that reproduces every one of them exactly — byte for byte — matches the Enqpy™ reference. They are exported from the reference self-test harness (`-DENQPY_SELFTEST`, **78/78 passing**) so that anyone can implement Enqpy™ in any language and prove their implementation is correct without trusting us, and without us trusting them.

Passing these vectors is the *technical* bar. It puts a port at the **Reference-Compatible** level (self-attested) and is the prerequisite for the Foundation's higher levels — but it is not certification, and it does not by itself grant any right to the Enqpy™ marks. See "Conformance levels" below.

All hexadecimal in these vectors is **UPPERCASE, no separators**, matching §2 of the Formal Cryptographic Description (FCD) and the reference implementation's output. The fundamental unit is the 4-bit nibble (one hex digit, value 0–15).

## Files

| File | Role |
| --- | --- |
| `enqpy-vectors.json` | **Canonical, machine-readable source of truth.** Every conformance check runs against this. |
| `enqpy-vectors.example.json` | A small copy showing the structure, seeded with the real vectors below. |

JSON is canonical because every language parses it with zero dependencies. If you also publish a human-readable `.txt`, **generate it from the JSON** so the two cannot drift; the JSON always wins.

## The I/O contract (base / Ideal Configuration)

Enqpy™ is a two-key stream cipher. The reference entry point is:

```
PDAF_SEC(ek, qk, or_nibs, or_ctr, n, target, nTextLen, out)  ->  bytes written, or -1
```

| Field | Meaning |
| --- | --- |
| `ek` | Encryption Key — master secret, `n` nibbles. MUST differ from `qk`. |
| `qk` | Query Key — companion master secret, `n` nibbles. |
| `or` | Open Return — per-message public nonce, `n` nibbles (CSPRNG; never reused per (EK,QK)). |
| `or_ctr` | 64-bit Open Return Counter (monotonic). Input as an integer. |
| `n` | Key length in nibbles: 32 (LOW), 48 (MEDIUM), or 64 (HIGH, default). |
| `pt` / `ct` | Plaintext / ciphertext bytes. Output length equals input length exactly. |

There is **no associated-data field** in the base cipher. AD / HMIX is the **EnqpyADS™** variant, specified separately; do not add an `ad` field to base-cipher vectors. The transmitted public value `eff_or` is *derived* inside the cipher from `or` + `or_ctr`; it is an intermediate, not an input.

## How the keystream combines with the message — it IS XOR

The keystream unit `W` is produced as nibbles through the cipher's internal pipeline, which uses MOD16 (modular-16) arithmetic throughout — PDAF, OWC, and the three Selection cases. **But the message-combine step is plain byte-wise XOR.** Per FCD Phase 4:

```
W_byte  = (W_nibble_even << 4) | W_nibble_odd     // high nibble = even index
CT_byte = W_byte XOR PT_byte                       // encrypt
PT_byte = W_byte XOR CT_byte                       // decrypt
```

So the reflexive `ct = pt ^ keystream` is correct. **Do not** reach for modular addition at the combine step — the MOD16 arithmetic lives inside `W` generation, not in the plaintext combine.

The real subtle-bug surface is the **`W` generation pipeline**, not the combine. A port can XOR correctly and still be wrong if it gets any of these wrong: the five phases (setup → key generation → Selection → cipher → Phase-5 update); PDAF Mode 1 tiling to length `2n + 16`; the CS-ordered three-case Selection and the **required CS re-derivation after each Phase-5 update**; and the Key Role Separation of the Ideal Configuration (EK and QK appear only as ValueKey; VKP/OKP derive from the nonce alone). The zero-plaintext vector below is the fastest way to isolate a `W`-generation bug, because there CT equals `W` directly.

## Verified canonical vectors

All values below are taken from the reference self-test (78/78 PASS) and are authoritative.

**OWC** (FCD §13.1) — input `FCB578`, `nSkip = 1` → output `B0F`.

**PDAF Mode 0** — VK `FB382C001A`, OK `CC69100AB4`, n=10, nDigits=30 →
`B7913C0ACE7FEBD00B53F4851014AF`

**PDAF Mode 1** — VK `FB382C001A`, OK `CC69100AB4`, n=10, nDigits=30 →
`7DD02C010CDF74C01B5BF8D811B92B`

**PDAF_SEC — Ideal Configuration, HIGH profile (n=64), zero plaintext.** Because the 8-byte plaintext is all zeros, the ciphertext equals the keystream `W[0..7]` exactly — making this the cleanest isolation of `W` generation.

```
EK     = CB1E1203C479F30C1C356F12362FE43B47E8B5906C992013468395489A17D957
QK     = 0E2EAB25A9F78620ABB6726CF81A012776511B3988431D427DA911BDC2130680
OR     = 3667A507E1109EE32CD50718FA511065900EB422AC187AC5CD47EF5B18D86E0C
OR_CTR = 1
PT     = 0000000000000000        (8 bytes)
CT     = 24C43F3E949BBC35        (8 bytes)  == W[0..7]
```

Round-trip: `PDAF_SEC` applied to `CT` with the same parameters returns the zero plaintext. Coset invariant ([+8] global shift): `EK + 8·1` and `QK + 8·1` (nibble-wise, mod 16) both produce the identical `CT` — a port that reproduces this confirms the Key Role Separation wiring of the Ideal Configuration.

## How to verify a port

1. Load `enqpy-vectors.json`.
2. Feed each test's inputs into your implementation at the right entry point (OWC, PDAF, or PDAF_SEC).
3. Compare your output to the expected value **byte-for-byte** (uppercase hex).
4. A port matches the reference when **100%** of the canonical vectors pass at the profile(s) you implement (n = 32 / 48 / 64).

Illustrative verifier (adapt to your real API):

```python
import json, binascii
from enqpy import pdaf_sec            # your port

hx = binascii.unhexlify
data = json.load(open("enqpy-vectors.json"))

passed = failed = 0
for grp in data["groups"]:
    if grp["group"] != "pdaf_sec":     # handle each group's entry point
        continue
    for t in grp["tests"]:
        out = pdaf_sec(ek=hx(t["ek"]), qk=hx(t["qk"]),
                       or_nibs=hx(t["or"]), or_ctr=int(t["or_ctr"]),
                       n=t["n"], target=hx(t["pt"]))
        got = binascii.hexlify(out).decode().upper()
        if got == t["ct"]:
            passed += 1
        else:
            failed += 1
            print("MISMATCH", t["id"])

print(f"{passed} passed, {failed} failed")
```

## Conformance levels

Reproducing these vectors proves your port is technically correct. Where that sits in the Foundation's program — defined in [`CONFORMANCE.md`](CONFORMANCE.md) and the Trademark Use Guidelines (TUG) — is three ordered levels:

- **Reference-Compatible** — self-attested: your port passes the published vectors, no Foundation review. You may state *"Reference-Compatible with the Enqpy™ reference implementation."* You may **not** put the Enqpy™ mark in your project name, branding, or marketing.
- **Conformant Implementation** — Foundation-reviewed: an independent reviewer walks your implementation against the FCD with no semantic divergence, and you're listed in the Foundation's public registry. Submit per `CONFORMANCE.md` §3.2 (acknowledged within 14 days; review 30–90 days at Rev 0.1).
- **Enqpy™-Certified** — formal Conformance Certificate plus trademark license; the only level that authorizes the Enqpy™ / Enqpy™-Certified marks in product naming. The full process publishes in Conformance Spec Rev 1.0 (Months 4–6 post-launch); first issuance is anticipated Months 9–12.

Passing these vectors puts you at **Reference-Compatible** — the entry level and the prerequisite to the other two. It does not by itself grant any right to the Enqpy™ marks. Give your port its own name and describe it as *"Reference-Compatible with the Enqpy™ reference implementation"* — **not** "Enqpy-Rust," "PyEnqpy," or any name that puts the mark in the project's name (TUG §5.1).

## What the vectors should cover

To make "passes the vectors" mean "passes the reference," export the full self-test set and include the edge cases:

- [ ] OWC, PDAF Mode 0, PDAF Mode 1 primitives (above)
- [ ] PDAF Mode 1 self-referential (VK = OK), the Phase-1 OR_EXP case (FCD §13.2a)
- [ ] PDAF_SEC zero plaintext (CT = W) — the isolation vector
- [ ] PDAF_SEC round-trip (decrypt(encrypt(PT)) == PT)
- [ ] [+8] coset invariants (EK+8 and QK+8)
- [ ] empty plaintext (zero-length)
- [ ] lengths crossing the n² W-cycle boundary, to exercise Phase-5 update + CS re-derivation
- [ ] each profile you support: n = 32, 48, 64
- [ ] EK = QK rejected (returns −1)

## Versioning & reporting

`vectors_version` is part of any conformance claim — a port states which version it passed. Never edit published vectors in place; bump the version instead. A discrepancy between a port and the vectors, or between the reference and the vectors, is a real finding — open an issue or contact RPM@enqpy.com.
