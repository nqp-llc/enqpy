/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2026 NQP LLC (Paul McGough) */
#define _POSIX_C_SOURCE 200809L
/* =============================================================================
 * Enqpy(tm) Stream Cipher -- C Reference Implementation  Rev 4.0
 * Copyright (c) 2026 NQP LLC (Paul McGough).
 *
 * WHAT THIS FILE IS
 * -----------------
 * This is the canonical software reference for the Enqpy(tm) cipher CORE --
 * the profile for which the Rev 4.0 formal results are proved in the
 * CIPHERTEXT-ONLY, single-key-epoch model:
 *
 *   Key axis:     H(EK, QK | T^inf) = log2(4) = 2 bits exactly in the
 *                 ciphertext-only view (Theorem 2).
 *   Message axis: |S(CT,OR)| >= 2^128 for HIGH (n=64), with the posterior
 *                 UNIFORM over the FULL consistent set, so
 *                 H(PT|CT,OR) = Hinf(PT|CT,OR) >= 128 bits (Theorem 3),
 *                 under ciphertext-only observation.
 *
 * KNOWN-PLAINTEXT BOUNDARY and DEPLOYMENT (Rev 4.0):
 *   The results above are CIPHERTEXT-ONLY and per-key-epoch. Under known
 *   plaintext at the Enqpy layer with one continuing key state, exposure
 *   accumulates: ~2 fully-known 2,048-byte windows under distinct nonces
 *   transfer across fresh nonces (cross-OR decryption). See FCD Rev 4.0
 *   8.5 / proof 16. The deployable profile is Enqpy-HKU: the core run under
 *   fresh external-entropy key-epoch rotation (the NIL-Comm Method 2 primitive
 *   below) with encrypt-then-MAC, rotating before a second fully-known window
 *   (FCD 8.10 / proof 17). This file implements the CORE and the Method 2
 *   rotation primitive; full HKU epoch-management and its KATs are a separate
 *   Rev 4.0 code update.
 *
 * Enqpy is the Canonical Configuration: nonce-only OffsetKey derivation
 * (Key Role Separation), Case-1 W generation, a normative 2,048-byte (HIGH)
 * plaintext window, and synchronized key update. Generating W by Case 1 is
 * exactly what makes the (EK,QK) -> W map a Z16-module homomorphism, which is
 * what closes the message-axis min-entropy theorem (Theorem 3).
 *
 * CONSTRUCTION (FCD Rev 4.0)
 * --------------------------
 * Phase 1 (OR_EXP/eff_or), Phase 2 (nonce-only VKP/OKP; VKC/OKC), the PDAF
 * primitive, and the NIL-Comm primitives derive the per-session keys:
 *
 *   - Phase 3 (W generation) is Case 1:
 *         W[c*n + p] = (OKC[p] + VKC[(p + c) mod n]) mod 16
 *     yielding n^2 nibbles = 2,048 bytes per window at HIGH.
 *   - w_byte_max = n^2 / 2 = 2,048 (HIGH), the normative window bound (R6).
 *   - Phase 5 performs the synchronized key update.
 *   - Credential rotation (NIL-Comm) REQUIRES Method 2 (external entropy);
 *     Method 1 is prohibited (R6), because its deterministic chain collapses
 *     the equivocation coset 4->1 and is simulable, so it does not reset the
 *     known-plaintext accumulation of FCD Rev 4.0 8.5 (see FCD 7.4; the
 *     formal sufficiency of the Method 2 reset is the fresh-HKU reset lemma,
 *     proof 17). The OWC/Mode-0 primitives are retained; only the policy gate
 *     applies.
 *
 * The PUBLIC API (PDAF, PDAF_SEC, ENQPY_NIL_COMM_UPDATE, OWC, enqpy_init) is
 * the object linked by the AEAD benchmark harness (aead_bench.c).
 *
 * LICENSE
 * -------
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0 or in the
 * LICENSE file in this repository.  Unless required by applicable law or
 * agreed to in writing, software distributed under the License is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND.  See the
 * License and the NOTICE file for governing terms.
 *
 * The CIPHER itself is patent-safe for everyone under the Open-Infrastructure
 * Patent Non-Assertion Covenant (https://enqpy.com/covenant): anyone may write
 * an independent implementation and deploy it at any scale, including
 * commercially, with no fee and no separate agreement required for that use.
 *
 * ALGORITHM OVERVIEW (Enqpy)
 * --------------------------------
 *  Phase 1  OR_EXP and eff_or
 *           Expand the 64-bit OR counter to n nibbles via PDAF Mode 1
 *           self-expansion, then mix with the stored OR register -> eff_or.
 *
 *  Phase 2  VKP, OKP, VKC, OKC  [CANONICAL CONFIGURATION -- nonce-only pointers]
 *           VKP = PDAF1(eff_or, eff_or)[:n]   -- nonce only, public.
 *           OKP[i] = (VKP[i] + DS_SEP) mod 16 -- domain-separated, public.
 *           VKC = PDAF1(EK, VKP)              -- EK as ValueKey only.
 *           OKC = PDAF1(QK, OKP)              -- QK as ValueKey only.
 *
 *  Phase 3  W keystream generation -- CASE 1
 *           W[c*n + p] = (OKC[p] + VKC[(p+c) mod n]) mod 16, n^2 nibbles,
 *           packed 2 nibbles/byte = 2,048 bytes per window at HIGH.
 *
 *  Phase 4  XOR  -- W XOR plaintext/ciphertext.
 *
 *  Phase 5  Key update (synchronized)
 *           VKNext = PDAF1(OKC, VKP, Mode 1)
 *           OKNext = PDAF1(VKC, OKP, Mode 1)
 *           VKP/OKP are nonce-derived, so the [+8] coset invariant propagates
 *           across the 2,048-byte window boundary.
 *
 * SECURITY PROFILES (key size)
 * ----------------------------
 *   LOW   n=32 nibbles (128-bit)
 *   MED   n=48 nibbles (192-bit)
 *   HIGH  n=64 nibbles (256-bit)  -- default; the canonical proof profile.
 *
 * OFFICIAL TEST VECTORS (Rev 4.0 -- Enqpy core; unchanged from Rev 3.0)
 * ---------------------------------------------
 *   PDAF primitive (n=10, 30-nibble output):
 *     VK = FB382C001A   OK = CC69100AB4
 *     Mode 0 -> B7913C0ACE7FEBD00B53F4851014AF
 *     Mode 1 -> 7DD02C010CDF74C01B5BF8D811B92B
 *
 *   PDAF_SEC Enqpy (n=64, 8-byte zero plaintext), canonical keys:
 *     EK = cb1e1203c479f30c1c356f12362fe43b47e8b5906c992013468395489a17d957
 *     QK = 0e2eab25a9f78620abb6726cf81a012776511b3988431d427da911bdc2130680
 *     OR = 3667a507e1109ee32cd50718fa511065900eb422ac187ac5cd47ef5b18d86e0c
 *     OR_CTR = 0x0000000000000001
 *     Enqpy W[0..7]  = 24 34 B5 88 45 C6 FD E8   (CT = W, plaintext zeros)
 *     Enqpy W[0..15] = 24 34 B5 88 45 C6 FD E8 A3 38 55 C3 6B 7D A1 96
 *     Window-boundary byte [2046..2047] = 54 28   (last 2 bytes of the
 *                                                  2,048-byte Case-1 window)
 *   Coset invariant verified (all generated by this reference):
 *     W(EK+8,QK) == W(EK,QK) == W(EK,QK+8) == W(EK+8,QK+8).
 *
 * BUILD
 * -----
 *   # Library only:
 *   cc -O2 -std=c11 -c enqpy_reference.c -o enqpy_reference.o
 *   # With self-test:
 *   cc -O3 -std=c11 -DENQPY_SELFTEST enqpy_reference.c -o enqpy_test
 *   # With benchmark:
 *   cc -O3 -std=c11 -DENQPY_BENCHMARK enqpy_reference.c -o enqpy_bench
 *   # AEAD benchmark (link with aead_bench.c):
 *   cc -O3 -march=native -std=c11 -c enqpy_reference.c -o enqpy_ref.o
 *   cc -O3 -march=native -std=c11 -D_POSIX_C_SOURCE=200809L aead_bench.c enqpy_ref.o -o aead_bench
 * ============================================================================= */

