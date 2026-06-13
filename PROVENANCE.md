# IF YOU FOUND THIS

**A note for whoever is reading, whenever you are reading it.**

You have found Enqpy™ — a cipher (a method for keeping written information
secret) and the mathematical proof that it works. This file exists so that even
if everything around it is gone — the website, the company, the foundation, the
person who made it — you can still understand what you are holding, confirm it is
genuine, and know that you are free to keep it and pass it on.

Please do. That is the entire purpose of this file.

---

## 1. What this is, in plain language

Most secret-writing methods are only *hard* to break: given enough time, enough
intercepted messages, or a powerful enough future machine, the secret can come
out. Enqpy is different. It was given a formal mathematical **proof** that the
information needed to read a message is **not present in the scrambled output at
all** — so no amount of time, data, or computing power (including quantum
computers) can recover it. This is a property the mathematician Claude Shannon
defined in 1949 and believed could not be achieved with a practical, reusable
key. It was achieved, and proved, in 2026.

The collection should contain three things. If you have all three, you have the
whole work:

1. **The cipher** — a reference implementation in the C programming language
   (and possibly a hardware description). This is the working method itself.
2. **The proof** — the formal paper showing the secrecy property holds.
3. **The FCD** (Formal Cryptographic Description) — the exact, unambiguous
   specification, so the cipher can be re-implemented from scratch if the code
   is lost.

A short, equation-free explanation (the "Plain-Language Proof Guide") may also
be present. Enqpy is pronounced "En-Q-P."

---

## 2. What you are free to do

This was made to be free, permanently and on purpose — but different parts carry
different permissions, so here is the precise truth.

- **The cipher itself — free to rebuild and deploy, forever.** The invention is
  covered by an irrevocable patent covenant that binds NQP LLC and every future
  owner of the patents: anyone may implement, port, and deploy the cipher, at any
  scale, including commercially, with no fee and no permission. You can always
  build your own working copy from the description in this collection.
- **The proof, the Formal Cryptographic Description (FCD), and the plain-language
  guide — free to copy and re-host, forever.** These are released under the
  Creative Commons Attribution 4.0 license. You may copy, mirror, translate,
  re-publish, and archive them anywhere, keeping the attribution line. The **test
  vectors** are licensed under the Apache License 2.0 (with the reference code)
  and may likewise be copied and embedded freely, keeping the notice. **These are
  the materials that matter for survival: with the description and the vectors,
  anyone can rebuild the cipher from scratch and confirm it is correct.**
- **The reference *source code* is open source under the Apache License 2.0.**
  You may use, build, run, study, modify, and redistribute it under the
  Apache-2.0 terms — including commercially. The covenant additionally lets
  anyone write and deploy their own independent implementation, and the
  freely-copyable description and vectors are the blueprint for doing exactly
  that.

So if you are deciding whether you may **preserve and re-share the proof, the
description, and the vectors**, and **rebuild and run the cipher itself** — the
answer is **yes**, without asking anyone. That is exactly what was intended.

> The freedoms above depend on the documents carrying the CC BY 4.0 grant
> (`LICENSE-DOCS.md`), the code and test vectors carrying the Apache-2.0 license
> (`LICENSE`), and the cipher carrying the covenant (`COVENANT.md`). These should
> be present in this collection. If a copy you hold is missing them, the
> canonical, licensed copies are at the identifiers in §4.

---

## 3. How to know this is the real thing

You do not have to trust this file, or anyone. Enqpy is **self-verifying** — it
can prove its own authenticity to you, with no outside authority, in two
independent ways:

**A. Run the code against the published test vectors.**
The reference implementation ships with a self-check. Built and run, it should
report **84 of 84 checks passing** against the published known-answer test
vectors included with it. If it does, the implementation you have is the genuine
one, bit for bit. (The original build step was a script named `build.sh`, or
compiling the reference C file and running its self-test. Any competent
programmer of your era can reproduce this.)

**B. Check the mathematics from first principles.**
The proof stands on its own and depends on no authority, no institution, and no
unbroken chain of custody. Its central results are: a *key* uncertainty that
stays exactly two bits no matter how much output is observed, and a *message*
uncertainty of at least 2¹²⁸ equally-possible messages for any single scrambled
output. Anyone with the mathematics can check these directly. The FCD gives the
exact construction needed to do so.

