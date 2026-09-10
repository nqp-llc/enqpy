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

Most secret-writing methods are only *hard* to break: the secret stays safe
because nobody is believed to have a fast enough method or a big enough machine.
That is a belief, and beliefs can be overturned. Enqpy is different in a
specific, proved way. It was given a formal mathematical **proof** that, **from
the scrambled output alone**, at least 2^128 different messages remain
mathematically consistent with what an eavesdropper holds — and that number is
a counting fact about the mathematics, not a bet on a computation staying hard,
so no amount of computing power (quantum included) erases it. This is the
property the mathematician Claude Shannon defined in 1949 and believed could not
be achieved with a practical, reusable key; the ciphertext-only form of it was
achieved, and proved, in 2026.

Be careful about what that does and does not say. It does **not** say the
information "is not in the ciphertext" — a great deal about the message is
narrowed down by it. It says the ciphertext alone never singles the message out.
And for ordinary, redundant text there is a finite length beyond which the
proof's uncertainty conclusion no longer applies. The strong, unconditional part
is this: the ambiguity is *proved* rather than assumed. Using it safely also
depends on the key-management discipline described in §5.

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

A second, independent check ships alongside: `tests/vectors/vectors_check.c`
links the reference and asserts every value in `enqpy-vectors.json` — all three key
profiles, the key-domain rule, the window boundary, the counter expansion and the
rotation policy — 34 assertions in one pass. Build it the same way and it should
report that every published vector reproduces exactly. The self-test proves the
code is consistent with itself; this proves the code is consistent with the
published data.

**B. Check the mathematics from first principles.**
The proof stands on its own and depends on no authority, no institution, and no
unbroken chain of custody. Its central results are: a *message* uncertainty of
at least 2¹²⁸ mathematically consistent messages for any single scrambled output
(the primary result — and under the proof's stated uniform priors those are
equally likely), and a *key* uncertainty that never falls below two bits no
matter how much ciphertext is observed. Anyone with the mathematics can check
these directly. The FCD gives the exact construction needed to do so.

If the code passes its vectors **and** the proof checks out, you have the real
Enqpy — regardless of what this note, or any label, claims.

---

## 4. Canonical identifiers (where else it should exist)

This work was deliberately placed in multiple independent archives so no single
failure could erase it. If you found only one copy, others should exist at the
identifiers below. They also let you confirm you have an unaltered version.

> **Note on revisions.** The archive identifiers below were minted for the Rev 5.0
> launch bundle of 1 June 2026. Rev 5.1 is a documentation revision: it changed no
> cipher behaviour, and the Rev 3.0 known-answer test vectors still reproduce
> byte-for-byte, so a Rev 5.0 copy remains a genuine and usable copy of the cipher.
> What Rev 5.1 corrected is what the documents *claim* — several statements were
> withdrawn as unsupported, and the key-domain rule changed (equal keys are now
> permitted and must not be rejected). If you hold a Rev 5.0 copy, the code is
> sound; prefer the Rev 5.1 documents where the two disagree. The hashes below are
> for Rev 5.1.

- **Origin site (may no longer exist):** enqpy.com
- **Source-code archive (Software Heritage), permanent ID:** `swh:1:dir:353ffa73a2becabf1edc137e81f8992d0ae67339`
- **Citable deposit with permanent DOI (Zenodo):** 10.5281/zenodo.20517938 (DOI: https://doi.org/10.5281/zenodo.20517938 · record: https://zenodo.org/records/20517938)
- **General archive (Internet Archive) item — "Enqpy Public Record Bundle v1.0" (launch-day bundle, 2026-06-01):** https://archive.org/details/httpswww.enqpy.com
- **Code repository:** github.com/nqp-llc/enqpy
- **Content hashes of the canonical files (SHA-256, Rev 5.1):**

  ```
  59b89ca11d363d8d4585ce0895bb1b07237994955e1533326afcf8c78471d741  FCD.md
  b9366a89e86a1acc078c7ae12d31ca399793fbb586697a7ecb01b2ab5203ad60  enqpy_reference.c
  19b77d773645cfebca97b57dd01f918b33d3c7c15b06aff4e7d8d5a066168039  enqpy_full_Rev5_1.pdf
  e746a7b67b5c6b3664b348ff32ac1a726657f12e348095402711717e3026ba22  enqpy-vectors.json
  ```

  A `SHA256SUMS` file carrying exactly these four lines should sit beside the
  files in every deposit. Verify with `sha256sum -c SHA256SUMS`.

  The test vectors are included in this list deliberately: §2 says the
  description and the vectors are what let anyone rebuild the cipher and confirm
  it is correct, so the vectors are a survival artifact and need a hash like the
  rest.

  **The proof PDF is byte-reproducible.** LaTeX normally stamps a build time into
  the PDF, which would make its hash differ on every build and useless for
  verification. The published PDF is therefore built with the timestamp pinned:

  ```
  SOURCE_DATE_EPOCH=1788998400 FORCE_SOURCE_DATE=1 \
      latexmk -pdf enqpy_full_Rev5_1.tex
  ```

  (1788998400 is 2026-09-10T00:00:00Z.) Rebuilding from `enqpy_full_Rev5_1.tex`
  with those variables set reproduces the hash above exactly. Without them the
  content is identical but the hash will not match — if you are verifying a copy
  and only the PDF hash differs, check that first before concluding anything is
  wrong. `FCD.md`, `enqpy_reference.c` and `enqpy-vectors.json` are plain text and
  have no such caveat.

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
- The proved property is about the **scrambled output on its own**: the
  ciphertext never singles the message out. If an attacker also learns enough of
  the *original* text that went with intercepted outputs, and the **same key
  keeps being reused**, that knowledge can build up and eventually unlock the
  rest of that record. The intended discipline prevents spread: encrypt each
  record under its own distinct, non-reused credential, so a build-up inside one
  record cannot reach another. The proof and the FCD state the boundary and the
  per-record credential rule.
- The proof's uncertainty conclusion assumes the message could have been
  anything (a uniform prior). Ordinary language is not like that, and for
  redundant text there is a finite length beyond which the ciphertext no longer
  leaves the message genuinely undetermined. Every finite-key cipher has this
  limit. What is unusual here is that the ambiguity is proved rather than
  assumed.
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
