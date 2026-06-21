# Enqpy™ Rev 4.0 — Reference Core + Deployable HKU Wrapper

Two layers, matching the Rev 4.0 documents (FCD §8.10.1, proof §17.9 / Proposition 2):

| File | Role |
|---|---|
| `enqpy_reference.c` | **Core / proof artifact.** The Canonical Configuration (ciphertext-only, single-key-epoch) for which the Rev 4.0 theorems are proved. It *may* expose Phase 5 multi-window continuation — that is a core/research operation, not a deployment operation. **Left byte-identical; its 84/84 self-test is unchanged.** |
| `enqpy_hku.h` / `enqpy_hku.c` | **Deployable API.** Enforces the single-window credential rule in code: it **refuses** a single segment longer than one window (2,048 B at HIGH) and **segments** long messages into independent single-window messages, each under a fresh independent `(EK_j, QK_j)`, its own nonce, and its own encrypt-then-MAC tag. It never calls the core in a way that triggers Phase 5. |

## Why two layers

The core allows Phase 5 to continue a message past one window under the same
credential. Under known plaintext that continuation is the cross-window
propagation path (FCD §8.5 / proof §16). The deployable wrapper removes the
continuing key state entirely: **one credential, one window, one segment, one
MAC.** Cross-segment non-accumulation then follows from credential independence
(Proposition 2) and needs no reset lemma. Neither layer saves the window in
which known plaintext lands — within-window determinability is structural;
segmentation only *confines* an exposure to its own segment.

## Build

```sh
# core's own KAT (must print: Self-test: 84 PASS  0 FAIL)
cc -O2 -DENQPY_SELFTEST enqpy_reference.c -o core_selftest && ./core_selftest

# deployable wrapper + its self-test (core compiled WITHOUT the selftest/bench
# defines, so it exposes no main(); the wrapper supplies one)
cc -O2 -c enqpy_reference.c -o enqpy_reference.o
cc -O2 -Wall -Wextra -DENQPY_HKU_DEMO enqpy_hku.c enqpy_reference.o -o enqpy_hku_demo
./enqpy_hku_demo            # 8 passed, 0 failed
```

To link the wrapper into an application, compile `enqpy_reference.c` (no
`ENQPY_SELFTEST`/`ENQPY_BENCHMARK`) and `enqpy_hku.c` (no `ENQPY_HKU_DEMO`)
together, and call the `enqpy_hku_*` API from `enqpy_hku.h`.

## Using the wrapper

- **`enqpy_hku_seal_segment` / `enqpy_hku_open_segment`** — the guarded
  primitive. One segment ≤ one window, one credential, encrypt-then-MAC.
  Returns `ENQPY_HKU_ERR_TOO_LONG` if asked to exceed a window.
- **`enqpy_hku_seal_message` / `enqpy_hku_open_message`** — the recommended
  default. Segments a long plaintext and pulls a fresh independent credential
  per segment from a caller-supplied `enqpy_hku_cred_fn`.

### Credential independence is the security-bearing requirement

The caller's `cred_fn` **must** draw each segment's `(EK_j, QK_j)` from an
independent CSPRNG (or an out-of-band one-time store). It must **not** be a key
chain, counter schedule, or KDF/PRG expansion of a single seed — a deterministic
relation between segment credentials relocates the §8.5 propagation to the key
layer. The receiver's `cred_fn` supplies the matching out-of-band `(EK_j, QK_j)`;
the public nonce travels in the frame.

> The `SplitMix64` generator in the demo is **TEST-ONLY**, for a reproducible
> self-test. It MUST NOT be used to key real traffic.

## Encrypt-then-MAC

Per FCD §8.6: `IK = HKDF-SHA-256(IKM = EK_bytes, salt = nonce_bytes,
info = "EnqpyINTEGRITY")`, `Tag = HMAC-SHA-256(IK, nonce_bytes ‖ ciphertext ‖
metadata)`, verified constant-time before any plaintext is released. The
per-segment metadata binds segment index, total segments, and segment length,
so reordering, truncation, and splicing are detected (both are exercised in the
self-test). `IK` is a per-segment secret, zeroized after the tag.

SHA-256 / HMAC / HKDF are bundled in `enqpy_hku.c` so the wrapper has no
external dependency. (`nonce_bytes` is the transmitted public `OR` field rather
than the cipher-internal `eff_or`, so the core stays untouched; both bind the
per-message nonce.)
