# Enqpy™ ESE — optional KPA-hardening reference (Rev 5.1)

`C = S(P + W1) + W2`  (+ = per-nibble mod-16 addition over ℤ₁₆)

Base Enqpy's record encryption is byte XOR; ESE's two masking steps use mod-16
addition — the ℤ₁₆ structure the obstruction analysis rests on. ESE composes
separately derived Enqpy keystreams around a secret permutation; it does not change
base Enqpy's operation.

This is the **optional** known-plaintext (KP) hardening layer for Enqpy. It is not
part of the base cipher and not required for the base cipher's proven guarantee.
It exists for deployments where a single key epoch also encrypts attacker-known
plaintext, and you want to raise the bar against that case in addition to the base
cipher's ciphertext-only proof.

## What it is

The base Enqpy cipher (`enqpy_reference.c`) has a proof about **ciphertext-only**
plaintext equivocation in the stated model: from ciphertext alone at least 2^128
plaintexts remain mathematically consistent at HIGH for any quantity of
ciphertext, and under the uniform key and plaintext priors of Theorem 3 that
support bound closes to at least 128 bits of min-entropy. The support bound is
unconditional; the min-entropy statement is prior-dependent. Neither rests on
any computational-hardness assumption. Known plaintext at
one key epoch is outside that proof and is handled operationally by the
per-record credential rule of FCD 8.10 (`nil_comm` Method 2 is the
continuing-key-state rotation mechanism, which the per-record profile does not
rely on).

The ESE layer adds a second line of defense at the cipher layer:

- `W1`, `W2` — two **separately derived** Enqpy keystreams (`PDAF_SEC` under
  distinct, domain-separated per-record credentials). *"Independent" is reserved in
  the Rev 5.1 corpus for the statistical claim of the information-theoretic
  key-supply profile (FCD §8.10); under the computational profile these keystreams
  are computationally, not statistically, separated.*
- `S` — a **secret, per-record-fresh** byte permutation of the window, applied
  between the two masks. The inner mask `W1` plus the secret `S` is what raises the
  KP threshold.

Byte/nibble handling: plaintext bytes are read as two nibbles; `W1` and `W2` are
applied nibblewise; `S` permutes byte positions, carrying both nibbles of each byte
together; ciphertext is re-packed as bytes.

## Canonical `S` = keyed butterfly / Benes network

`S` is a keyed butterfly (Benes-class) conditional-exchange network: `PASSES`
passes × `LOGN` stages, the switch bit of every pair taken from a third independent
keystream (`KSW`). Decryption reuses the same switch bits with the stage order
reversed.

This construction is canonical for three reasons. Feistel and Fisher-Yates
permutations were evaluated as alternatives during development; neither is
published here, and neither is a conforming `S`.

1. **Bit-identical in C and HDL.** It is just conditional swaps, so software and
   silicon derive the *same* permutation and interoperate. The Feistel needs
   cycle-walking and Fisher-Yates is sequential — neither maps cleanly to both.
2. **Self-contained.** It is built only from Enqpy keystream — no foreign
   primitive. A ChaCha-keyed Fisher-Yates shuffle would give a cleaner "uniform
   random permutation" argument, but it imports ChaCha, which defeats the point of
   a cipher whose pitch is "simpler than ChaCha, no S-boxes." That is why the
   butterfly was chosen despite the weaker distributional argument.
3. **Tested.** It is the construction the A3 falsification sweep exercised
   (secret-network equivocation held past known-S, to threshold m=7 at n=4).

## Security scope — read this

The ESE **raises** the known-plaintext threshold; it does **not** restore
information-theoretic security under KP. In the n=4 determinacy sweep, secret-`S`
equivocation held past the known-`S` point but was not unbounded. The strong,
proven claim remains the base cipher's **ciphertext-only** plaintext equivocation.
State it that way: "the base proof is ciphertext-only; this optional layer raises
the KP bar, tested by falsification to the documented threshold." Do not state
or imply a KP impossibility result.

`S` must be **secret and fresh per record** — its security as a hardening layer
depends on the switch-bit keystream being unknown and not reused. Pair the AEAD
with a standard MAC (encrypt-then-MAC) exactly as the base cipher does.

## Files

### In this repository (Apache-2.0)

| file | role |
|---|---|
| `enqpy_ese_reference.c` | canonical C reference (this layer; `S` + masks), self-test + parameter guard |

The base-cipher benchmark harness is `aead_bench.c`, which does not exercise ESE.

### Not in this repository

NQP's HDL references — the VHDL behavioural model, the synthesisable Verilog
datapath, their testbenches and the place-and-route harness — are **not published
here**, are **not** covered by the Open-Infrastructure Patent Non-Assertion
Covenant, and are **not** under the Apache-2.0 grant. They are available under NDA
or a signed agreement.

This is not a gap in the covenant, and it is worth being precise about why. The
covenant frees the cipher **invention**: anyone may implement Enqpy and ESE in
HDL, at any scale including commercially, with no fee and no permission, and the
FCD plus the published vectors are the blueprint for doing exactly that. What is
withheld is NQP's *particular RTL* — an implementation, not the invention. The
same split already governs the base cipher, where the covenant frees the cipher
and Apache-2.0 governs NQP's reference source.