If the code passes its vectors **and** the proof checks out, you have the real
Enqpy — regardless of what this note, or any label, claims.

---

## 4. Canonical identifiers (where else it should exist)

This work was deliberately placed in multiple independent archives so no single
failure could erase it. If you found only one copy, others should exist at the
identifiers below. They also let you confirm you have an unaltered version.

- **Origin site (may no longer exist):** enqpy.com
- **Source-code archive (Software Heritage), permanent ID:** `swh:1:dir:353ffa73a2becabf1edc137e81f8992d0ae67339`
- **Citable deposit with permanent DOI (Zenodo):** 10.5281/zenodo.20517938 (DOI: https://doi.org/10.5281/zenodo.20517938 · record: https://zenodo.org/records/20517938)
- **General archive (Internet Archive) item — "Enqpy Public Record Bundle v1.0" (launch-day bundle, 2026-06-01):** https://archive.org/details/httpswww.enqpy.com
- **Code repository:** github.com/nqp-llc/enqpy
- **Content hashes of the canonical files (SHA-256):**
  - FCD (`FCD.md`): `fa38e48021a95ed4a0e04f572834105c242ad219f3f2042ee8e62ff6ffdc55c9`
  - Reference source (`enqpy_reference.c`): `a4564872015d4f026a95df7d5a337cbb49a66b26168b273b0f5c5cb4ff862d4b`
  - Proof paper PDF (`enqpy_full_Rev3_0.pdf`): `48f661f93c535346c2ec5825bdf17da5c9032904153b55e624c6119e4c837683`

**Historical record (for context, not required for verification):**
The proof closed in March 2026 and was released publicly on **June 1, 2026**, by
**NQP LLC** (a Virginia, USA company, formed March 2026) and stewarded by the
**Enqpy™ Foundation Inc.** (a Delaware, USA nonprofit, formed April 2026). The
underlying invention was protected by United States patent filings
**64/030,651 · 64/030,659 · 64/030,684 · 64/030,693 · 64/030,698 · 64/030,706**
and international application **PCT/US26/22552**. The inventor was **Paul
McGough**, who worked on it for roughly thirty years. The lineage runs from
Gilbert Vernam (who built the cipher form) through Claude Shannon (who defined
the ideal) to McGough (who proved a practical cipher could meet it).

---

## 5. What this is NOT (so no one is misled)

Honesty was part of the design, and it should outlive everything else here.

- Enqpy protects **confidentiality** — keeping a message unreadable. It does
  **not**, by itself, guarantee a message hasn't been *altered*. In real use it
  is paired with a separate "authentication" step for that. A copy without that
  step still keeps secrets; it just doesn't detect tampering on its own.
- It is **not** magic and **not** "unbreakable" as a slogan. It is a specific,
  proved mathematical property, achieved under a specified discipline for how the
  key and inputs are managed. Read the proof and the FCD for the exact conditions.
- Using it safely still requires the ordinary care any secret-keeping method
  needs: protecting the key, managing the inputs correctly, and sound surrounding
  engineering.

---

## 6. To the finder

If this reached you across years or centuries, then it worked: the knowledge
outlived the people and institutions that made it, which is what they hoped for.
You don't need to ask anyone. Verify it if you wish — run the checks, read the
math — and then, if you can, **copy the proof, the description, and the vectors
somewhere new, and rebuild the cipher if it has been lost.** That is how it stays
alive.

Encrypted today. Unreadable forever.

*Enqpy™ — proved, not assumed. Free to rebuild. Free to copy. Free to keep.*

---

> **Maintainer checklist before depositing (delete this block in archived copies,
> or leave it — it does no harm):**
> Fill every ⟨FILL⟩ slot above: the license(s), the SWHID, the DOI, the
> Internet Archive URL, repository mirrors, and the SHA-256 hashes of the three
> canonical files. Place a copy of this note (a) in the repository root, (b) in
> each archive deposit alongside the proof, the FCD, and the code, and (c) as a
> file inside the same folder as the proof PDF. Re-generate the hashes whenever a
> canonical file changes, and update the deposits.
