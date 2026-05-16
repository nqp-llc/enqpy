# Contributing to Enqpy™

**Enqpy™ Reference Implementation · github.com/nqp-llc/enqpy**

Effective: June 1, 2026

---

Thank you for your interest in contributing. This document explains
**how** to contribute. Before you start, please read
[`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md) — it explains **what** kinds
of contributions are accepted and the project's structural posture as
a reference implementation that must remain in exact correspondence
with the formal proof.

## Quick summary

- Documentation, build fixes, portability, additional test vectors, and
  language ports — **welcome**.
- Modifications to the core cryptographic algorithm — **not accepted**.
  The implementation must match the proof.
- For suspected security or cryptanalytic findings — **do not file a
  public issue**; see [`SECURITY.md`](SECURITY.md).
- For substantive technical questions or framing discussions — open an
  issue. They are welcomed.

## How to contribute

### 1. Open an issue first (for anything non-trivial)

For changes larger than a typo fix or a one-line correction, please
open an issue first to discuss the proposed change. This avoids work
on contributions that, on reflection, fall outside the accepted scope.
Maintainers acknowledge new issues within fourteen days; substantive
discussion follows from there.

### 2. Fork and branch

Fork the repository to your own GitHub account, then create a topic
branch from `main`:

```
git checkout -b your-topic-branch
```

Use a short, descriptive branch name (`fix-build-arm`, `docs-spec-typo`,
`test-vector-additions`).

### 3. Make focused changes

Each pull request should address one logical change. If you find
yourself wanting to combine unrelated fixes, split them into separate
pull requests instead. Smaller, focused changes are reviewed faster
and merged more reliably.

### 4. Match the existing style

For C code, match the formatting, naming, and structural conventions
of the existing reference implementation. The single-file C11
implementation is the canonical style. If a formatting tool or
configuration file is present in the repository, use it.

For documentation, match the Markdown conventions of the existing
files (sentence-case headings, 80-character soft wrap where practical,
fenced code blocks with language tags).

### 5. Run the tests

The reference implementation includes self-test assertions against the
published vectors. Run them locally before submitting. See the
repository `README` for build and test instructions on your platform.
All tests must pass before a pull request will be reviewed.

If your change adds functionality, add or update tests that cover it.
If your change adds test vectors, include the source and rationale
for the new vectors and demonstrate that they are consistent with the
canonical specification.

### 6. Sign your commits (DCO)

This project requires the **Developer Certificate of Origin (DCO)**
sign-off on every commit in your pull request. The DCO is a simple
attestation that you have the right to submit the contribution under
the project's license. It is added by running:

```
git commit -s -m "your commit message"
```

This appends a line to your commit message:

```
Signed-off-by: Your Name <your.email@example.com>
```

The name and email must match a real identity you can be reached at.
Pull requests with unsigned commits will be asked to amend before
review.

The full DCO text is available at <https://developercertificate.org/>.
By signing your commits, you certify the four DCO clauses for that
contribution.

### 7. Open a pull request

When your branch is ready, open a pull request against `main`. In the
PR description:

- **What** the change does (one or two sentences)
- **Why** the change is being made (issue number if applicable)
- **How** the change was tested (which test targets, which platforms)
- **Scope** confirmation: this PR does not modify the core algorithm
  (or, if it does, why — see the next section)

### 8. Engage in review

Maintainers will respond to pull requests within fourteen days,
typically sooner. Review may include requests for changes,
clarifications, or test additions. The bar for merge is correctness,
consistency with the canonical specification, clear motivation, and
adherence to the project's contribution scope.

Not every well-intentioned PR is merged. Maintainers may close a PR
with a reasoned explanation when the change falls outside accepted
scope, conflicts with the formal proof, or introduces risk that
outweighs the benefit.

## Special cases

### Changes that would affect the cryptographic algorithm

If you believe a change to the core algorithm is necessary — for
example, because you have identified an error in the formal proof, an
inconsistency between the proof and the reference implementation, or
a deployment-blocking limitation — **do not file this as a routine
pull request**. The appropriate path is:

1. For a suspected error in the proof or a cryptanalytic finding,
   follow [`SECURITY.md`](SECURITY.md) for coordinated disclosure.
2. For a suspected proof–implementation inconsistency, open an issue
   labeled clearly as such; maintainers will coordinate with the
   inventor and the Enqpy™ Foundation on appropriate handling.

Any change accepted into the canonical algorithm requires update of
the formal proof, the test vectors, the paper, and the conformance
specification in coordinated fashion.

### Language ports and wrappers

We welcome implementations of Enqpy™ in other languages (Rust, Go,
Python, Java, JavaScript, etc.), but in most cases these are best
maintained in separate repositories rather than as additions to this
canonical C reference. If you are working on a port:

- For coordination, open an issue or email RPM@enqpy.com with
  `[Port]` in the subject line.
- For trademark use, see [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md) —
  forks may state "based on the Enqpy™ reference implementation" but
  may not be presented as the canonical implementation, as
  Enqpy™-Certified, or as Enqpy™-Compatible without Foundation
  certification and trademark licensing.

The Foundation conformance program (Year 1 deliverable) will provide a
formal path for non-C implementations to be certified against the
specification.

## Patience expectation

This is an inventor-stewarded project with a small maintainer team
through the launch period. Response times will be reasonable but not
instantaneous. Substantive contributions are read and considered with
care; not every contribution will result in a merge, and that is by
design.

## Where to ask questions

- General questions about contributing: open a GitHub discussion or
  email RPM@enqpy.com with `[General]` in the subject line.
- Technical questions about the cipher, the proof, or the reference
  implementation: file a GitHub issue or email RPM@enqpy.com with
  `[Technical]` in the subject line.
- Security and vulnerability reports: see [`SECURITY.md`](SECURITY.md).
- Code of conduct violations: email RPM@enqpy.com with `[Conduct]` in
  the subject line.

---

*NQP LLC · Enqpy™ Foundation Inc. · Effective June 1, 2026*

*Thank you for considering a contribution.*