#include <stdint.h>
#include <string.h>
#include <stddef.h>

#if defined(ENQPY_BENCHMARK) || defined(ENQPY_SELFTEST)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#endif

/* ============================================================================
 * SECTION 1 -- CONFIGURATION CONSTANTS
 * ============================================================================ */

#define ENQPY_MAX_N     64
#define ENQPY_TILE_LEN  (2 * ENQPY_MAX_N + 16)  /* 144 */
#define DS_SEP        0xFu   /* OKP domain separator: OKP[i] = (VKP[i] + 0xF) mod 16 */
#define ORC_NIBS      16     /* 64-bit OR_CTR -> 16 nibbles */

/* Enqpy: Case-1 only.  W is n^2 nibbles = n^2/2 bytes per window. */
#define ENQPY_W_NIBS    (ENQPY_MAX_N * ENQPY_MAX_N)   /* 4096 */
#define ENQPY_W_BYTES   (ENQPY_W_NIBS / 2)            /* 2048 */

/* ============================================================================
 * SECTION 2 -- MOD16 LOOKUP TABLE
 * ============================================================================ */

static uint8_t MOD16[16][16];

void enqpy_init(uint64_t persisted_or_ctr)
{
    (void)persisted_or_ctr;
    for (int a = 0; a < 16; a++)
        for (int b = 0; b < 16; b++)
            MOD16[a][b] = (uint8_t)((a + b) & 0xF);
}

/* ============================================================================
 * SECTION 3 -- OWC (One-Way Compression)
 * Used exclusively in the Nil-Communication Key Update path.  The Enqpy uses
 * it only under the Nil-Communication Method 2 path.
 * ============================================================================ */

