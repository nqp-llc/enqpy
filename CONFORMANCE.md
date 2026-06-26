---
title: "ENQPY™ CONFORMANCE SPECIFICATION"
subtitle: "Rev 0.1 framework document — conformance levels, canonical sources, submission process · Enqpy™ Foundation Inc."
author: "Rev 0.1 · June 2026"
---

# Purpose

This document defines what "conformance" means for implementations of
the Enqpy™ stream cipher, and the process by which the Enqpy™ Foundation
Inc. (the "Foundation") recognizes and certifies third-party
implementations.

Conformance serves three audiences:

1. **Community-port authors** who want to publish a Rust, Go, Python,
   Java, or other-language implementation of Enqpy™ and have it
   recognized as a faithful implementation of the cipher.
2. **Commercial deployers** (including Partner Program participants
   under NQP LLC's commercial framework) who need a clear technical
   baseline for what they're deploying.
3. **Independent reviewers** — cryptographers, security researchers,
   and standards bodies — who need a published criterion against which
   to evaluate an implementation's correctness.

This is **Rev 0.1**, the launch-day framework document. **Rev 1.0**,
which includes the full conformance test harness and full Enqpy-Certified
process, is scheduled for publication in Months 4–6 post-launch per the
Post-Launch Playbook §6.3.

# Document status and scope

| Item | Status |
|---|---|
| Document revision | Rev 0.1 (framework document, launch baseline) |
| Effective date | June 1, 2026 |
| Next revision | Rev 0.2 (post-launch refinement, Months 2–3) |
| Rev 1.0 target | Months 4–6 post-launch (full conformance test harness) |
| Authoritative source | enqpy.com/foundation.html and github.com/nqp-llc/enqpy |
| Steward | Enqpy™ Foundation Inc. (Conformance Program) |

**What this document is:** A framework establishing conformance levels,
canonical sources, and the high-level submission process. Sufficient
for the launch-day reference set in the LICENSE file, the Contribute
page, and the FAQ.

**What this document is not:** The full conformance test harness, the
side-channel-resistance requirements, the hardware-conformance criteria,
or the formal Enqpy-Certified Conformance Certificate template. Those
arrive with Rev 1.0.

---

# 1. Canonical sources

A conformant Enqpy implementation must agree, in observable behavior,
with the canonical sources defined below.

## 1.1 Canonical software reference

**The Enqpy™ C reference implementation** at
`github.com/nqp-llc/enqpy` (the "**C Reference**") is the canonical
software implementation. The C Reference is:

- **Single-file C11.** No external dependencies; portable across 8-bit
  microcontrollers to 64-bit servers.
- **Test-anchored.** Ships with 84 self-test assertions against published
  test vectors.
- **Apache-2.0 licensed.** Source-code rights are granted under the
  Apache License 2.0 (see the LICENSE and NOTICE files). Patent rights
  in the cipher are addressed by the Open-Infrastructure Patent
  Non-Assertion Covenant (https://enqpy.com/covenant), under which the
  cipher is patent-safe for all conforming use.

The C Reference is the source against which **algorithmic correctness**
is measured. Any conformant implementation must, given the same inputs,
produce the same outputs as the C Reference.

## 1.2 Canonical hardware reference

**The Enqpy™ VHDL reference implementation** (the "**VHDL Reference**")
is the canonical hardware implementation, licensed by NQP LLC for
hardware deployment. The VHDL Reference is:

- The authoritative source for hardware integrators (FPGA, ASIC, secure
  element).
- Available under commercial license through NQP LLC.
- Synchronized in algorithmic behavior with the C Reference (every
  test vector that passes against the C Reference passes against the
  VHDL Reference).

Hardware implementations claim conformance against the VHDL Reference
in addition to (or in lieu of) the C Reference, depending on deployment
context.

## 1.3 Formal Cryptographic Description

**The Formal Cryptographic Description (FCD)** is the algorithmic
specification of the Enqpy cipher: phase definitions, state-transition
rules, key-schedule semantics, and the algebraic relationships that
govern the cipher's behavior. The FCD is the **prose-and-mathematics
companion** to the C Reference — it tells an implementer *what* the
cipher does, while the C Reference shows *how* one specific
implementation does it.

The FCD is published as [`FCD.md`](FCD.md) at the root of
`github.com/nqp-llc/enqpy` and is incorporated by reference into this
Conformance Specification.

## 1.4 Test vectors

**The published Enqpy test vectors** (the "**Test Vectors**") are
the deterministic input/output triples (key, plaintext, expected
ciphertext) and their reverse forms (key, ciphertext, expected
plaintext) against which implementations are validated.

The launch-day Test Vectors are the 84 input/output triples consumed
by the Enqpy C Reference's self-test harness. Rev 1.0 of this Conformance
Specification will extend the Test Vector set substantially.

Test Vectors are published at `github.com/nqp-llc/enqpy` under
`tests/vectors/` (or equivalent path) and are versioned alongside the
C Reference releases.

## 1.5 The formal proof paper

**Ciphertext-Only Plaintext Equivocation in a Finite-Key MOD16 Record Cipher, with Exact Known-Plaintext Bounds** (the "**Paper**") is the formal
mathematical reference. The Paper establishes the non-vanishing ciphertext-only
plaintext-equivocation property (motivated by Shannon's Ideal-System target), the
supporting key-axis floor, the known-plaintext boundary, and the
independent-record-credential deployment profile; it
is the foundation on which conformance is built but is not itself a test
instrument. Implementations are not "conformant to the Paper" — they are
conformant to the C Reference and FCD, which together implement what the Paper
proves.

---

# 2. Conformance levels

The Foundation recognizes three conformance levels, ordered by
increasing rigor.

## 2.1 Reference-Compatible

**Definition.** An implementation is *Reference-Compatible* if it
passes the published Test Vectors. Specifically, given each Test
Vector's input, the implementation produces output that exactly matches
the C Reference's output.

**Verification.** Self-attested by the implementer, against the
public Test Vector set. No Foundation review required.

**Permitted use.**
- The implementer may state "Reference-Compatible with the Enqpy™
  reference implementation."
- The implementer may NOT use the **Enqpy™** trademark in marketing,
  branding, or product naming. Trademark use requires Enqpy-Certified
  status (§2.3 below) or a Partner Program license from NQP LLC.

**Operational purpose.** This is the entry-level claim for any
community-language port (Rust, Go, Python, Java, etc.) at the moment
the port author has run the Test Vectors successfully against their
implementation. It signals technical correctness without invoking the
Foundation review process.

## 2.2 Conformant Implementation

**Definition.** An implementation is a *Conformant Implementation* if
it is Reference-Compatible AND its algorithm has been walked through
by an independent reviewer against the FCD with no semantic divergence
identified.

**Verification.** Foundation-coordinated review. The implementer
submits the implementation source, the test results, and an
implementation note describing any architectural choices. The
Foundation arranges an independent technical reviewer (initially from
the technical-review lane on the Contribute page; over time, from the
Foundation's reviewer roster).

**Permitted use.**
- The implementer may state "Conformant Implementation of the Enqpy™
  cipher" with appropriate Foundation attribution.
- The implementer may NOT use the **Enqpy™** trademark in product
  branding without an additional Partner Program license.
- The implementer may reference the Foundation's review in
  publications, technical reports, and academic citations.

**Operational purpose.** This is the level at which a community-language
port becomes recommended for non-trivial use. A Conformant
Implementation has been read by someone other than its author and
verified to faithfully implement the cipher as described in the FCD.

## 2.3 Enqpy-Certified

**Definition.** An implementation is *Enqpy-Certified* if it is a
Conformant Implementation AND has been issued a written Conformance
Certificate by the Foundation, with the corresponding trademark
license granted under the Foundation–NQP IP License Agreement
(Article 3, Patent Sublicensing to Certified Implementers).

**Verification.** Foundation Conformance Program review, which
includes:

1. Reference-Compatibility verification (all current Test Vectors pass)
2. FCD walkthrough by Foundation-designated reviewer
3. Algorithmic correctness assessment against the C Reference
4. Implementation-specific review (memory handling, error states,
   API soundness)
5. Performance review (does the implementation meet baseline
   throughput characteristics)
6. Side-channel resistance review (timing, power, EM — at the level
   specified for the implementation's deployment context)
7. Formal Conformance Certificate issuance

**Permitted use.**
- The implementer may use the **Enqpy™** and **Enqpy-Certified**
  trademarks under the terms of the Conformance Certificate.
- The implementer may state "Enqpy-Certified Implementation, Conformance
  Certificate No. [X], issued by the Enqpy™ Foundation Inc."
- The implementer receives the Foundation's certification mark for use
  in product documentation, marketing, and packaging.

**Operational purpose.** This is the level required for:

- Commercial deployments under NQP LLC's Partner Program.
- Government, defense, and critical-infrastructure deployments where
  formal third-party validation of the cipher implementation is
  required.
- Public claims that a product "uses Enqpy" — these require either
  Enqpy-Certified status or an alternative Partner Program license.

**Note on Rev 0.1 scope.** The full Enqpy-Certified review process —
including the performance-review and side-channel-resistance review
components — will be defined in Rev 1.0 (Months 4–6 post-launch). At
launch (Rev 0.1), the Foundation does not issue Enqpy-Certified
certificates; the first Enqpy-Certified issuance is anticipated for
Months 9–12 post-launch (per Post-Launch Playbook §6.1).

---

# 3. Submission process for community implementations

Community-language ports (Rust, Go, Python, Java, etc.) and independent
implementations follow the submission process below. The process is
deliberately lightweight at Rev 0.1; Rev 1.0 will formalize it.

## 3.1 Self-attestation (Reference-Compatible)

To claim Reference-Compatible status, the implementer:

1. Runs the published Test Vectors against the implementation.
2. Confirms 100% pass rate.
3. May publish the implementation (e.g., on GitHub, crates.io, npm,
   PyPI) with the statement *"Reference-Compatible with the Enqpy™
   reference implementation."*

No Foundation submission required. No fee. The Foundation may at any
time review a public Reference-Compatible claim and request the
implementer correct or retract it if the test vectors do not pass.

## 3.2 Submission for Conformant Implementation review

To request Conformant Implementation status, the implementer submits
to the Foundation at `RPM@enqpy.com` (Rev 0.1 contact path; Rev 1.0
will provide a dedicated conformance submission email and a public
issue tracker):

1. **Implementation source** (or a stable public URL).
2. **Test result log** demonstrating Reference-Compatible status.
3. **Implementation note** (1–3 pages) describing:
   - Language, runtime environment, deployment target
   - Architectural choices (memory model, integer types,
     concurrency posture, error handling)
   - Any divergences from the C Reference (e.g., different data
     structures used internally — algorithmic outputs must still match)
   - Maintenance and contact information
4. **License declaration** for the implementation itself (it does not
   need to match the C Reference's license, but it must be one under
   which the Foundation can review it).

The Foundation acknowledges submission within 14 days and provides an
estimated review timeline. Initial reviews are expected to take 30–90
days at Rev 0.1.

## 3.3 Submission for Enqpy-Certified status

**Available beginning Months 9–12 post-launch.** Process to be defined
in Conformance Specification Rev 1.0. Prerequisites known at Rev 0.1:

1. Prior Conformant Implementation review.
2. Engagement with NQP LLC's Partner Program (since Enqpy-Certified
   status entails trademark licensing).
3. Performance and side-channel review per the Rev 1.0 process.
4. Conformance Certificate issuance and trademark sublicense execution.

---

# 4. Test Vector authority

The published Test Vectors are the operational authority for
Reference-Compatibility claims. The following rules govern Test Vectors
at Rev 0.1:

| Rule | Detail |
|---|---|
| **Source of truth** | `github.com/nqp-llc/enqpy` — the Test Vectors committed to the repository at the time of an implementation's claimed Reference-Compatibility |
| **Versioning** | Test Vectors are versioned alongside C Reference releases (semantic versioning: e.g., Test Vectors v5.0 ship with C Reference Rev 5.0; the canonical KATs are unchanged since Rev 3.0) |
| **Backward compatibility** | New Test Vectors may be added; existing Test Vectors will not be changed except to correct demonstrated errors |
| **Validation** | An implementation must pass the Test Vectors version current at the time of its claim |
| **Extension** | Rev 1.0 will add Test Vector coverage for boundary conditions, error states, and cryptographic edge cases identified during the first months of community implementation work |

---

# 5. What this Rev 0.1 does NOT cover

To set clear expectations, the following are out of scope for Rev 0.1
and will be addressed in Rev 1.0 (Months 4–6 post-launch):

| Out of scope at Rev 0.1 | Addressed in Rev 1.0 |
|---|---|
| Full conformance test harness (beyond the 84-assertion baseline) | ☑ |
| Side-channel resistance requirements (timing, power, EM) | ☑ |
| Hardware-specific conformance criteria (FPGA, ASIC, secure element) | ☑ |
| Performance benchmarking requirements | ☑ |
| The formal Conformance Certificate template | ☑ |
| The Foundation reviewer roster and reviewer-qualification criteria | ☑ |
| Conformance review fees (if any) | ☑ |
| Multi-year revalidation requirements for Enqpy-Certified implementations | ☑ |
| Process for revoking Enqpy-Certified status | ☑ |
| Internationalization of the conformance process (non-English-language submissions) | ☑ |

**Implementers building against Rev 0.1** should expect that their
Conformant Implementation review may require additional submissions
when Rev 1.0 publishes; the Foundation will provide a clear migration
path and will not retroactively invalidate Rev 0.1 reviews.

---

# 6. Roadmap

| Revision | Target date | Scope |
|---|---|---|
| **Rev 0.1** | June 1, 2026 (launch) | This document — framework, levels, submission process |
| **Rev 0.2** | Months 2–3 post-launch | Refinement based on first community-port submissions; clarifications and corrections |
| **Rev 1.0** | Months 4–6 post-launch | Full conformance test harness, performance review, side-channel resistance, Enqpy-Certified process |
| **Rev 1.x** | Annual | Additions, clarifications, deprecation of obsolete criteria |
| **Rev 2.0** | Year 3+ | Major revision if cipher specification evolves; alignment with standards-body work (IETF, ISO/IEC, ETSI) |

---

# 7. Governance and contact

This Conformance Specification is published by the Enqpy™ Foundation
Inc. as part of the public technical record. Substantive revisions
require Foundation board approval.

Operational questions, submission inquiries, and review requests:

- **Email:** RPM@enqpy.com (Rev 0.1 contact path)
- **Issue tracker:** github.com/nqp-llc/enqpy/issues (use the
  Conformance / Port issue template)
- **Foundation page:** enqpy.com/foundation.html

---

# Appendix A — Quick reference

**For a community port author who wants to publish a Rust implementation:**

1. Build the implementation against the FCD.
2. Run the Test Vectors. Confirm 100% pass.
3. Publish with the statement *"Reference-Compatible with the Enqpy™
   reference implementation."*
4. (Optional) Submit to the Foundation for Conformant Implementation
   review per §3.2.
5. (Future) Pursue Enqpy-Certified status per §3.3 when Rev 1.0
   process is available.

**For a commercial deployer evaluating an implementation:**

1. Check the implementation's conformance level (Reference-Compatible /
   Conformant / Enqpy-Certified).
2. For non-critical deployments: Reference-Compatible is acceptable
   provided you've done your own due diligence.
3. For mission-critical deployments: require Conformant Implementation
   or Enqpy-Certified status.
4. For commercial product branding using the Enqpy™ trademark: require
   Enqpy-Certified status or a Partner Program license from NQP LLC.

**For an independent technical reviewer:**

1. The FCD is the authoritative algorithmic specification.
2. The C Reference is the authoritative software baseline.
3. The Test Vectors are the conformance criterion.
4. Reviews may be submitted to the Foundation through the
   technical-review contribution lane (enqpy.com/contribute.html).

---

*Rev 0.1 · Enqpy™ Foundation Inc. · Effective June 1, 2026 ·
Companion to the formal proof paper, the C Reference, the VHDL
Reference, and the Formal Cryptographic Description ·
Trademark "Enqpy™" is owned by the Enqpy™ Foundation Inc. ·
Patent rights are covered by the Open-Infrastructure Patent Non-Assertion
Covenant (https://enqpy.com/covenant)*
