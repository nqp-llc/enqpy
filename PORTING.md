# Porting Enqpy™ — ports & bindings welcome

Enqpy™ (pronounced *"En-Q-P"*) is meant to be used. If you want it in your language, runtime, or stack, please port it — you don't need to ask anyone to start writing one.

Two things to read before you ship a port: how the rights work (the cipher is patented and the reference is source-available, not OSI open source), and how naming works (the Enqpy™ marks are certified, not self-claimed).

## How the rights work

Two instruments govern use, and they do different jobs. The **Open-Infrastructure Patent Non-Assertion Covenant** (at https://enqpy.com/covenant and in this repo) covers the **patent**; the **NQP LLC Public License** (see [`LICENSE`](LICENSE)) covers **copyright in NQP's reference source**. In short:

- **The cipher is patent-safe for everyone.** Under the Covenant, NQP LLC irrevocably commits not to assert the NQP Patents against any conforming implementation of the cipher — by anyone, for any purpose, at any scale, including commercial deployment. No fee, no signup, and no license is required to write a port, deploy it, or ship it in a product. There are no size thresholds and nothing to "outgrow."
- **Source-code rights — everyone.** Under the Evaluation License, anyone may clone, build, run the self-test and benchmark harnesses, analyze the source, compare output against the vectors and paper, and cite the work with attribution. You may **not** redistribute NQP's reference source, relicense it, or remove the license/notice. Your port is your own code — writing an independent implementation needs no copyright license here; that right comes from the Covenant.
- **Optional commercial relationships — the earned edge.** Foundation certification + marks, NQP's high-performance proprietary implementations, services/support/indemnity, the Partner Program, and an optional signed patent license are available at **https://enqpy.com/use.html** for organizations that want them — but none is a condition of using the cipher.

A port is your own code implementing a patent-safe cipher: write it, deploy it, ship it under the Covenant, with no license required for that use. The [Covenant](https://enqpy.com/covenant) and the [`LICENSE`](LICENSE) are the authoritative terms — this page is a plain-English pointer, not the grant itself.

## How naming works

The Enqpy™ marks are governed by the [Trademark Use Guidelines](https://enqpy.com/foundation.html) and the [`CONFORMANCE.md`](CONFORMANCE.md) levels. The Foundation recognizes **three ordered conformance levels**:

- **Reference-Compatible** (self-attested) — your port passes the published vectors; no Foundation review. You may state **"Reference-Compatible with the Enqpy™ reference implementation."** This is the level a community port reaches the moment its vectors pass.
- **Conformant Implementation** (Foundation-reviewed) — an independent reviewer walks your implementation against the FCD; you're listed in the Foundation's public registry. Submit per `CONFORMANCE.md` §3.2.
- **Enqpy™-Certified** (certificate + trademark license) — the only level that authorizes the Enqpy™ marks in product naming/branding. Full process publishes in Conformance Spec Rev 1.0 (Months 4–6 post-launch); first issuance ~Months 9–12.

Two naming rules that trip people up:

- **Name your port your own name.** You may **not** call it "Enqpy-Rust," "Enqpy.go," "PyEnqpy," or anything that puts the mark in the project's name (TUG §5.1). Pick your own name, then describe it as "Reference-Compatible with the Enqpy™ reference implementation."
- **Don't claim a level you don't hold.** "Conformant" and "Certified" require the corresponding Foundation review; until then you are Reference-Compatible by self-attestation, and the Enqpy™ mark stays out of your branding and marketing.

So: write the port freely; deploy it under the Covenant (patent-safe, no license required for that use); name it your own name and describe it as Reference-Compatible with Enqpy™.

## What you need

Everything to build and verify a port lives in this repo:

- **Canonical test vectors** — `tests/vectors/enqpy-vectors.json`, documented in [`TEST_VECTORS.md`](TEST_VECTORS.md). The authoritative inputs and expected outputs (UPPERCASE hex). Your port matches the reference if and only if it reproduces these exactly.
- **Reference implementation** — the C reference (`-DENQPY_SELFTEST` runs 78/78). Ground truth when in doubt.
- **Formal Cryptographic Description (FCD)** — the full specification of OWC, PDAF, PDAF_SEC, the five phases, profiles, and key management.
- **The proof / paper** — [the formal proof paper](https://enqpy.com/technical.html) (companion [12]). Why it works; not required to port.
- **Conformance Specification** — [`CONFORMANCE.md`](CONFORMANCE.md). The three conformance levels, canonical sources, and the submission process.
- **Trademark Use Guidelines** — [link](https://enqpy.com/foundation.html). The naming rules above.

## How to verify your port

Run it against the canonical vectors and match every one byte-for-byte at the profile(s) you implement (n = 32 LOW / 48 MEDIUM / 64 HIGH). Full procedure, the I/O contract, and a verifier example are in [`TEST_VECTORS.md`](TEST_VECTORS.md). Start with the zero-plaintext vector — there the ciphertext equals the keystream, so it isolates keystream generation from the XOR.

## Get your port listed

Open an issue or pull request:

```
Port: Enqpy for <language / runtime>
Repository: <url>
Author / maintainer: <name or handle>
Profile(s) implemented: <LOW n=32 / MEDIUM n=48 / HIGH n=64>
Vectors version: <which enqpy-vectors.json version you ran against>
Vector results: <summary — e.g., "all canonical vectors pass; output attached/linked">
Notes: <anything we should know>
```

We verify against the canonical vectors and, on a pass, list your port in [`PORTS.md`](PORTS.md) at the **Reference-Compatible** level — described as "Reference-Compatible with the Enqpy™ reference implementation." For the Foundation-reviewed **Conformant Implementation** level, submit per `CONFORMANCE.md` §3.2; **Enqpy™-Certified** (the only level that licenses the mark for product use) is separate (https://enqpy.com/use.html).

## One note specific to cryptography

A port that *looks* like it works can still be subtly wrong in a way that silently weakens security — and it would carry the Enqpy™ name. The message combine is plain XOR, so that part is hard to get wrong; the real bug surface is keystream (`W`) generation — the five phases, PDAF Mode 1 tiling, the CS-ordered Selection with required re-derivation after each Phase-5 update, and the Ideal Configuration's key-role separation. That is exactly why the vectors define correctness by exact reproduction rather than "it runs." If you find a discrepancy with the vectors or the reference, open an issue — that's a report we want.

## Questions

RPM@enqpy.com — or open an issue. Commercial: https://enqpy.com/use.html (or email with `[Commercial]` in the subject).

Ports welcome. Tell us where Enqpy™ ends up. :)