int OWC(const uint8_t *key_nibs, int nLen, int nSkip, uint8_t *out_nibs)
{
    if (!key_nibs || !out_nibs || nLen < 2) return -1;
    int out_len = 0;
    /* Non-overlapping pairs: the position pointer advances by (1 + nSkip) per
     * output nibble, combining positions i and i+nSkip. If i+nSkip falls beyond
     * the end of the array, fall back to the adjacent nibble at i+1.          */
    for (int i = 0; i < nLen - 1; i += (1 + nSkip)) {
        int j = i + nSkip;
        if (j >= nLen) j = i + 1;
        out_nibs[out_len++] = MOD16[key_nibs[i] & 0xF][key_nibs[j] & 0xF];
    }
    return out_len;
}

/* ============================================================================
 * SECTION 4 -- PDAF CORE
 *
 * Mode 1 (primary cipher path):
 *   Both arrays tiled to ENQPY_TILE_LEN.  For each output nibble nN:
 *     delta = ok_tile[p]
 *     out[nN] = MOD16(vk_tile[p+c], vk_tile[p+delta+1+c])
 *
 * Mode 0 (Nil-Comm path only):
 *   out[nN] = MOD16(ok[p % n], vk[(p+c) % n])
 * ============================================================================ */

static void pdaf_mode1(const uint8_t *vk, const uint8_t *ok, int n,
                       int nDigits, uint8_t *out)
{
    uint8_t vk_t[ENQPY_TILE_LEN];
    uint8_t ok_t[ENQPY_TILE_LEN];
    for (int i = 0; i < ENQPY_TILE_LEN; i++) {
        vk_t[i] = vk[i % n] & 0xF;
        ok_t[i] = ok[i % n] & 0xF;
    }
    int p = 0, c = 0;
    for (int nN = 0; nN < nDigits; nN++) {
        int delta = ok_t[p];
        out[nN]   = MOD16[vk_t[p + c]][vk_t[p + delta + 1 + c]];
        if (++p >= n) { p = 0; c++; }
    }
}

static void pdaf_mode0(const uint8_t *vk, const uint8_t *ok, int n,
                       int nDigits, uint8_t *out)
{
    int p = 0, c = 0;
    for (int nN = 0; nN < nDigits; nN++) {
        out[nN] = MOD16[ok[p % n] & 0xF][vk[(p + c) % n] & 0xF];
        if (++p >= n) { p = 0; c++; }
    }
}

int PDAF(const uint8_t *vk, const uint8_t *ok, int n,
         int nMode, int nDigits, uint8_t *out)
{
    if (!vk || !ok || !out || n < 1 || nDigits < 1) return -1;
    if (nMode == 1) pdaf_mode1(vk, ok, n, nDigits, out);
    else            pdaf_mode0(vk, ok, n, nDigits, out);
    return nDigits;
}

/* ============================================================================
 * SECTION 5 -- W KEYSTREAM GENERATION  [ENQPY: CASE 1 ONLY]
 *
 * W[c*n + p] = (OKC[p] + VKC[(p + c) mod n]) mod 16, for c,p in 0..n-1.
 * This is a single Z16-linear stream (the Case-1 W engine). n^2 nibbles are packed 2-per-byte, in order, with
 * no interleaving and no running-parity branch (n is even, so n^2 is even and
 * every pair (2k, 2k+1) forms one byte exactly).
 * ============================================================================ */

static void generate_w(const uint8_t *vkc, const uint8_t *okc, int n,
                          uint8_t *w_bytes)
{
    uint8_t vk_t[ENQPY_TILE_LEN];
    uint8_t ok_t[ENQPY_TILE_LEN];
    for (int i = 0; i < ENQPY_TILE_LEN; i++) {
        vk_t[i] = vkc[i % n] & 0xF;
        ok_t[i] = okc[i % n] & 0xF;
    }
    uint8_t w[ENQPY_MAX_N * ENQPY_MAX_N];
    for (int c = 0; c < n; c++) {
        const int cn = c * n;
        for (int p = 0; p < n; p++)
            w[cn + p] = (uint8_t)((ok_t[p] + vk_t[p + c]) & 0xF);  /* MOD16 */
    }
    int nn = n * n, bi = 0;
    for (int k = 0; k < nn; k += 2)         /* 2 nibbles -> 1 byte, in order */
        w_bytes[bi++] = (uint8_t)((w[k] << 4) | w[k + 1]);
}

/* ============================================================================
 * SECTION 6 -- PDAF_SEC (Main Encrypt / Decrypt)  [ENQPY]
 *
 * Phases 1 and 2 perform nonce-only pointer derivation / Key Role Separation.
 * Phase 3 is Case-1 W generation; Phase 5 is the synchronized key update;
 * the window is n^2/2 = 2,048 bytes at HIGH.
 * ============================================================================ */

