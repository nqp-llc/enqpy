# Enqpy™ ESE — optional KPA-hardening reference (Rev 5.0)

`C = S(P + W1) + W2`  (+ = per-nibble mod-16 addition over ℤ₁₆)

Base Enqpy's record encryption is byte XOR; ESE's two masking steps use mod-16
addition — the ℤ₁₆ structure the obstruction analysis rests on. ESE composes
independent Enqpy keystreams around a secret permutation; it does not change base
Enqpy's operation.

This is the **optional** known-plaintext (KP) hardening layer for Enqpy. It is not
part of the base cipher and not required for the base cipher's proven guarantee.
It exists for deployments where a single key epoch also encrypts attacker-known
plaintext, and you want to raise the bar against that case in addition to the base
cipher's ciphertext-only proof.

## What it is

The base Enqpy cipher (`enqpy_reference.c`) has a proof about **ciphertext-only**
plaintext equivocation in the stated model: from ciphertext alone, the message
keeps at least 128 bits of min-entropy at HIGH, for any quantity of ciphertext,
and the bound does not rest on public-key hardness assumptions. Known plaintext at
one key epoch is outside that proof and is handled operationally by per-record
epoch rotation (`nil_comm`/HKU).

The ESE layer adds a second line of defense at the cipher layer:

- `W1`, `W2` — two **independent** Enqpy keystreams (`PDAF_SEC` under separate
  per-record credentials).
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

This construction is canonical (over the Feistel and Fisher-Yates variants in
`aead_bench_ese.c`, which exist only for speed comparison) for three reasons:

1. **Bit-identical in C and HDL.** It is just conditional swaps, so software and
   silicon derive the *same* permutation and interoperate. The Feistel needs
   cycle-walking and Fisher-Yates is sequential — neither maps cleanly to both.
2. **Self-contained.** It is built only from Enqpy keystream — no foreign
   primitive. (The ChaCha-keyed Fisher-Yates variant gives a cleaner "uniform
   random permutation" argument but imports ChaCha, which defeats the point of a
   cipher whose pitch is "simpler than ChaCha, no S-boxes.")
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

| file | role |
|---|---|
| `enqpy_ese_reference.c` | canonical C reference (this layer; `S` + masks), self-test |
| `enqpy_ese_hardening.vhd` | silicon reference (same `S`); round-trip + bijection TB |
| `enqpy_ese_xcheck_tb.vhd` | SW↔HW interop anchor: VHDL must reproduce the C ciphertext |
| `aead_bench_ese.c` | speed harness only (Feistel / Fisher-Yates `S` variants) — not canonical |

`W1`/`W2`/`KSW` are supplied as inputs here (matching the VHDL module interface). In
deployment they are independent Enqpy keystreams from `enqpy_reference.c`'s
`PDAF_SEC` under domain-separated per-record credentials.

## Build & verify

```sh
# software reference: round-trip + permutation-validity self-test
cc -O2 -o ese_ref enqpy_ese_reference.c && ./ese_ref

# silicon reference: round-trip + bijection
ghdl -a --std=08 enqpy_reference_pkg.vhd enqpy_ese_hardening.vhd enqpy_ese_tb.vhd
ghdl -r --std=08 enqpy_ese_tb

# SW<->HW interop anchor: VHDL ciphertext must equal the C reference
ghdl -a --std=08 enqpy_ese_xcheck_tb.vhd && ghdl -r --std=08 enqpy_ese_xcheck_tb
```

### Interop KAT (the shared golden vector)

Vectors `P[i]=(7i+1)&255`, `W1[i]=(11i+3)&255`, `W2[i]=(13i+5)&255`,
`KSW[k]=(k^(k>>1)^(k>>2))&1`, profile `NB=64, LOGN=6, PASSES=3`:

```
CT[0..7] = 09 72 27 58 41 72 37 B0      (C reference and VHDL agree)
```

Both `enqpy_ese_reference.c` and `enqpy_ese_xcheck_tb.vhd` reproduce this value.
The production HIGH window is `NB=2048, LOGN=11, PASSES=3`.

## License & export

Apache-2.0 (code), consistent with the base repository. As publicly available
encryption source code, file the same EAR §742.15(b) notification for this artifact
that the base release used before publishing.
