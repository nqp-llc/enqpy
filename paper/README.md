# The Enqpy™ Paper

This folder is reserved for the IACR ePrint paper that underlies this
reference implementation:

> **Enqpy™ Stream Cipher: Constructive Proof of Shannon's Ideal System
> for a Finite-Key Cipher** — Paul McGough, NQP LLC, 2026.

Once published on IACR ePrint, the PDF will be mirrored here and the link
added below.

---

## Where to read the paper

- **IACR ePrint:** link forthcoming
- **Project site:** [enqpy.com](https://enqpy.com)
- **Acquisition inquiries:** [RPM@enqpy.com](mailto:RPM@enqpy.com)

---

## What the paper establishes

The paper proves, constructively and unconditionally, that the Enqpy™
PDAF_SEC cipher is the first finite-key cipher formally shown to satisfy
Shannon's Ideal System property, as defined in Shannon's 1949
"Communication Theory of Secrecy Systems."

### Principal results

| Result | Statement | Location |
|---|---|---|
| **Theorem 1** | PDAF Mode 1 inversion lower bound: \|P(O\*)\| ≥ 2 (worst case) or ≥ 16 (non-degenerate). Algebraic and unconditional. | §5, Appendix B |
| **Theorem 2** | Shannon Ideal System: H(EK, QK \| T^∞) = log₂(4) = 2 bits exact (Ideal Configuration); T_{>t} ⊥ (EK, QK) \| T_{≤t} (exact conditional independence). | §6, Appendix A |
| **Corollary 2** | All bounds invariant under quantum computation. No computational hardness assumption is invoked anywhere in the proof. | §7 |
| **Theorem 3** | Plaintext equivocation: H(PT \| CT, OR) ≥ 128 bits for HIGH profile (n=64); \|S(CT, OR)\| ≥ 2^128 consistent plaintexts for any ciphertext; posterior uniform over proved Case 1 achievable subset under uniform key prior. | §12, Appendix C |

The combination of Theorem 2 and Theorem 3 establishes Shannon's "many
alternatives, all of reasonable probability" criterion on **both axes**
(key axis and message axis) simultaneously.

### What the paper is *not*

- Not an IND-CPA / IND-CCA claim. The paper proves a specific
  information-theoretic property distinct from computational security
  notions.
- Not a claim of resistance to active attacks, nonce-misuse, or
  side-channel vulnerabilities. Operational deployment requires the
  additional mechanisms specified in §16 of the paper.

---

## How this repository relates to the paper

This repository is the **canonical C reference implementation** of the
Ideal Enqpy™ Configuration — the configuration under which Theorem 2 is
proved.

The 78 self-test assertions in `src/enqpy_reference.c` verify:

- **PDAF Mode 0** output (matches FCD §13.2 and paper §15)
- **PDAF Mode 1** output (matches FCD §13.2 and paper §15)
- **PDAF_SEC** full-cipher output and round-trip (matches FCD §13.3 and
  paper §15)
- **`[+8]` coset invariants** for both EK and QK axes — the empirical
  confirmation of Remark 1 (paper §6, Step 1) that drives the
  4-element ciphertext-equivalent coset
  {EK, EK+8} × {QK, QK+8} at the heart of the Ideal System result

See [`../tests/README.md`](../tests/README.md) for the complete
assertion-to-proof-claim mapping.

---

## Companion documents

The paper references several companion documents maintained by NQP LLC:

- **Formal Cryptographic Description (FCD), Rev 1.0** — authoritative
  cipher specification. Test vectors in §13, NIST SP 800-22 results in
  §14. Available via direct inquiry.
- **Formal Information-Theoretic Proof (ISP), Rev 1.0** — standalone
  five-layer proof document. Reproduced in full as **Appendix A** of the
  ePrint paper.
- **Open Items Closure (OIC), Rev 1.0** — extended algebraic proofs
  (exact preimage count, cycle case analysis, equivocation tightness).
  Reproduced in full as **Appendix B** of the ePrint paper.
- **Plaintext Equivocation Theorem (PET), Rev 1.0** — the message-axis
  result. Reproduced in full as **Appendix C** of the ePrint paper.
- **Performance Benchmark Report, Rev 1.0** — extended software and
  FPGA benchmarks. Summary in §14 of the paper; full report via direct
  inquiry.

---

*For questions, licensing, or acquisition inquiries:*
*[RPM@enqpy.com](mailto:RPM@enqpy.com)*