int PDAF_SEC(const uint8_t *ek, const uint8_t *qk,
             const uint8_t *or_nibs, uint64_t or_ctr,
             int n, const uint8_t *restrict target,
             int nTextLen, uint8_t *restrict out)
{
    if (!ek || !qk || !or_nibs || !target || !out) return -1;
    if (n < 32 || n > ENQPY_MAX_N || nTextLen < 0)   return -1;

    /* -----------------------------------------------------------------------
     * PHASE 1 -- OR_EXP and eff_or
     * ----------------------------------------------------------------------- */
    uint8_t or_ctr_nibs[ENQPY_MAX_N];
    memset(or_ctr_nibs, 0, sizeof(or_ctr_nibs));
    for (int i = 0; i < ORC_NIBS; i++)
        or_ctr_nibs[i] = (uint8_t)((or_ctr >> (60 - i * 4)) & 0xF);

    uint8_t or_exp[ENQPY_MAX_N];
    pdaf_mode1(or_ctr_nibs, or_ctr_nibs, n, n, or_exp);

    uint8_t eff_or[ENQPY_MAX_N];
    for (int i = 0; i < n; i++)
        eff_or[i] = MOD16[or_nibs[i] & 0xF][or_exp[i]];

    /* -----------------------------------------------------------------------
     * PHASE 2 -- VKP, OKP, VKC, OKC  [CANONICAL CONFIGURATION -- nonce-only]
     *   VKP    = PDAF1(eff_or, eff_or)[:n]
     *   OKP[i] = (VKP[i] + DS_SEP) mod 16
     *   VKC    = PDAF1(EK, VKP)    OKC = PDAF1(QK, OKP)
     * ----------------------------------------------------------------------- */
    uint8_t vkp[ENQPY_MAX_N], okp[ENQPY_MAX_N];
    pdaf_mode1(eff_or, eff_or, n, n, vkp);
    for (int i = 0; i < n; i++)
        okp[i] = MOD16[vkp[i]][DS_SEP];

    uint8_t vkc[ENQPY_MAX_N], okc[ENQPY_MAX_N];
    {
        uint8_t vk_t_ek[ENQPY_TILE_LEN], ok_t_vkp[ENQPY_TILE_LEN];
        uint8_t vk_t_qk[ENQPY_TILE_LEN], ok_t_okp[ENQPY_TILE_LEN];
        for (int i = 0; i < ENQPY_TILE_LEN; i++) {
            vk_t_ek[i]  = ek[i  % n] & 0xF;
            ok_t_vkp[i] = vkp[i % n] & 0xF;
            vk_t_qk[i]  = qk[i  % n] & 0xF;
            ok_t_okp[i] = okp[i % n] & 0xF;
        }
        int p = 0, c = 0;
        for (int nN = 0; nN < n; nN++) {
            int dv  = ok_t_vkp[p];
            vkc[nN] = MOD16[vk_t_ek[p + c]][vk_t_ek[p + dv + 1 + c]];
            int do_ = ok_t_okp[p];
            okc[nN] = MOD16[vk_t_qk[p + c]][vk_t_qk[p + do_ + 1 + c]];
            if (++p >= n) { p = 0; c++; }
        }
    }

    /* -----------------------------------------------------------------------
     * PHASES 3, 4, 5 -- Streaming loop (Enqpy: Case-1 W)
     * Window bound: w_byte_max = n^2/2 (= 2,048 at HIGH).
     * ----------------------------------------------------------------------- */
    uint8_t w_bytes[ENQPY_W_BYTES];
    int w_byte_max = (n * n) / 2;     /* Enqpy window (R6) */
    int text_done  = 0;

    while (text_done < nTextLen) {
        /* Phase 3 -- Case 1 only */
        generate_w(vkc, okc, n, w_bytes);

        /* Phase 4 */
        int chunk = nTextLen - text_done;
        if (chunk > w_byte_max) chunk = w_byte_max;
        const uint8_t *in_ptr  = target + text_done;
        uint8_t       *out_ptr = out    + text_done;
        for (int i = 0; i < chunk; i++)
            out_ptr[i] = w_bytes[i] ^ in_ptr[i];
        text_done += chunk;

        if (text_done >= nTextLen) break;

        /* Phase 5 -- Key update (synchronized)
         *   VKNext = PDAF1(OKC, VKP, Mode 1)
         *   OKNext = PDAF1(VKC, OKP, Mode 1)
         * VKP/OKP are nonce-derived; the [+8] coset invariant is preserved. */
        uint8_t vk_t_okc[ENQPY_TILE_LEN], ok_t_vkp5[ENQPY_TILE_LEN];
        uint8_t vk_t_vkc[ENQPY_TILE_LEN], ok_t_okp5[ENQPY_TILE_LEN];
        for (int i = 0; i < ENQPY_TILE_LEN; i++) {
            vk_t_okc[i]  = okc[i % n] & 0xF;
            ok_t_vkp5[i] = vkp[i % n] & 0xF;
            vk_t_vkc[i]  = vkc[i % n] & 0xF;
            ok_t_okp5[i] = okp[i % n] & 0xF;
        }
        uint8_t vkn[ENQPY_MAX_N], okn[ENQPY_MAX_N];
        int p = 0, c = 0;
        for (int nN = 0; nN < n; nN++) {
            int dv  = ok_t_vkp5[p];
            vkn[nN] = MOD16[vk_t_okc[p + c]][vk_t_okc[p + dv + 1 + c]];
            int do_ = ok_t_okp5[p];
            okn[nN] = MOD16[vk_t_vkc[p + c]][vk_t_vkc[p + do_ + 1 + c]];
            if (++p >= n) { p = 0; c++; }
        }
        memcpy(vkc, vkn, n);
        memcpy(okc, okn, n);
    }

    memset(vkp,    0, sizeof vkp);
    memset(okp,    0, sizeof okp);
    memset(vkc,    0, sizeof vkc);
    memset(okc,    0, sizeof okc);
    memset(or_exp, 0, sizeof or_exp);
    memset(eff_or, 0, sizeof eff_or);
    memset(w_bytes,0, sizeof w_bytes);

    return text_done;
}

