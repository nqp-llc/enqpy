# Enqpy™ Ports & Bindings

Community ports and bindings, each under **its own project name**, described as *"Reference-Compatible with the Enqpy™ reference implementation."* A listing here means the port reproduces the **canonical test vectors** at the stated profile(s) and self-attests to the **Reference-Compatible** level — a technical acknowledgment, not a trademark grant.

The higher levels are separate: **Conformant Implementation** is Foundation-reviewed (and tracked in the Foundation's own registry); **Enqpy™-Certified** carries a Conformance Certificate and trademark license and is the only level that authorizes the Enqpy™ mark in product naming. See [`CONFORMANCE.md`](CONFORMANCE.md) for the levels and [`PORTING.md`](PORTING.md) to add yours.

**Vectors status:** ✅ all canonical vectors pass · 🔄 under review · 🧪 in progress, not yet verified

| Project name | Language / Runtime | Author / Maintainer | Repository | Profiles | Vectors version | Level | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Enqpy C reference | C | NQP LLC | github.com/nqp-llc/enqpy | LOW / MED / HIGH | 5.1 | reference | ✅ |

*Vectors v5.1 ADD four cases (Rev 5.1 key domain, both-axes coset, empty plaintext, window boundary) and edit none in place — every v5.0 value reproduces byte-for-byte. A port that passed v5.0 still passes those vectors, but should re-run against v5.1 before claiming Rev 5.1 conformance: `sec-0002-ekEqualsQk` is the case that catches an implementation still rejecting equal keys. v5.1 also publishes full **LOW (n=32)** and **MEDIUM (n=48)** coverage, so the Profiles column can now be substantiated rather than asserted. As of v5.1 the export is complete: every case the C self-test exercises has a published vector, and `vectors_check.c` asserts all 34 in one pass.*
| — | — | — | — | — | — | — | — |

*No community ports yet — yours could be the first. See [`PORTING.md`](PORTING.md).*
