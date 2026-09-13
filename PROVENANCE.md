# IF YOU FOUND THIS

**A note for whoever is reading, whenever you are reading it.**

---

## 1. What this is

**You have found Enqpy™.**

It is a working finite-key stream-cipher core, the exact specification needed to
rebuild it, and the formal proof of its defining ciphertext-only property.

At HIGH, every ciphertext produced by the proved core is mathematically
consistent with at least 2^128 plaintexts. That floor is a consequence of the
algebra of the construction, not an assumption that some computation remains
difficult, so additional computing power cannot erase it.

**To the author's knowledge, no prior finite-key cipher construction had carried
a formal proof of a non-vanishing ciphertext-only equivocation property of this
kind.**

Claude Shannon framed the finite-key equivocation question in 1949. Enqpy
supplies a concrete proved construction for the ciphertext-only form studied in
the accompanying proof.

This file exists for one reason: so the result can survive the website, the
company, the Foundation, and the person who made it. It tells you what you are
holding, how to verify it, what you are free to preserve and rebuild, and where
canonical copies should exist.

**Do not trust the claim because this file says so. Read the proof. Run the
code. Compare the vectors.**

### What the collection contains

If you have all three of these, you have the technical core of the work —
the test vectors, the covenant and the license files matter too, and are listed
in §2 and §4:

1. **The cipher** — a reference implementation in the C programming language
   (and possibly a hardware description). This is the working method itself.
2. **The proof** — the formal paper establishing the stated
   ciphertext-only property.
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
  owner of the patent estate: anyone may implement, port, and deploy the cipher, at any
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

## 3. How to verify what you have

**Three checks answer three different questions.** The canonical hashes and
archive identifiers tell you whether the files match the published artifacts.
The test vectors tell you whether an implementation reproduces the specified
cipher. The mathematics tells you whether the claimed theorem is valid. None of
those checks substitutes for the others.

**A. Check the artifacts — hashes and archives.**
Compare the SHA-256 values in §4 against the files you hold, or run
`sha256sum -c SHA256SUMS`. A match means your copy is byte-identical to the
published one. The archive identifiers in §4 are independent places the same
files should exist, so a copy can be checked against something other than this
note.

**B. Check the implementation — the self-test and the vectors.**
The reference implementation ships with a self-check. Built and run, it should
report **84 of 84 checks passing** against the published known-answer test
vectors included with it. (The original build step was a script named
`build.sh`, or compiling the reference C file and running its self-test. Any
competent programmer of your era can reproduce this.)

A second, independent check ships alongside: `tests/vectors/vectors_check.c`
links the reference and asserts every value in `enqpy-vectors.json` — all three key
profiles, the key-domain rule, the window boundary, the counter expansion and the
rotation policy — 34 assertions in one pass. The self-test proves the code is
consistent with itself; this proves the code is consistent with the published
data. Together they establish that an implementation reproduces the specified
cipher. They do not establish where the files came from — that is check A.

**C. Check the theorem — the proof and the FCD.**
The proof stands on its own and depends on no authority, no institution, and no
unbroken chain of custody. Its central results are: at HIGH, a plaintext
support of at least 2¹²⁸ mathematically consistent plaintexts for any ciphertext
— and, under the proof's stated uniform key and plaintext priors, a uniform
posterior over that set — together with a ciphertext-only nominal-key
equivocation floor of at least two bits. Anyone with the mathematics
can check these directly. The FCD gives the exact construction needed to do so.

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
> sound; prefer the Rev 5.1 documents where the two disagree.
>
> **Rev 5.1 was corrected and frozen on 13 September 2026**, and the hashes below
> are for that final build. Again no cipher behaviour changed and no test vector
> moved. Three things were fixed. First, several passages had said the cipher
> "does not survive redundant plaintext past the unicity distance" — wording that
> ran together two different statements, only one of which is established. The
> equally-likely conclusion does not extend to ordinary language; the counting
> bound does, and is unaffected. Second, every remaining place where the proof or
> the FCD stated the uniform posterior or the entropy equality now names the
> priors those conclusions require, while the support bound is stated as holding
> for every plaintext distribution. Third, five paragraphs of the FCD carried
> characters lost to an earlier format conversion, including two occurrences of
> the preimage-count formula. `enqpy_reference.c` and `enqpy-vectors.json` are
> untouched and keep the hashes they had.