/* ============================================================================
 * SECTION 7 -- NIL-COMMUNICATION KEY UPDATE
 *
 * ENQPY POLICY (Rev 4.0, R6 / FCD 8.5 boundary, proof 17):
 *   Method 2 (external entropy) is REQUIRED for Enqpy credential
 *   rotation. Method 1 (deterministic chain) is PROHIBITED here because it
 *   collapses the four-element equivocation coset to a single new pair and is
 *   simulable, so it does not reset the known-plaintext accumulation: in the
 *   known-plaintext limit the post-rotation current-epoch key is determined.
 *   Method 2's external entropy re-injects key uncertainty at each rotation
 *   and is the Enqpy-HKU reset primitive (its formal sufficiency against the
 *   coset/linearity structure is the fresh-HKU reset lemma, proof 17).
 *   This reference enforces the policy by rejecting method != 2.
 *   The OWC + Mode-0 seed derivation is part of the Nil-Communication path.
 * ============================================================================ */

int ENQPY_NIL_COMM_UPDATE(const uint8_t *ek, const uint8_t *qk, int n,
                          const uint8_t *e_ext, int method,
                          uint8_t *ek_new, uint8_t *qk_new)
{
    if (!ek || !qk || !ek_new || !qk_new) return -1;
    if (n < 32 || n > ENQPY_MAX_N)          return -1;
    /* Enqpy: Method 2 is mandatory; Method 1 is prohibited (R6). */
    if (method != 2 || !e_ext)             return -1;

    uint8_t pdaf_m0[ENQPY_MAX_N];
    pdaf_mode0(ek, qk, n, n, pdaf_m0);

    uint8_t e_seed_raw[ENQPY_MAX_N / 2];
    int seed_len = OWC(pdaf_m0, n, 1, e_seed_raw);
    if (seed_len < 1) return -1;

    uint8_t e_combined[ENQPY_MAX_N];
    for (int i = 0; i < n; i++)
        e_combined[i] = MOD16[e_seed_raw[i % seed_len]][e_ext[i] & 0xF];

    {
        uint8_t vk_t_ek[ENQPY_TILE_LEN], ok_t_ec[ENQPY_TILE_LEN];
        uint8_t vk_t_qk[ENQPY_TILE_LEN];
        for (int i = 0; i < ENQPY_TILE_LEN; i++) {
            vk_t_ek[i] = ek[i % n] & 0xF;
            ok_t_ec[i] = e_combined[i % n] & 0xF;
            vk_t_qk[i] = qk[i % n] & 0xF;
        }
        int p = 0, c = 0;
        for (int nN = 0; nN < n; nN++) {
            int d       = ok_t_ec[p];
            ek_new[nN]  = MOD16[vk_t_ek[p + c]][vk_t_ek[p + d + 1 + c]];
            qk_new[nN]  = MOD16[vk_t_qk[p + c]][vk_t_qk[p + d + 1 + c]];
            if (++p >= n) { p = 0; c++; }
        }
    }

    memset(pdaf_m0,    0, sizeof pdaf_m0);
    memset(e_seed_raw, 0, sizeof e_seed_raw);
    memset(e_combined, 0, sizeof e_combined);
    return 0;
}

/* ============================================================================
 * SECTION 8 -- SELF-TEST HARNESS  (Enqpy KATs)
 * ============================================================================ */
#ifdef ENQPY_SELFTEST

static const uint8_t TV_EK[64] = {
    0xC,0xB,0x1,0xE, 0x1,0x2,0x0,0x3, 0xC,0x4,0x7,0x9,
    0xF,0x3,0x0,0xC, 0x1,0xC,0x3,0x5, 0x6,0xF,0x1,0x2,
    0x3,0x6,0x2,0xF, 0xE,0x4,0x3,0xB, 0x4,0x7,0xE,0x8,
    0xB,0x5,0x9,0x0, 0x6,0xC,0x9,0x9, 0x2,0x0,0x1,0x3,
    0x4,0x6,0x8,0x3, 0x9,0x5,0x4,0x8, 0x9,0xA,0x1,0x7,
    0xD,0x9,0x5,0x7
};
static const uint8_t TV_QK[64] = {
    0x0,0xE,0x2,0xE, 0xA,0xB,0x2,0x5, 0xA,0x9,0xF,0x7,
    0x8,0x6,0x2,0x0, 0xA,0xB,0xB,0x6, 0x7,0x2,0x6,0xC,
    0xF,0x8,0x1,0xA, 0x0,0x1,0x2,0x7, 0x7,0x6,0x5,0x1,
    0x1,0xB,0x3,0x9, 0x8,0x8,0x4,0x3, 0x1,0xD,0x4,0x2,
    0x7,0xD,0xA,0x9, 0x1,0x1,0xB,0xD, 0xC,0x2,0x1,0x3,
    0x0,0x6,0x8,0x0
};
static const uint8_t TV_OR[64] = {
    0x3,0x6,0x6,0x7, 0xA,0x5,0x0,0x7, 0xE,0x1,0x1,0x0,
    0x9,0xE,0xE,0x3, 0x2,0xC,0xD,0x5, 0x0,0x7,0x1,0x8,
    0xF,0xA,0x5,0x1, 0x1,0x0,0x6,0x5, 0x9,0x0,0x0,0xE,
    0xB,0x4,0x2,0x2, 0xA,0xC,0x1,0x8, 0x7,0xA,0xC,0x5,
    0xC,0xD,0x4,0x7, 0xE,0xF,0x5,0xB, 0x1,0x8,0xD,0x8,
    0x6,0xE,0x0,0xC
};

