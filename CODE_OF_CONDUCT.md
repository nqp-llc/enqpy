# Code of Conduct and Acceptable Use Policy

**Enqpy™ Reference Implementation · github.com/nqp-llc/enqpy**

Effective: June 1, 2026

---

## What this repository is

The Enqpy™ GitHub repository hosts the reference implementation of the
Enqpy™ stream cipher, supporting documentation, test vectors, and related
materials. The reference implementation accompanies the formal proof paper
that establishes the cipher's non-vanishing ciphertext-only key-equivocation
property (motivated by Shannon's Ideal-System target). The
implementation is the canonical, authoritative realization of the proved
algorithm.

**This is not a community-developed project.** The reference implementation
must remain in exact correspondence with the formal proof; the algorithm
itself is closed to community modification. Documentation improvements,
build and portability fixes, test additions, and similar contributions are
welcomed; modifications to the core cryptographic algorithm are not.

This document defines acceptable conduct in issues, discussions, and
pull requests on this repository, the contribution posture, the path for
vulnerability disclosure, and the boundaries on trademark and derivative
use.

## Who this applies to

Everyone interacting with the repository: visitors, readers, contributors,
issue authors, pull-request authors, commenters, maintainers, and anyone
communicating about the project through the repository's interfaces.

## Acceptable behavior

We expect everyone to:

- **Communicate substantively.** Issues, discussions, and PR comments
  should be technically grounded and specific. Vague complaints,
  drive-by criticism, or rhetorical posturing without substance will be
  closed without extended engagement.
- **Communicate civilly.** Disagree freely about technical content;
  treat people as people. No personal attacks, no harassment, no
  discrimination on the basis of identity.
- **Stay on topic.** This is a cryptographic-implementation repository.
  Discussions outside the scope of the cipher, its proof, its
  implementation, or its use are not appropriate here.
- **Respect cryptographic-research norms.** Substantive cryptanalytic
  claims, suspected vulnerabilities, or suspected proof errors go
  through `SECURITY.md` for coordinated disclosure first. See
  "Cryptanalytic claims and findings" below.

## Unacceptable behavior

- Personal attacks, threats, harassment, or intimidation in any form
- Discrimination, slurs, or demeaning language on any basis
- Doxxing or revealing private information about any person
- Promotion of unrelated products, services, or cryptographic schemes
- Automated scraping or interaction that imposes unreasonable load
- Impersonation of NQP LLC, the Enqpy™ Foundation, Paul McGough, or any
  contributor
- Use of the repository to distribute malware, illegal content, or
  content unrelated to the project
- Off-topic political, ideological, or commercial advocacy

## Contribution posture

The Enqpy™ reference implementation is not community-developed. This is
a deliberate, structural choice: the implementation must correspond
exactly to the algorithm as proved in the formal proof paper. Changes to
the core algorithm by anyone other than the inventor and the
Enqpy™ Foundation maintainers would break that correspondence.

**Pull requests welcome:**

- Documentation improvements and corrections
- Build, portability, and platform-support improvements
- Additional test vectors (cross-checked against the canonical vectors)
- Wrapper, binding, and language-port suggestions or scaffolding
- Performance benchmarking on new platforms
- Typo and formatting corrections

**Pull requests not accepted:**

- Modifications to the core cryptographic algorithm
- Changes to the proved primitive constructions (PDAF, OWC)
- Forks held out as "improvements" or "alternatives" to the canonical
  implementation that retain Enqpy™ branding

Maintainers will acknowledge pull requests within a reasonable time,
typically within fourteen days. Acceptance is at maintainer discretion.
Submissions accepted into the repository are licensed under the project's
posted license terms; by submitting, you warrant that you have the right
to grant that license, and that the contribution is your own work or is
licensed compatibly with the repository's terms.

## Cryptanalytic claims and findings

This is a cryptographic project. We expect and welcome cryptanalytic
review of the proof, the implementation, and the test vectors. We follow
standard coordinated-disclosure norms:

- **Substantive cryptanalytic claims, suspected vulnerabilities, or
  suspected proof errors:** do not post publicly first. Email the path
  in `SECURITY.md` (typically RPM@enqpy.com with `[Security]` in the
  subject line). We commit to acknowledging substantive reports within
  five business days and to engaging in good-faith coordinated
  disclosure on a reasonable timeline (typically 90 days, or as
  mutually agreed).
- **Public claims without prior contact** will not be substantively
  engaged in the issues or discussions on this repository. We will
  close such threads with a pointer to `SECURITY.md` and to the formal
  proof paper for the technical record.
- **Theoretical objections, framing questions, and
  clarifying-the-claim discussions** are appropriate as issues and
  discussions and are welcomed. The line is the difference between
  "is the proof clear about X?" (welcome here) and "I have found a
  break" (goes through `SECURITY.md`).

## Trademark use

The Enqpy™ mark is a trademark of Enqpy™ Foundation Inc.,
and Enqpy™-Certified is a certification mark of the Foundation.
"Enqpy™-Compatible" is a conformance designation granted only through the
Foundation conformance program. Their use in derivative work is governed by
the Foundation Trademark Use Guidelines.

For repository-related purposes specifically:

- You **may** state that a fork or derivative work "is based on the
  Enqpy™ reference implementation" with appropriate attribution and a
  link to this repository.
- You **may not** present a fork or derivative as *the* Enqpy™
  implementation, as Enqpy™-Certified, or as Enqpy™-Compatible without
  separate Foundation conformance certification and trademark licensing.
- You **may not** use the marks in a way that suggests Foundation or
  NQP endorsement of your work that has not been granted.

## Vulnerability and security reporting

See `SECURITY.md` in this repository for the formal security and
vulnerability disclosure path, expected response times, and coordinated-
disclosure norms.

## Enforcement

Repository maintainers are responsible for enforcing this Code of Conduct
and Acceptable Use Policy. Enforcement actions include, depending on
severity and context:

- Closing issues or pull requests without further engagement
- Locking discussions or comment threads
- Editing or removing comments that violate this policy (with a brief
  note explaining what was changed)
- Temporary blocking of accounts from the repository
- Permanent blocking of accounts from the repository

There is no appeal mechanism beyond the maintainers. This is an
inventor-stewarded project, not a community-governed one. We will
exercise enforcement authority sparingly, in good faith, and with a
preference for substantive engagement where it is possible.

## Reporting violations

To report a violation of this policy, email **RPM@enqpy.com** with
`[Conduct]` in the subject line. Provide:

- The location of the conduct (issue URL, comment URL, etc.)
- A description of the conduct
- Any other context you consider relevant

Reports are read by Paul McGough or an appointed maintainer. We
acknowledge serious reports within five business days.

## Changes to this policy

This Code of Conduct and Acceptable Use Policy may be updated over
time. The effective date at the top of this document will be updated
when changes are made. Material changes will also be noted in the
repository's release notes or commit log.

## Acknowledgment

This document draws on conventions from open-source community
standards (including the spirit of the Contributor Covenant 2.1) while
incorporating constraints specific to a reference cryptographic
implementation. It is licensed for free use and adaptation under
Creative Commons Attribution 4.0 International (CC BY 4.0).

---

*NQP LLC · Enqpy™ Foundation Inc. · Effective June 1, 2026*
