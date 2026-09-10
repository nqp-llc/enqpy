# Enqpy™ Canonical Test Vectors

These vectors **are** the technical conformance test. A port that reproduces every one of them exactly — byte for byte — matches the Enqpy™ reference. The repository ships a single canonical C reference, `enqpy_reference.c` (Enqpy, Case-1 `W` generation — `-DENQPY_SELFTEST` runs **84/84**). It is the conformance target for the Rev 5.1 ciphertext-only core claims. Rev 5.1 changed no cipher behaviour, so the Rev 3.0 KATs are unchanged and reproduce byte-for-byte; vectors **v5.1** adds four cases and edits none in place.

Passing these vectors is the *technical* bar. It puts a port at the **Reference-Compatible** level (self-attested) and is the prerequisite for the Foundation's higher levels — but it is not certification, and it does not by itself grant any right to the Enqpy™ marks. See "Conformance levels" below.

All hexadecimal in these vectors is **UPPERCASE, no separators**, matching §2 of the Formal Cryptographic Description (FCD) and the reference implementation's output. The fundamental unit is the 4-bit nibble (one hex digit, value 0–15).

## Files

| File | Role |
| --- | --- |
| `enqpy-vectors.json` | **Canonical, machine-readable source of truth.** Every conformance check runs against this. |
| `vectors_check.c` | Reference verifier — sits beside the vectors it checks, links the C reference and asserts every published value (34 assertions). Run `make vectors` from the repository root, or build directly: `cc -O2 -std=c11 -I src tests/vectors/vectors_check.c -o vectors_check`. |
| `enqpy-vectors.example.json` | A small copy showing the structure, seeded with the real vectors below. |

JSON is canonical because every language parses it with zero dependencies. If you also publish a human-readable `.txt`, **generate it from the JSON** so the two cannot drift; the JSON always wins.

## The I/O contract (Canonical Configuration)

Enqpy™ is a two-key stream cipher. The reference entry point is:

```
PDAF_SEC(ek, qk, or_nibs, or_ctr, n, target, nTextLen, out)  ->  bytes written, or -1
```

| Field | Meaning |
| --- | --- |
| `ek` | Encryption Key — master secret, `n` nibbles. Sampled independently and uniformly; there is **no** `ek ≠ qk` restriction (Rev 5.1). |
| `qk` | Query Key — companion master secret, `n` nibbles. |
| `or` | Open Return — per-message public nonce, `n` nibbles. MUST come from a CSPRNG; freshness is probabilistic, not structural (the map OR → VKP is not injective — FCD §7.2). |
| `or_ctr` | 64-bit Open Return Counter (monotonic). Input as an integer. |
| `n` | Key length in nibbles: 32 (LOW), 48 (MEDIUM), or 64 (HIGH, default). |
| `pt` / `ct` | Plaintext / ciphertext bytes. Output length equals input length exactly. |

There is **no associated-data field** in Enqpy; do not add an `ad` field to the vectors. The transmitted public value `eff_or` is *derived* inside the cipher from `or` + `or_ctr`; it is an intermediate, not an input.

## How the keystream combines with the message — it IS XOR

The keystream unit `W` is produced as nibbles through the cipher's internal pipeline, which uses MOD16 (modular-16) arithmetic throughout — PDAF, OWC, and Case-1 `W` generation. **But the message-combine step is plain byte-wise XOR.** Per FCD Phase 4:

```
W_byte  = (W_nibble_even << 4) | W_nibble_odd     // high nibble = even index
CT_byte = W_byte XOR PT_byte                       // encrypt
PT_byte = W_byte XOR CT_byte                       // decrypt
```

So the reflexive `ct = pt ^ keystream` is correct. **Do not** reach for modular addition at the combine step — the MOD16 arithmetic lives inside `W` generation, not in the plaintext combine.

The real subtle-bug surface is the **`W` generation pipeline**, not the combine. A port can XOR correctly and still be wrong if it gets any of these wrong: the five phases (setup → key generation → `W` generation → cipher → Phase-5 update); PDAF Mode 1 tiling to length `2n + 16`; Case-1 `W` generation; and the Key Role Separation of the Canonical Configuration (EK and QK appear only as ValueKey; VKP/OKP derive from the nonce alone). The zero-plaintext vector below is the fastest way to isolate a `W`-generation bug, because there CT equals `W` directly.

## Verified canonical vectors

All values below are taken from the reference self-test and are authoritative.

**OWC** (FCD §13.1) — input `FCB578`, `nSkip = 1` → output `B0F`.

**PDAF Mode 0** — VK `FB382C001A`, OK `CC69100AB4`, n=10, nDigits=30 →
`B7913C0ACE7FEBD00B53F4851014AF`

**PDAF Mode 1** — VK `FB382C001A`, OK `CC69100AB4`, n=10, nDigits=30 →
`7DD02C010CDF74C01B5BF8D811B92B`

**PDAF_SEC — HIGH profile (n=64), zero plaintext.** Because the 8-byte plaintext is all zeros, the ciphertext equals the keystream `W[0..7]` exactly — the cleanest isolation of `W` generation.

```
EK     = CB1E1203C479F30C1C356F12362FE43B47E8B5906C992013468395489A17D957
QK     = 0E2EAB25A9F78620ABB6726CF81A012776511B3988431D427DA911BDC2130680
OR     = 3667A507E1109EE32CD50718FA511065900EB422AC187AC5CD47EF5B18D86E0C
OR_CTR = 1
PT     = 0000000000000000        (8 bytes)

CT = 2434B58845C6FDE8   (8 bytes)  == W[0..7]
```