static int selftest(void)
{
    int pass = 0, fail = 0;
    enqpy_init(0);

    /* TV1: PDAF Mode 0 (n=10, 30-nibble) -- shared primitive vector */
    {
        uint8_t vk[64]={0xF,0xB,0x3,0x8,0x2,0xC,0x0,0x0,0x1,0xA};
        uint8_t ok[64]={0xC,0xC,0x6,0x9,0x1,0x0,0x0,0xA,0xB,0x4};
        uint8_t out[64]={0};
        const uint8_t exp[30]={
            0xB,0x7,0x9,0x1,0x3,0xC,0x0,0xA,0xC,0xE,
            0x7,0xF,0xE,0xB,0xD,0x0,0x0,0xB,0x5,0x3,
            0xF,0x4,0x8,0x5,0x1,0x0,0x1,0x4,0xA,0xF
        };
        PDAF(vk, ok, 10, 0, 30, out);
        for (int i=0;i<30;i++) {
            if (out[i]==exp[i]) pass++;
            else { fail++; printf("TV1 FAIL nibble[%d] got=%X exp=%X\n",i,out[i],exp[i]); }
        }
    }

    /* TV2: PDAF Mode 1 (n=10, 30-nibble) -- shared primitive vector */
    {
        uint8_t vk[64]={0xF,0xB,0x3,0x8,0x2,0xC,0x0,0x0,0x1,0xA};
        uint8_t ok[64]={0xC,0xC,0x6,0x9,0x1,0x0,0x0,0xA,0xB,0x4};
        uint8_t out[64]={0};
        const uint8_t exp[30]={
            0x7,0xD,0xD,0x0,0x2,0xC,0x0,0x1,0x0,0xC,
            0xD,0xF,0x7,0x4,0xC,0x0,0x1,0xB,0x5,0xB,
            0xF,0x8,0xD,0x8,0x1,0x1,0xB,0x9,0x2,0xB
        };
        PDAF(vk, ok, 10, 1, 30, out);
        for (int i=0;i<30;i++) {
            if (out[i]==exp[i]) pass++;
            else { fail++; printf("TV2 FAIL nibble[%d] got=%X exp=%X\n",i,out[i],exp[i]); }
        }
    }

    /* TV3: PDAF_SEC Enqpy -- W[0..7] = 24 34 B5 88 45 C6 FD E8 */
    {
        const uint8_t expected_w[8] = {0x24,0x34,0xB5,0x88,0x45,0xC6,0xFD,0xE8};
        uint8_t pt[8] = {0};
        uint8_t ct[8], rt[8];

        int r = PDAF_SEC(TV_EK, TV_QK, TV_OR, 0x0000000000000001ULL, 64, pt, 8, ct);
        if (r != 8) { fail++; printf("TV3 FAIL: PDAF_SEC returned %d\n", r); }
        else {
            for (int i=0;i<8;i++) {
                if (ct[i]==expected_w[i]) pass++;
                else { fail++; printf("TV3 FAIL CT[%d] got=%02X exp=%02X\n",
                                      i,ct[i],expected_w[i]); }
            }
        }
        /* Round-trip */
        PDAF_SEC(TV_EK, TV_QK, TV_OR, 0x0000000000000001ULL, 64, ct, 8, rt);
        for (int i=0;i<8;i++) {
            if (rt[i]==0x00) pass++;
            else { fail++; printf("TV3 RT FAIL[%d] got=%02X\n",i,rt[i]); }
        }
    }

    /* TV4: Coset invariant -- W(EK+8,QK) == W(EK,QK) (central proof property) */
    {
        uint8_t pt[8]={0}, ct[8], ct8[8];
        PDAF_SEC(TV_EK, TV_QK, TV_OR, 0x0000000000000001ULL, 64, pt, 8, ct);
        uint8_t ek8[64];
        for (int i=0;i<64;i++) ek8[i]=(TV_EK[i]+8)&0xF;
        PDAF_SEC(ek8, TV_QK, TV_OR, 0x0000000000000001ULL, 64, pt, 8, ct8);
        if (memcmp(ct, ct8, 8) == 0) pass++;
        else { fail++; printf("TV4 FAIL: EK+8 coset not preserved\n"); }

        /* TV5: QK+8 coset */
        uint8_t qk8[64], ct_q8[8];
        for (int i=0;i<64;i++) qk8[i]=(TV_QK[i]+8)&0xF;
        PDAF_SEC(TV_EK, qk8, TV_OR, 0x0000000000000001ULL, 64, pt, 8, ct_q8);
        if (memcmp(ct, ct_q8, 8) == 0) pass++;
        else { fail++; printf("TV5 FAIL: QK+8 coset not preserved\n"); }

        /* TV6: EK+8,QK+8 coset */
        uint8_t ct_b8[8];
        PDAF_SEC(ek8, qk8, TV_OR, 0x0000000000000001ULL, 64, pt, 8, ct_b8);
        if (memcmp(ct, ct_b8, 8) == 0) pass++;
        else { fail++; printf("TV6 FAIL: EK+8,QK+8 coset not preserved\n"); }
    }

    /* TV7: Window boundary -- byte [2046..2047] of the 2,048-byte window,
     * and that the SECOND window (after Phase 5) begins a fresh stream.       */
    {
        const uint8_t exp_tail[2] = {0x54, 0x28};
        uint8_t *pt = (uint8_t*)calloc(2050, 1);
        uint8_t *ct = (uint8_t*)malloc(2050);
        if (pt && ct) {
            PDAF_SEC(TV_EK, TV_QK, TV_OR, 0x0000000000000001ULL, 64, pt, 2050, ct);
            if (ct[2046]==exp_tail[0] && ct[2047]==exp_tail[1]) pass++;
            else { fail++; printf("TV7 FAIL: window tail [2046,2047] got=%02X %02X exp=%02X %02X\n",
                                  ct[2046],ct[2047],exp_tail[0],exp_tail[1]); }
            /* Byte 2048 is the first byte of the post-Phase-5 window; just
             * confirm round-trip integrity across the boundary.              */
            uint8_t *rt = (uint8_t*)malloc(2050);
            if (rt) {
                PDAF_SEC(TV_EK, TV_QK, TV_OR, 0x0000000000000001ULL, 64, ct, 2050, rt);
                int rt_ok = 1;
                for (int i=0;i<2050;i++) if (rt[i]!=0) { rt_ok=0; break; }
                if (rt_ok) pass++;
                else { fail++; printf("TV7 FAIL: 2050-byte round-trip mismatch\n"); }
                free(rt);
            }
        }
        free(pt); free(ct);
    }

    /* TV8: NIL-Comm Method 2 required; Method 1 rejected (rotation policy) */
    {
        uint8_t ek_new[64], qk_new[64];
        uint8_t e_ext[64];
        for (int i=0;i<64;i++) e_ext[i]=(uint8_t)((i*11+3)&0xF);
        /* Method 1 must be rejected. */
        int r1 = ENQPY_NIL_COMM_UPDATE(TV_EK, TV_QK, 64, NULL, 1, ek_new, qk_new);
        if (r1 == -1) pass++;
        else { fail++; printf("TV8 FAIL: Method 1 not rejected (got %d)\n", r1); }
        /* Method 2 must succeed and preserve the coset. */
        int r2 = ENQPY_NIL_COMM_UPDATE(TV_EK, TV_QK, 64, e_ext, 2, ek_new, qk_new);
        if (r2 == 0) pass++;
        else { fail++; printf("TV8 FAIL: Method 2 failed (got %d)\n", r2); }

        uint8_t ek8[64], qk8[64], ek_new8[64], qk_new8[64];
        for (int i=0;i<64;i++){ek8[i]=(TV_EK[i]+8)&0xF; qk8[i]=(TV_QK[i]+8)&0xF;}
        ENQPY_NIL_COMM_UPDATE(ek8, qk8, 64, e_ext, 2, ek_new8, qk_new8);
        /* Coset members map to the same new pair (Lemma B4 4->1 collapse). */
        if (memcmp(ek_new, ek_new8, 64)==0 && memcmp(qk_new, qk_new8, 64)==0) pass++;
        else { fail++; printf("TV8 FAIL: NIL Method 2 coset collapse not observed\n"); }
    }

    printf("Self-test: %d PASS  %d FAIL\n", pass, fail);
    return fail ? 1 : 0;
}