Earlier revisions of this document listed a speed-comparison harness
(`aead_bench_ese.c`) carrying Feistel and Fisher-Yates `S` variants. That file
does not exist; the reference is withdrawn rather than left as a broken pointer.

## Cross-language interop anchor

The C reference, NQP's VHDL model and NQP's Verilog datapath are all held to one
number. For the shared vector set (`NB=64, LOGN=6, PASSES=3`, `P=7i+1`,
`W1=11i+3`, `W2=13i+5`, `KSW=k^(k>>1)^(k>>2)`) every implementation produces:

```
CT[0..7] = 09 72 27 58 41 72 37 B0
```

Re-verified for Rev 5.1 under GHDL 4.1.0 and Icarus Verilog 12.0. Only the C side
is reproducible from this repository; the HDL side is verified in NQP's private
build and is demonstrable under NDA. Your own HDL implementation should reproduce
the same value — that is the point of publishing it here.

`W1`/`W2`/`KSW` are supplied as inputs here (matching the VHDL module interface). In
deployment they are separately derived Enqpy keystreams from `enqpy_reference.c`'s
`PDAF_SEC` under distinct, domain-separated per-record credentials.

## Profile parameters — checked, not assumed

`ese_encrypt` and `ese_decrypt` return **0 on success and −1 on an invalid
profile**, matching `PDAF_SEC`'s convention in the base reference. Callers MUST
check the return value. The precondition is:

| Requirement | Why |
| --- | --- |
| `NB == (1 << LOGN)` | the butterfly network is defined over exactly `2^LOGN` bytes |
| `NB <= 2048` (`ESE_MAX_NB`) | fixed internal buffers |
| `PASSES >= 2` | Benes-class permutation coverage — see below |
| `length(KSW) >= PASSES * LOGN * (NB / 2)` | one switch bit per pair, per stage |

Two of these are **silent** failure modes — the network still produces a bijection
and still round-trips, so a naive self-check passes while `S` is materially weaker.
Both are therefore checked rather than assumed, in the C (`ese_params_valid()`) and
by an elaboration-time assertion in the VHDL.

**`NB != (1 << LOGN)`.** The stages never mix across the top block boundary and `S`
degenerates into a permutation confined to sub-blocks. Measured at `NB = 64`: with
`LOGN = 6` the permutation crosses the midpoint and reaches a maximum displacement
of 59; with `LOGN = 5` it **never** crosses the midpoint and reaches only 27.

**`PASSES < 2`.** A single butterfly pass is not rearrangeably nonblocking — it can
only reach a subset of the permutation group. Measured exhaustively at `NB = 8`,
`LOGN = 3`, enumerating every switch setting: **one pass reaches 4,096 of the
40,320 permutations (10.2%); two passes reach all 40,320 (100%).** Displacement
statistics do *not* reveal this — one pass looks fine by that measure — which is
precisely why the bound is stated as a requirement rather than left to inspection.
The canonical profile uses `PASSES = 3`.

Use `ese_ksw_len(NB, LOGN, PASSES)` to size the switch-bit buffer; at the
production HIGH profile (`NB=2048, LOGN=11, PASSES=3`) it is 33,792 bytes.

## Build & verify

What you can run from this repository:

```sh
# software reference: round-trip + permutation validity + parameter guard
cc -O2 -o ese_ref enqpy_ese_reference.c && ./ese_ref
```

That build is the conformance target. It prints the interop KAT above, so an
independent HDL implementation can be checked against it without any NQP RTL.

The corresponding HDL builds (GHDL for the VHDL model and its two testbenches,
Icarus or a vendor tool for the Verilog datapath) run against files that are not
published here; they were re-verified for Rev 5.1 and are demonstrable under NDA.

### Interop KAT (the shared golden vector)

Vectors `P[i]=(7i+1)&255`, `W1[i]=(11i+3)&255`, `W2[i]=(13i+5)&255`,
`KSW[k]=(k^(k>>1)^(k>>2))&1`, profile `NB=64, LOGN=6, PASSES=3`:

```
CT[0..7] = 09 72 27 58 41 72 37 B0      (C reference and VHDL agree)
```

Both `enqpy_ese_reference.c` and `enqpy_ese_xcheck_tb.vhd` reproduce this value.
The production HIGH window is `NB=2048, LOGN=11, PASSES=3`.

## License & export

**The C reference (`enqpy_ese_reference.c`) is Apache-2.0**, consistent with the
base repository, and carries an `SPDX-License-Identifier` header. As publicly
available encryption source code, file the same EAR §742.15(b) notification for
it that the base release used before publishing.

**NQP's HDL is not published and is not Apache-2.0.** It is not covered by the
covenant either — see "Not in this repository" above for why that is a
distinction about implementations rather than a limit on the invention. Because
it is not publicly available source code, the §742.15(b) notification route does
not apply to it; treat any HDL disclosure as a controlled transfer and take
export advice before sending it, including under NDA. As publicly available
encryption source code, file the same EAR §742.15(b) notification for this artifact
that the base release used before publishing.
