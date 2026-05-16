# Security Policy

**Enqpy™ Reference Implementation · github.com/nqp-llc/enqpy**

Effective: June 1, 2026

---

## Reporting a vulnerability

Email **RPM@enqpy.com** with `[Security]` in the subject line.

Do not file public GitHub issues for security reports. Public posting
of suspected vulnerabilities, cryptanalytic findings, or proof errors
before coordinated disclosure undermines the disclosure process and
will be redirected to this email path.

For encrypted communication, request our PGP key in your first message
and we will provide it.

## What we commit to

- **Acknowledgment within five business days** of receipt.
- **Substantive response within fourteen days** for in-scope reports —
  either confirmation of the finding, a request for clarification, or
  a reasoned explanation of why the report is out of scope.
- **Coordinated disclosure on a 90-day default window**, or as mutually
  agreed if the finding warrants a longer or shorter timeline.
- **Credit in disclosure publications** where the reporter wishes to be
  credited; anonymous reports are also accepted.
- **No legal action** against good-faith security researchers who follow
  this policy, do not exfiltrate data, and do not disrupt services.

## In scope

Reports concerning:

- **Cryptanalytic findings** against the formal proof — proof errors,
  missed cases, unstated assumptions, or arguments that the proof's
  bounds do not hold as claimed.
- **Implementation bugs in the reference code** — memory safety
  violations, undefined behavior, side-channel leakage observable in
  the canonical implementation, incorrect outputs for documented
  inputs, or behavior inconsistent with the paper and test vectors.
- **Inconsistencies** between the paper, the reference implementation,
  and the published test vectors.
- **Build, packaging, or test-harness issues** that could lead to
  insecure deployment by downstream users following the documented
  instructions.
- **Documentation errors** with security implications.

## Out of scope

- **Theoretical disagreements** with Shannon's framework, with the
  proof's chosen model, or with information-theoretic methodology
  in general. These are welcome as issues or discussions in the
  repository, not as security reports.
- **General questions** about the proof's validity that do not
  identify a specific finding. The IACR ePrint paper is the
  technical record; engagement with it via standard academic
  channels is welcomed and not bottlenecked through this disclosure
  path.
- **Issues with downstream uses** of Enqpy™ in third-party products,
  forks, or implementations that are not the canonical reference
  implementation in this repository.
- **Operational security questions** about key management, protocol
  design, side-channel resistance in specific deployment environments,
  or other surrounding-system concerns. These are best addressed in
  the deploying party's own security review or via separate NQP LLC
  commercial engagement.
- **Social engineering, physical attacks**, or anything that does not
  concern the cipher, the proof, or the reference implementation.

## What to include in a report

If you can include the following, it accelerates triage and response:

- **Affected component** — paper, primitive, reference implementation,
  specific test vector, specific build target.
- **Description of the finding** — what is wrong, with enough detail
  for a competent reviewer to reproduce or verify.
- **Severity assessment** — your view of the impact and exploitability.
- **Reproduction steps** — for implementation bugs, the inputs and
  expected/actual outputs; for proof findings, the relevant section
  and theorem numbers.
- **Suggested fix**, if you have one.
- **Disclosure preferences** — credit, anonymity, embargo timing,
  publication coordination.

## After disclosure

After a coordinated disclosure window has elapsed and the finding has
been addressed:

- Findings will be acknowledged in the repository's release notes for
  the affected version.
- The Enqpy™ Foundation maintains a public log of resolved security
  findings as part of the conformance-program records.
- Reporters who wish to be credited will be credited in the disclosure
  publication; anonymous credit is honored on request.

## Scope outside this policy

This policy concerns the Enqpy™ reference implementation in this
repository and the formal materials it accompanies. NQP LLC commercial
products, custom implementations developed under separate agreements,
and third-party implementations not licensed through the Enqpy™
Foundation conformance program are not covered by this policy.

---

*NQP LLC · Enqpy™ Foundation Inc. · Effective June 1, 2026*

*Questions about this policy that are not themselves security reports:
email RPM@enqpy.com with `[General]` in the subject line.*