#endif /* ENQPY_SELFTEST */

/* ============================================================================
 * SECTION 9 -- BENCHMARK HARNESS  (Enqpy)
 * ============================================================================ */
#ifdef ENQPY_BENCHMARK

#define BENCH_ITERS   1000
#define BENCH_WARMUP  10

static double now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

static volatile uint8_t sink8;
static void use_buf(const uint8_t *buf, int len)
{
    for (int i = 0; i < len; i++) sink8 = buf[i];
}

static void bench_w_gen(void)
{
    printf("\n--- Phase 3 W generation (n=64, %d bytes/call, Case-1) ---\n", ENQPY_W_BYTES);
    printf("%-12s  %-12s  %-14s\n","iters","ns/call","MB/s");
    uint8_t vkc[64]={0},okc[64]={0},wb[ENQPY_W_BYTES];
    for(int i=0;i<64;i++){vkc[i]=(uint8_t)(i*3+2)&0xF;okc[i]=(uint8_t)(i*7+11)&0xF;}
    for(int w=0;w<BENCH_WARMUP;w++) generate_w(vkc,okc,64,wb);
    double t0=now_ns();
    for(int it=0;it<BENCH_ITERS;it++){generate_w(vkc,okc,64,wb);use_buf(wb,ENQPY_W_BYTES);}
    double el=now_ns()-t0;
    printf("%-12d  %-12.1f  %-14.2f\n",
           BENCH_ITERS, el/BENCH_ITERS,
           (double)ENQPY_W_BYTES*BENCH_ITERS/(el/1e9)/1e6);
}

