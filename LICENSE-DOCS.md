# Enqpy™ Documentation & Specification License

This file licenses the **written and data artifacts** of the Enqpy™ project — the
formal proof, the Formal Cryptographic Description (FCD), the published test
vectors, and the plain-language guides. It exists because these documents are the
public scientific record and the blueprint for rebuilding the cipher, and they
must be freely copyable and re-hostable so the work survives independent of any
single host, company, or person.

It is **separate** from, and does not modify, the project's other two instruments:

- **Patents → `COVENANT.md`** (the Open-Infrastructure Patent Non-Assertion
  Covenant), which frees the cipher *invention* for anyone to implement and
  deploy.
- **Reference source code → `LICENSE`** (the NQP LLC Public License for Enqpy™,
  Rev 2.0), an evaluation license governing NQP's reference *source code*.

> Not legal advice. This is a license grant prepared by the rights holder; have
> counsel confirm scope before relying on it commercially. NQP LLC is the
> copyright holder making this grant.

---

## 1. Covered works

The following written and data artifacts of the Enqpy™ project (the "Documents"):

- The formal proof paper — ⟨FILL: confirm path, e.g., `papers/enqpy-paper.pdf` and its LaTeX source⟩
- The Formal Cryptographic Description (FCD) — ⟨FILL: confirm path/filename⟩
- The plain-language proof guide and other explanatory documents — ⟨FILL: e.g., `papers/enqpy-plain-language-guide.pdf`⟩
- The published known-answer **test vectors / data** — ⟨FILL: confirm path⟩

This license does **not** cover the reference *source code* (see `LICENSE`), the
patents (see `COVENANT.md`), or the trademarks (reserved; see §4).

---

## 2. Grant — proof, FCD, and guides: Creative Commons Attribution 4.0 (CC BY 4.0)

The proof, the FCD, and the guides are licensed under the **Creative Commons
Attribution 4.0 International License**.

> `SPDX-License-Identifier: CC-BY-4.0`

You may **copy, redistribute, mirror, translate, adapt, excerpt, and archive**
these Documents in any medium or format, for any purpose, including commercially,
**forever** — provided you keep the attribution notice:

> *Enqpy™ — the Shannon Ideal System proof and Formal Cryptographic Description.
> © 2026 Paul McGough / NQP LLC. Licensed CC BY 4.0. Origin: enqpy.com.*

Include the official license text in the repository as `LICENSE-CC-BY-4.0.txt`,
copied verbatim from https://creativecommons.org/licenses/by/4.0/legalcode
(do not retype it).

---

## 3. Grant — test vectors / known-answer data: CC0 1.0 (public domain)

The published test vectors and known-answer data are dedicated to the public
domain under **CC0 1.0 Universal**, so any implementation may bundle and
redistribute them for conformance testing with no attribution friction.

> `SPDX-License-Identifier: CC0-1.0`

Official text: https://creativecommons.org/publicdomain/zero/1.0/legalcode
(include as `LICENSE-CC0-1.0.txt`).

*(Alternative, if you prefer attribution on the vectors too: license them CC BY
4.0 under §2 instead. CC0 is recommended for data so conformance suites can embed
them freely.)*

---

## 4. What this license does NOT cover

- **The cipher invention / patents** — governed by `COVENANT.md`. (You may always
  implement and deploy the cipher, at any scale including commercially, free,
  under the covenant — and the CC-licensed FCD and vectors above are how you do
  it.)
- **NQP's reference source code** — governed by `LICENSE` (NQP LLC Public License,
  Rev 2.0). You may clone, build, run, and study it; redistribution of that
  specific source is restricted there.
- **The trademarks** — Enqpy™, EnqpyADS™, Enqpy™-Certified, and Security,
  Settled.™ are reserved to the Enqpy™ Foundation Inc.; nothing here licenses
  them. Refer to the marks factually; do not imply endorsement or certification.

---

## 5. Why these documents are freed

The proof lets anyone *verify* the result; the FCD and the test vectors let anyone
*rebuild a conforming, patent-safe implementation from scratch*. Freeing them
under CC ensures the cipher can be reconstructed and the science re-hosted by
anyone, forever — even if this repository, NQP LLC, and the Foundation all cease
to exist. It does not touch the reference-code license or the commercial model,
which rest on certification, proprietary high-performance implementations, and
services — not on locking up the public specification.

---

*© 2026 NQP LLC. The proof, FCD, and guides are licensed CC BY 4.0; the test
vectors are dedicated to the public domain under CC0. Read together with
`COVENANT.md` (patents) and `LICENSE` (reference code).*