- **Origin site (may no longer exist):** enqpy.com
- **Source-code archive (Software Heritage), permanent ID:** `swh:1:dir:353ffa73a2becabf1edc137e81f8992d0ae67339`
- **Citable deposit with permanent DOI (Zenodo):** 10.5281/zenodo.20517938 (DOI: https://doi.org/10.5281/zenodo.20517938 · record: https://zenodo.org/records/20517938)
- **General archive (Internet Archive) item — "Enqpy Public Record Bundle v1.0" (launch-day bundle, 2026-06-01):** https://archive.org/details/httpswww.enqpy.com
- **Code repository:** github.com/nqp-llc/enqpy
- **Content hashes of the canonical files (SHA-256, Rev 5.1):**

  ```
  ec56c2a2742697c0831c52901fcee42a0814059ac3a083c0c00dd926d8090317  FCD.md
  b9366a89e86a1acc078c7ae12d31ca399793fbb586697a7ecb01b2ab5203ad60  enqpy_reference.c
  bc1c56b621d700ea199875b80702189dc4cef3584a5b484f95858356ce3d8905  enqpy_full_Rev5_1.pdf
  e746a7b67b5c6b3664b348ff32ac1a726657f12e348095402711717e3026ba22  enqpy-vectors.json
  ```

  Superseded (the 10 September 2026 build of the same revision), recorded so a
  copy carrying these is recognised as genuine rather than altered:

  ```
  59b89ca11d363d8d4585ce0895bb1b07237994955e1533326afcf8c78471d741  FCD.md
  19b77d773645cfebca97b57dd01f918b33d3c7c15b06aff4e7d8d5a066168039  enqpy_full_Rev5_1.pdf
  ```

  A `SHA256SUMS` file carrying exactly these four lines should sit beside the
  files in every deposit. Verify with `sha256sum -c SHA256SUMS`.

  The test vectors are included in this list deliberately: §2 says the
  description and the vectors are what let anyone rebuild the cipher and confirm
  it is correct, so the vectors are a survival artifact and need a hash like the
  rest.

  `FCD.md` is a Markdown rendering of the canonical Word document
  (`Enqpy_FCD_Rev5_1.docx`): the same text, converted with `pandoc`, plus a
  table of contents and a footer line that the Word footer does not carry across.
  It is the hashed artifact because plain text survives formats. Where the two
  ever disagree, the Word document governs.

  **The proof PDF is byte-reproducible.** LaTeX normally stamps a build time into
  the PDF, which would make its hash differ on every build and useless for
  verification. The published PDF is therefore built with the timestamp pinned:

  ```
  SOURCE_DATE_EPOCH=1788998400 FORCE_SOURCE_DATE=1 \
      latexmk -pdf enqpy_full_Rev5_1.tex
  ```

  (1788998400 is 2026-09-10T00:00:00Z. It is a fixed constant chosen so the build
  is repeatable, not a claim about when the document was last edited.) Rebuilding
  from `enqpy_full_Rev5_1.tex` with those variables set, **and with the same TeX
  distribution**, reproduces the hash above exactly. A different TeX version can
  produce a byte-different but content-identical PDF. So if you are verifying a
  copy and only the PDF hash differs, check the build variables and the
  toolchain before concluding anything is wrong — and if you can, check the
  `.tex` source instead, which has no such caveat. `FCD.md`, `enqpy_reference.c`
  and `enqpy-vectors.json` are plain text and have no such caveat either.

**Historical record (for context, not required for verification):**
The proof closed in March 2026 and was released publicly on **June 1, 2026**, by
**NQP LLC** (a Virginia, USA company, formed March 2026) and stewarded by the
**Enqpy™ Foundation Inc.** (a Delaware, USA nonprofit, formed April 2026). The
underlying invention was protected by United States patent filings
**64/030,651 · 64/030,659 · 64/030,684 · 64/030,693 · 64/030,698 · 64/030,706**
and international application **PCT/US26/22552**. The inventor was **Paul
McGough**, who worked on it for roughly thirty years. The work sits in the stream-cipher
lineage associated with Vernam and uses Shannon's equivocation framework;
Enqpy's claimed contribution is the concrete finite-key ciphertext-only result
proved in the accompanying paper.

---

## 5. Technical scope

Enqpy is a symmetric confidentiality core. The proved property is
ciphertext-only. Conforming deployment uses fresh, non-reused credentials per
record and authenticates each record.

The proof and the FCD contain the exact adversary model, the source-prior
distinction, the known-plaintext boundary and the deployment rules. **They
govern; this preservation note does not attempt to restate them.**

If you are implementing Enqpy rather than preserving it, read those documents
first.

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
> Confirm the license files, the SWHID, the DOI, the Internet Archive URL, the
> repository mirrors, and the SHA-256 hashes of all **four** canonical files
> (`FCD.md`, `enqpy_reference.c`, `enqpy_full_Rev5_1.pdf`, `enqpy-vectors.json`)
> are current. Place a copy of this note (a) in the repository root, (b) in each
> archive deposit alongside the proof, the FCD, and the code, and (c) as a file
> inside the same folder as the proof PDF. Re-generate the hashes whenever a
> canonical file changes, update `SHA256SUMS` in the same pass, and update the
> deposits. **Open:** the Zenodo DOI, the Software Heritage SWHID and the
> Internet Archive item still point at the Rev 5.0 launch bundle, so the hashes
> above do not yet correspond to anything archived.