It round-trips: applying `PDAF_SEC` to the `CT` with the same parameters returns the zero plaintext. The `[+8]` coset invariants (`EK + 8·1`, `QK + 8·1`, and both together — nibble-wise mod 16) reproduce the identical `CT`, confirming the Key Role Separation wiring. The self-test additionally verifies the 2,048-byte window boundary (TV7) and the NIL key-update policy (TV8); see the README self-test breakdown.

## Profile key material — and why it is a suffix

The HIGH vectors use the canonical `EK` / `QK` / `OR` in full. The **LOW (n=32)**
and **MEDIUM (n=48)** vectors use the **last** `n` nibbles of those same canonical
values:

| Profile | n | Key material | Window |
| --- | ---: | --- | ---: |
| LOW | 32 | last 32 nibbles of the canonical values | 512 bytes |
| MEDIUM | 48 | last 48 nibbles | 1,152 bytes |
| HIGH | 64 | the canonical values in full | 2,048 bytes |

The suffix is deliberate, and the reason is worth knowing if you ever generate
your own profile vectors. With a **prefix**, all three profiles produce the
*identical* first sixteen keystream nibbles: at `c = 0` no index exceeds
`p + 15 + 1 = 31 < n`, and with `or_ctr = 1` the counter expansion agrees over
that range — so the zero-plaintext ciphertext would depend only on the first 32
nibbles, which a prefix makes common to all three. A prefix-derived vector would
therefore be reproduced by an implementation that **ignored `n` entirely**, and
would not test what it claims to test.

With the suffix the three profiles disagree, and the published `ct` values differ
accordingly:

| Vector | LOW | MEDIUM | HIGH |
| --- | --- | --- | --- |
| zero-plaintext `ct` | `880D1D26F1A183AD` | `AD3855C36B7D4B41` | `2434B58845C6FDE8` |
| `EK = QK` `ct` | `A26E1C021AD0230E` | `7DF090DDBA60C8CC` | `B8DCA43E483718B3` |
| window tail | `5F D3` @ [510,511] | `96 64` @ [1150,1151] | `54 28` @ [2046,2047] |

## Two groups worth a note

**`pdaf_mode1_selfref`** publishes the Phase-1 `OR_EXP` expansion of FCD §13.2a:
`OR_EXP = PDAF1(or_ctr_nibs, or_ctr_nibs)`, the same array as both ValueKey and
OffsetKey. Small counters give visibly low-entropy output — `or_ctr = 1` expands
to `…0011`, `or_ctr = 2` to `…0022` — and that is **expected and correct**.
`OR_EXP` contributes counter state and diffusion, *not* uniqueness. Nonce
freshness is probabilistic and comes from the CSPRNG `OR` component (FCD §7.2).
The `or_ctr = 0xA5C3` vector is the one that shows real propagation.

**`nil_comm_update`** publishes the rotation policy. Method 2 (external entropy)
is required; Method 1 (deterministic chain) must be **rejected**, because a
public chain is simulable — an adversary holding the current key state computes
the next one, so the rotation resets nothing. Method 2 called *without* external
entropy must also be rejected. One thing not to misread: mapping the `[+8]` coset
to a single new pair is **not** what distinguishes the two methods; Method 2 does
it too, and the vector records that explicitly. Simulability is the whole reason.

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

Published in **v5.1** and mechanically checkable today:

- [x] OWC, PDAF Mode 0, PDAF Mode 1 primitives (above)
- [x] PDAF_SEC zero plaintext (CT = W) — the isolation vector
- [x] PDAF_SEC round-trip (decrypt(encrypt(PT)) == PT)
- [x] `[+8]` coset invariants — EK+8, QK+8, **and both axes together**
- [x] EK = QK **accepted** and round-trips (`sec-0002-ekEqualsQk`). Rev 5.1 removed the equality prohibition; an implementation that rejects or resamples equal keys is **non-conformant**, and this is the vector that catches it
- [x] empty plaintext, zero-length (`sec-0003-emptyPT`, returns 0)
- [x] lengths crossing the n² W-cycle boundary, exercising the Phase-5 update (`sec-0004-windowBoundary`)

- [x] each profile: **n = 32 (LOW), 48 (MEDIUM), 64 (HIGH)** — zero-plaintext, EK=QK, and window-boundary vectors published for all three
- [x] PDAF Mode 1 self-referential (VK = OK), the Phase-1 OR_EXP case (FCD §13.2a) — `pdaf1sr-0001/2/3`
- [x] NIL key-update policy — Method 1 rejected, Method 2 accepted, Method 2 without external entropy rejected (`nil-0001/2/3`)

**The export is complete.** Every case the self-test exercises is now published as a
machine-checkable vector, and `tests/vectors/vectors_check.c` runs all 34
assertions against the reference in one pass — `make vectors`, or `make check` to
run it together with the 84/84 self-test.

## Versioning & reporting

`vectors_version` is part of any conformance claim — a port states which version it passed. Never edit published vectors in place; bump the version instead. A discrepancy between a port and the vectors, or between the reference and the vectors, is a real finding — open an issue or contact RPM@enqpy.com.