static void bench_pdaf_sec(void)
{
    printf("\n--- PDAF_SEC Enqpy throughput (n=64) ---\n");
    printf("%-14s  %-10s  %-12s  %-12s\n","msg_size","iters","ns/call","MB/s");

    static const uint8_t ek[64]={
        0xC,0xB,0x1,0xE,0x1,0x2,0x0,0x3,0xC,0x4,0x7,0x9,0xF,0x3,0x0,0xC,
        0x1,0xC,0x3,0x5,0x6,0xF,0x1,0x2,0x3,0x6,0x2,0xF,0xE,0x4,0x3,0xB,
        0x4,0x7,0xE,0x8,0xB,0x5,0x9,0x0,0x6,0xC,0x9,0x9,0x2,0x0,0x1,0x3,
        0x4,0x6,0x8,0x3,0x9,0x5,0x4,0x8,0x9,0xA,0x1,0x7,0xD,0x9,0x5,0x7
    };
    static const uint8_t qk[64]={
        0x0,0xE,0x2,0xE,0xA,0xB,0x2,0x5,0xA,0x9,0xF,0x7,0x8,0x6,0x2,0x0,
        0xA,0xB,0xB,0x6,0x7,0x2,0x6,0xC,0xF,0x8,0x1,0xA,0x0,0x1,0x2,0x7,
        0x7,0x6,0x5,0x1,0x1,0xB,0x3,0x9,0x8,0x8,0x4,0x3,0x1,0xD,0x4,0x2,
        0x7,0xD,0xA,0x9,0x1,0x1,0xB,0xD,0xC,0x2,0x1,0x3,0x0,0x6,0x8,0x0
    };
    static const uint8_t or_nibs[64]={
        0x3,0x6,0x6,0x7,0xA,0x5,0x0,0x7,0xE,0x1,0x1,0x0,0x9,0xE,0xE,0x3,
        0x2,0xC,0xD,0x5,0x0,0x7,0x1,0x8,0xF,0xA,0x5,0x1,0x1,0x0,0x6,0x5,
        0x9,0x0,0x0,0xE,0xB,0x4,0x2,0x2,0xA,0xC,0x1,0x8,0x7,0xA,0xC,0x5,
        0xC,0xD,0x4,0x7,0xE,0xF,0x5,0xB,0x1,0x8,0xD,0x8,0x6,0xE,0x0,0xC
    };

    const int msg_sizes[]={1024,16*1024,1024*1024,0};
    const char *sz_labels[]={"1 KB","16 KB","1 MB",NULL};

    for(int mi=0;msg_sizes[mi];mi++){
        int sz=msg_sizes[mi];
        uint8_t *pt=(uint8_t*)calloc(sz,1);
        uint8_t *ct=(uint8_t*)malloc(sz);
        if(!pt||!ct){free(pt);free(ct);continue;}
        int iters=(sz<=1024)?BENCH_ITERS:(sz<=16384)?BENCH_ITERS/4:BENCH_ITERS/100;
        for(int w=0;w<BENCH_WARMUP&&w<5;w++)
            PDAF_SEC(ek,qk,or_nibs,(uint64_t)(w+1),64,pt,sz,ct);
        double t0=now_ns();
        for(int it=0;it<iters;it++){
            PDAF_SEC(ek,qk,or_nibs,(uint64_t)(it+1),64,pt,sz,ct);
            use_buf(ct,1);
        }
        double el=now_ns()-t0;
        printf("%-14s  %-10d  %-12.1f  %-12.2f\n",
               sz_labels[mi],iters,el/iters,
               (double)sz*iters/(el/1e9)/1e6);
        free(pt);free(ct);
    }
}

#endif /* ENQPY_BENCHMARK */

/* ============================================================================
 * SECTION 10 -- main()
 * ============================================================================ */
#if defined(ENQPY_SELFTEST) || defined(ENQPY_BENCHMARK)

int main(void)
{
    printf("Enqpy(tm) Stream Cipher -- Reference  Rev 4.0\n");
    printf("Canonical Configuration core, Case-1 W generation (ciphertext-only proof profile)\n");
    printf("Copyright (c) 2026 NQP LLC -- Apache License 2.0\n");
    printf("Platform: n=%d, tile_len=%d, W_bytes=%d (window)\n\n",
           ENQPY_MAX_N, ENQPY_TILE_LEN, ENQPY_W_BYTES);

    enqpy_init(0);

#ifdef ENQPY_SELFTEST
    int st = selftest();
    if (st) { printf("Self-test FAILED -- stopping.\n"); return 1; }
    printf("Self-test PASSED.\n\n");
#endif

#ifdef ENQPY_BENCHMARK
    bench_w_gen();
    bench_pdaf_sec();
    printf("\nBenchmark complete.\n");
#endif

    return 0;
}

#endif
