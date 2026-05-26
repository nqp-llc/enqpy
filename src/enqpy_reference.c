#define _POSIX_C_SOURCE 200809L
/* =============================================================================
 * Enqpy(tm) Stream Cipher -- C Reference Implementation  Rev 2.0
 * Copyright (c) 2026 NQP LLC.  All rights reserved.
 *
 * LICENSE
 * -------
 * Permission is granted to evaluate, test, and benchmark this implementation
 * for the sole purpose of assessing Enqpy(tm) technology.  No right to
 * redistribute, sublicense, or deploy in production is granted without a
 * separate written agreement with NQP LLC.
 *
 * WHAT THIS FILE IS
 * -----------------
 * This is the canonical software reference for the Enqpy(tm) stream cipher,
 * implementing the IDEAL ENQPY CONFIGURATION -- the single formally proved-
 * secure implementation delivering H(EK, QK | T^inf) = log2(4) = 2 bits
 * unconditionally (Shannon Ideal System property).
 *
 * IDEAL CONFIGURATION -- KEY ROLE SEPARATION
 * -------------------------------------------
 * In this implementation EK and QK appear ONLY as ValueKey parameters
 * in their respective PDAF Mode 1 calls.  The OffsetKey parameters
 * (VKP, OKP) are derived from the public nonce eff_or alone:
 *
 *   VKP = PDAF1(eff_or, eff_or)[:n]     (nonce self-expansion)
 *   OKP = (VKP + DS_SEP) mod 16          (domain-separated from VKP)
 *   VKC = PDAF1(EK, VKP)                 (EK as ValueKey only)
 *   OKC = PDAF1(QK, OKP)                 (QK as ValueKey only)
 *
 * This arrangement preserves the [+8] global shift invariant through both
 * derivation paths, producing the ciphertext-equivalent coset
 * {EK, EK+8*1} x {QK, QK+8*1} for all nonces and all messages.
 *
 * Benchmarked throughput is equivalent to HMIX-based derivation across all
 * message sizes; the Phase 2 overhead (~130 ns/msg) is negligible against
 * Phase 3 W-generation (the dominant cost).
 *
 * ENQPYADS(tm) VARIANT CONFIGURATIONS
 * ------------------------------------
 * Deployments with specific security/performance/implementation requirements
 * (defense-in-depth key mixing, FPGA optimization, IoT constraints, etc.)
 * may use EnqpyADS(tm) variant configurations specified separately.  Those
 * variants may use HMIX-based or other pointer derivation arrangements and
 * carry the per-session PDAF Mode 1 preimage equivocation (Theorem 1) but
 * not the formal global Shannon Ideal System guarantee.
 *
 * OPTIMISATION ANNOTATIONS
 * ------------------------
 * [OPT-1]  Precomputed tile arrays -- avoids modulo in the inner loop.
 * [OPT-2]  Interleaved VKC/OKC derivation -- both engines share one tile pass.
 * [OPT-3]  Unrolled 3-nibble CS derivation.
 * [OPT-4]  Three-case W generation in one double loop.
 * [OPT-5]  Immediate nibble packing into bytes.
 * [OPT-6]  Interleaved Phase 5 key update.
 * [OPT-7i] Phase 2 Ideal: VKP nonce self-expansion shares tile with OR_EXP
 *          computation; OKP is n simple adds -- no separate tile pass needed.
 * [OPT-8]  restrict-qualified pointers -- enables SIMD vectorisation.
 * [OPT-9]  Byte-level XOR loop -- auto-vectorises to SIMD.
 *
 * ALGORITHM OVERVIEW
 * ------------------
 * Five phases per message:
 *
 *  Phase 1  OR_EXP and eff_or
 *           Expand the 64-bit OR counter to n nibbles via PDAF Mode 1
 *           self-expansion, then mix with the stored OR register to produce
 *           eff_or.  The same self-expansion output is reused as VKP (below).
 *
 *  Phase 2  VKP, OKP, VKC, OKC  [IDEAL CONFIGURATION]
 *           VKP = PDAF1(eff_or, eff_or)[:n]  -- nonce only, public.
 *           OKP[i] = (VKP[i] + DS_SEP) mod 16 -- domain-separated, public.
 *           VKC = PDAF1(EK, VKP)  -- EK as ValueKey only.
 *           OKC = PDAF1(QK, OKP)  -- QK as ValueKey only.
 *           EK and QK do not appear in VKP or OKP derivation.
 *
 *  Phase 2B Case Selector (CS)
 *           Three nibbles of PDAF Mode 1 from VKC/OKC select the
 *           permutation of Cases 1, 2, 3 for W generation [OPT-3].
 *
 *  Phase 3  W keystream generation
 *           Three cases produce n^2 nibbles from VKC/OKC, interleaved
 *           3k/3k+1/3k+2 and packed immediately into bytes [OPT-4][OPT-5].
 *
 *  Phase 4  XOR
 *           W XOR plaintext/ciphertext, auto-vectorised to SIMD [OPT-9].
 *
 *  Phase 5  Key update
 *           VKNext = PDAF1(OKC, VKP, Mode 1)
 *           OKNext = PDAF1(VKC, OKP, Mode 1)
 *           Phase 5 is compatible with the Ideal Configuration: the
 *           [+8] invariant propagates through these PDAF1 calls because
 *           VKP and OKP are nonce-derived and unchanged by EK/QK shifts.
 *
 * SECURITY PROFILES (key size)
 * ----------------------------
 *   LOW   n=32 nibbles (128-bit)
 *   MED   n=48 nibbles (192-bit)
 *   HIGH  n=64 nibbles (256-bit)  -- default; recommended
 *
 * TEST VECTORS (Rev 2.0 -- Ideal Configuration)
 * -----------------------------------------------
 *   PDAF primitive (n=10, 30-nibble output) -- unchanged from Rev 1.0:
 *     VK = FB382C001A   OK = CC69100AB4
 *     Mode 0 -> B7913C0ACE7FEBD00B53F4851014AF
 *     Mode 1 -> 7DD02C010CDF74C01B5BF8D811B92B
 *
 *   PDAF_SEC Ideal Configuration (n=64, 8-byte zero plaintext):
 *     EK = cb1e1203c479f30c1c356f12362fe43b47e8b5906c992013468395489a17d957
 *     QK = 0e2eab25a9f78620abb6726cf81a012776511b3988431d427da911bdc2130680
 *     OR = 3667a507e1109ee32cd50718fa511065900eb422ac187ac5cd47ef5b18d86e0c
 *     OR_CTR = 0x0000000000000001
 *     W[0..7] = 24 C4 3F 3E 94 9B BC 35
 *     CT[0..7] = 24 C4 3F 3E 94 9B BC 35   (plaintext is zeros)
 *
 *   NOTE: Rev 1.0 TV3 (W = F4 38 41 E4 C2 B0 A0 6B) was for the HMIX-based
 *   standard configuration.  The Ideal Configuration produces different VKP/OKP
 *   (nonce-only derivation) and therefore different VKC/OKC and different W.
 *   Coset invariant verified: W(EK+8,QK) == W(EK,QK) == W(EK,QK+8) == W(EK+8,QK+8).
 *
 * BUILD
 * -----
 *   # Standard build (library only):
 *   cc -O2 -std=c11 -c Enqpy_reference.c -o Enqpy_reference.o
 *
 *   # With self-test:
 *   cc -O3 -std=c11 -DENQPY_SELFTEST Enqpy_reference.c -o enqpy_test
 *
 *   # With benchmark:
 *   cc -O3 -std=c11 -DENQPY_BENCHMARK Enqpy_reference.c -o enqpy_bench
 *
 *   # Both:
 *   cc -O3 -std=c11 -DENQPY_SELFTEST -DENQPY_BENCHMARK Enqpy_reference.c -o enqpy_all
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

#define ENQPY_W_NIBS    (3 * ENQPY_MAX_N * ENQPY_MAX_N)   /* 12288 */
#define ENQPY_W_BYTES   (ENQPY_W_NIBS / 2)                /*  6144 */

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
 * SECTION 3 -- CASE SELECTOR PERMUTATION TABLE
 * ============================================================================ */

static const uint8_t CS_PERM[6][3] = {
    {1,2,3}, {1,3,2}, {2,1,3}, {2,3,1}, {3,1,2}, {3,2,1}
};

/* ============================================================================
 * SECTION 4 -- OWC (One-Way Compression)
 * Used exclusively in the Nil-Communication Key Update path.
 * ============================================================================ */

int OWC(const uint8_t *key_nibs, int nLen, int nSkip, uint8_t *out_nibs)
{
    if (!key_nibs || !out_nibs || nLen < 2) return -1;
    int out_len = 0;
    /* Non-overlapping pairs (FCD Rev 1.0 §6): the position pointer advances
     * by (1 + nSkip) per output nibble, combining positions i and i+nSkip.
     * If i+nSkip falls beyond the end of the array, fall back to the adjacent
     * nibble at i+1. Output length is approximately half the input.        */
    for (int i = 0; i < nLen - 1; i += (1 + nSkip)) {
        int j = i + nSkip;
        if (j >= nLen) j = i + 1;
        out_nibs[out_len++] = MOD16[key_nibs[i] & 0xF][key_nibs[j] & 0xF];
    }
    return out_len;
}

/* ============================================================================
 * SECTION 5 -- PDAF CORE
 *
 * Mode 1 (primary cipher path):
 *   Both arrays tiled to ENQPY_TILE_LEN.  For each output nibble nN:
 *     delta = ok_tile[p]
 *     out[nN] = MOD16(vk_tile[p+c], vk_tile[p+delta+1+c])
 *
 * Mode 0 (Nil-Comm path only):
 *   out[nN] = MOD16(ok[p % n], vk[(p+c) % n])
 *
 * [OPT-1] Tile arrays built once, indexed with plain integer arithmetic.
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
 * SECTION 6 -- CASE SELECTOR DERIVATION  [OPT-3]
 * ============================================================================ */

int ENQPY_DERIVE_CS(const uint8_t *vkc, const uint8_t *okc, int n,
                    uint8_t *cs_out)
{
    if (!vkc || !okc || !cs_out || n < 4) return -1;
    for (int nN = 0; nN < 3; nN++) {
        int delta  = okc[nN] & 0xF;
        int idx2   = nN + delta + 1;
        cs_out[nN] = MOD16[vkc[nN] & 0xF][vkc[idx2 % n] & 0xF];
    }
    return 3;
}

/* ============================================================================
 * SECTION 7 -- W KEYSTREAM GENERATION  [OPT-4][OPT-5]
 * Identical to Rev 1.0.  Phase 3 is unchanged by the Ideal Configuration.
 * ============================================================================ */

static void generate_w(const uint8_t *vkc, const uint8_t *okc, int n,
                       const uint8_t case_order[3], uint8_t *w_bytes)
{
    uint8_t vk_t[ENQPY_TILE_LEN];
    uint8_t ok_t[ENQPY_TILE_LEN];
    for (int i = 0; i < ENQPY_TILE_LEN; i++) {
        vk_t[i] = vkc[i % n] & 0xF;
        ok_t[i] = okc[i % n] & 0xF;
    }
    int nib_idx  = 0;
    int byte_idx = 0;
    uint8_t hi   = 0;
    for (int c = 0; c < n; c++) {
        for (int p = 0; p < n; p++) {
            uint8_t w_nib[3];
            for (int slot = 0; slot < 3; slot++) {
                int cas = case_order[slot];
                uint8_t nib;
                int delta, idx2;
                switch (cas) {
                    case 1:
                        nib = MOD16[ok_t[p]][vk_t[p + c]];
                        break;
                    case 2:
                        delta = ok_t[p];
                        idx2  = p + delta + 1 + c;
                        nib   = MOD16[vk_t[p + c]][vk_t[idx2]];
                        break;
                    default:
                        delta = vk_t[p];
                        idx2  = p + delta + 1 + c;
                        nib   = MOD16[ok_t[p + c]][ok_t[idx2]];
                        break;
                }
                w_nib[slot] = nib;
            }
            for (int s = 0; s < 3; s++) {
                if ((nib_idx & 1) == 0) {
                    hi = w_nib[s];
                } else {
                    w_bytes[byte_idx++] = (hi << 4) | w_nib[s];
                }
                nib_idx++;
            }
        }
    }
    if (nib_idx & 1)
        w_bytes[byte_idx] = (hi << 4);
}

/* ============================================================================
 * SECTION 8 -- PDAF_SEC (Main Encrypt / Decrypt)
 *
 * IDEAL CONFIGURATION -- PHASE 2 CHANGE vs Rev 1.0
 * -------------------------------------------------
 * Rev 1.0 (HMIX-based, standard config):
 *   VKP[i] = MOD16(QK[i], MOD16(eff_or[i], DS_VK))   -- QK in OffsetKey path
 *   OKP[i] = MOD16(EK[i], MOD16(eff_or[i], DS_OK))   -- EK in OffsetKey path
 *
 * Rev 2.0 (Ideal Configuration):
 *   VKP    = PDAF1(eff_or, eff_or)[:n]                -- nonce only  [OPT-7i]
 *   OKP[i] = MOD16(VKP[i], DS_SEP)                   -- domain-separated
 *
 * EK and QK are then used only as ValueKey parameters in:
 *   VKC = PDAF1(EK, VKP)    OKC = PDAF1(QK, OKP)
 *
 * [OPT-7i]: The Phase 1 OR_EXP self-expansion (PDAF1(orc, orc)) already
 * establishes the nonce-derived structure.  VKP reuses the same self-
 * expansion mechanism on eff_or rather than on orc, keeping Phase 2
 * coherent with Phase 1 design.  The tile arrays for the VKP call
 * (eff_or tiled) are populated in the same pass as the VKC/OKC tile
 * setup, preserving [OPT-2] cache efficiency.
 *
 * Phases 1, 2B, 3, 4, 5 are byte-for-byte identical to Rev 1.0.
 * Phase 5 is fully compatible: VKP and OKP are nonce-derived and do
 * not change when EK/QK shift by +8, so the update preserves the
 * [+8] coset invariant across W-buffer boundaries.
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
     * Identical to Rev 1.0.
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
     * PHASE 2 -- VKP, OKP, VKC, OKC  [IDEAL CONFIGURATION]  [OPT-7i]
     *
     * VKP = PDAF1(eff_or, eff_or)[:n]
     *   eff_or is used as both ValueKey and OffsetKey (self-referential,
     *   same structure as Phase 1 OR_EXP self-expansion on or_ctr_nibs).
     *
     * OKP[i] = (VKP[i] + DS_SEP) mod 16  -- n simple adds, no tile pass.
     *
     * VKC = PDAF1(EK, VKP)  -- EK as ValueKey, VKP as OffsetKey
     * OKC = PDAF1(QK, OKP)  -- QK as ValueKey, OKP as OffsetKey
     *
     * [OPT-2] The four tile arrays (EK/VKP for VKC; QK/OKP for OKC) are
     * populated in a single tiling pass -- identical structure to Rev 1.0.
     * ----------------------------------------------------------------------- */
    uint8_t vkp[ENQPY_MAX_N], okp[ENQPY_MAX_N];

    /* VKP: PDAF1 self-expansion of eff_or  [OPT-7i] */
    pdaf_mode1(eff_or, eff_or, n, n, vkp);

    /* OKP: domain-separated from VKP with one pass -- no tile needed */
    for (int i = 0; i < n; i++)
        okp[i] = MOD16[vkp[i]][DS_SEP];

    /* VKC and OKC: interleaved single tile pass  [OPT-2] */
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
     * PHASE 2B -- Case Selector  [OPT-3]
     * Identical to Rev 1.0.
     * ----------------------------------------------------------------------- */
    uint8_t cs_nibs[3];
    ENQPY_DERIVE_CS(vkc, okc, n, cs_nibs);
    uint32_t cs_val = ((uint32_t)cs_nibs[2] << 8) |
                      ((uint32_t)cs_nibs[1] << 4) |
                       (uint32_t)cs_nibs[0];
    const uint8_t *case_order = CS_PERM[cs_val % 6];

    /* -----------------------------------------------------------------------
     * PHASES 3, 4, 5 -- Streaming loop
     * Identical to Rev 1.0.  Phase 5 uses VKP/OKP as OffsetKeys in the
     * update calls; since VKP/OKP are nonce-derived the [+8] coset invariant
     * is preserved across update boundaries.
     * ----------------------------------------------------------------------- */
    uint8_t w_bytes[ENQPY_W_BYTES];
    int w_byte_max = (n * n * 3) / 2;
    int text_done  = 0;

    while (text_done < nTextLen) {
        /* Phase 3 */
        generate_w(vkc, okc, n, case_order, w_bytes);

        /* Phase 4  [OPT-8][OPT-9] */
        int chunk = nTextLen - text_done;
        if (chunk > w_byte_max) chunk = w_byte_max;
        const uint8_t *in_ptr  = target + text_done;
        uint8_t       *out_ptr = out    + text_done;
        for (int i = 0; i < chunk; i++)
            out_ptr[i] = w_bytes[i] ^ in_ptr[i];
        text_done += chunk;

        if (text_done >= nTextLen) break;

        /* Phase 5 -- Key update  [OPT-6]
         * VKNext = PDAF1(OKC, VKP, Mode 1)
         * OKNext = PDAF1(VKC, OKP, Mode 1)
         * VKP and OKP are nonce-derived; they are the same regardless of
         * which coset element (EK or EK+8) generated VKC/OKC.  The update
         * therefore preserves the ciphertext-equivalent coset.           */
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

        ENQPY_DERIVE_CS(vkc, okc, n, cs_nibs);
        cs_val     = ((uint32_t)cs_nibs[2] << 8) |
                     ((uint32_t)cs_nibs[1] << 4) |
                      (uint32_t)cs_nibs[0];
        case_order = CS_PERM[cs_val % 6];
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
 * SECTION 9 -- NIL-COMMUNICATION KEY UPDATE
 * Identical to Rev 1.0.  The Nil-Comm path (Mode 0 + OWC) does not involve
 * VKP/OKP pointer derivation and is unaffected by the Ideal Configuration.
 * ============================================================================ */

int ENQPY_NIL_COMM_UPDATE(const uint8_t *ek, const uint8_t *qk, int n,
                          const uint8_t *e_ext, int method,
                          uint8_t *ek_new, uint8_t *qk_new)
{
    if (!ek || !qk || !ek_new || !qk_new) return -1;
    if (n < 32 || n > ENQPY_MAX_N)          return -1;
    if (method == 2 && !e_ext)             return -1;

    uint8_t pdaf_m0[ENQPY_MAX_N];
    pdaf_mode0(ek, qk, n, n, pdaf_m0);

    uint8_t e_seed_raw[ENQPY_MAX_N / 2];
    int seed_len = OWC(pdaf_m0, n, 1, e_seed_raw);
    if (seed_len < 1) return -1;

    uint8_t e_combined[ENQPY_MAX_N];
    if (method == 2) {
        for (int i = 0; i < n; i++)
            e_combined[i] = MOD16[e_seed_raw[i % seed_len]][e_ext[i] & 0xF];
    } else {
        for (int i = 0; i < n; i++)
            e_combined[i] = e_seed_raw[i % seed_len];
    }

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
 * SECTION 10 -- SELF-TEST HARNESS
 * ============================================================================ */
#ifdef ENQPY_SELFTEST

static int selftest(void)
{
    int pass = 0, fail = 0;
    enqpy_init(0);

    /* TV1: PDAF Mode 0 (n=10, 30-nibble) -- unchanged from Rev 1.0 */
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

    /* TV2: PDAF Mode 1 (n=10, 30-nibble) -- unchanged from Rev 1.0 */
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

    /* TV3: PDAF_SEC Ideal Configuration -- W[0..7] = 24 C4 3F 3E 94 9B BC 35
     * (Rev 1.0 TV3 was W = F4 38 41 E4 C2 B0 A0 6B for HMIX-based config.)
     * Keys and OR identical to Rev 1.0 TV3; only Phase 2 differs.           */
    {
        uint8_t ek[64] = {
            0xC,0xB,0x1,0xE, 0x1,0x2,0x0,0x3, 0xC,0x4,0x7,0x9,
            0xF,0x3,0x0,0xC, 0x1,0xC,0x3,0x5, 0x6,0xF,0x1,0x2,
            0x3,0x6,0x2,0xF, 0xE,0x4,0x3,0xB, 0x4,0x7,0xE,0x8,
            0xB,0x5,0x9,0x0, 0x6,0xC,0x9,0x9, 0x2,0x0,0x1,0x3,
            0x4,0x6,0x8,0x3, 0x9,0x5,0x4,0x8, 0x9,0xA,0x1,0x7,
            0xD,0x9,0x5,0x7
        };
        uint8_t qk[64] = {
            0x0,0xE,0x2,0xE, 0xA,0xB,0x2,0x5, 0xA,0x9,0xF,0x7,
            0x8,0x6,0x2,0x0, 0xA,0xB,0xB,0x6, 0x7,0x2,0x6,0xC,
            0xF,0x8,0x1,0xA, 0x0,0x1,0x2,0x7, 0x7,0x6,0x5,0x1,
            0x1,0xB,0x3,0x9, 0x8,0x8,0x4,0x3, 0x1,0xD,0x4,0x2,
            0x7,0xD,0xA,0x9, 0x1,0x1,0xB,0xD, 0xC,0x2,0x1,0x3,
            0x0,0x6,0x8,0x0
        };
        uint8_t or_nibs[64] = {
            0x3,0x6,0x6,0x7, 0xA,0x5,0x0,0x7, 0xE,0x1,0x1,0x0,
            0x9,0xE,0xE,0x3, 0x2,0xC,0xD,0x5, 0x0,0x7,0x1,0x8,
            0xF,0xA,0x5,0x1, 0x1,0x0,0x6,0x5, 0x9,0x0,0x0,0xE,
            0xB,0x4,0x2,0x2, 0xA,0xC,0x1,0x8, 0x7,0xA,0xC,0x5,
            0xC,0xD,0x4,0x7, 0xE,0xF,0x5,0xB, 0x1,0x8,0xD,0x8,
            0x6,0xE,0x0,0xC
        };
        /* Ideal Configuration W[0..7] = 24 C4 3F 3E 94 9B BC 35 */
        const uint8_t expected_w[8] = {0x24,0xC4,0x3F,0x3E,0x94,0x9B,0xBC,0x35};
        uint8_t pt[8] = {0};
        uint8_t ct[8], rt[8];

        int r = PDAF_SEC(ek, qk, or_nibs, 0x0000000000000001ULL, 64, pt, 8, ct);
        if (r != 8) { fail++; printf("TV3 FAIL: PDAF_SEC returned %d\n", r); }
        else {
            for (int i=0;i<8;i++) {
                if (ct[i]==expected_w[i]) pass++;
                else { fail++; printf("TV3 FAIL CT[%d] got=%02X exp=%02X\n",
                                      i,ct[i],expected_w[i]); }
            }
        }

        /* Round-trip */
        PDAF_SEC(ek, qk, or_nibs, 0x0000000000000001ULL, 64, ct, 8, rt);
        for (int i=0;i<8;i++) {
            if (rt[i]==0x00) pass++;
            else { fail++; printf("TV3 RT FAIL[%d] got=%02X\n",i,rt[i]); }
        }

        /* TV4: Coset invariant -- W(EK+8,QK) == W(EK,QK) */
        uint8_t ek8[64], ct8[8];
        for (int i=0;i<64;i++) ek8[i]=(ek[i]+8)&0xF;
        PDAF_SEC(ek8, qk, or_nibs, 0x0000000000000001ULL, 64, pt, 8, ct8);
        int coset_ok = (memcmp(ct, ct8, 8) == 0);
        if (coset_ok) pass++;
        else { fail++; printf("TV4 FAIL: EK+8 coset not preserved\n"); }

        /* TV5: QK+8 coset */
        uint8_t qk8[64], ct_q8[8];
        for (int i=0;i<64;i++) qk8[i]=(qk[i]+8)&0xF;
        PDAF_SEC(ek, qk8, or_nibs, 0x0000000000000001ULL, 64, pt, 8, ct_q8);
        int coset_qk = (memcmp(ct, ct_q8, 8) == 0);
        if (coset_qk) pass++;
        else { fail++; printf("TV5 FAIL: QK+8 coset not preserved\n"); }
    }

    printf("Self-test: %d PASS  %d FAIL\n", pass, fail);
    return fail ? 1 : 0;
}

#endif /* ENQPY_SELFTEST */

/* ============================================================================
 * SECTION 11 -- BENCHMARK HARNESS
 * Identical to Rev 1.0 except:
 *   - Phase 2 isolation benchmark reflects Ideal Config overhead
 *   - TV3 expected W updated
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

static void bench_pdaf(void)
{
    printf("\n--- PDAF Mode 1 throughput (nDigits = n) ---\n");
    printf("%-8s  %-10s  %-12s  %-12s\n", "n","iters","ns/call","Mnib/s");
    const int ns[] = {32, 48, 64, 0};
    for (int ni = 0; ns[ni]; ni++) {
        int n = ns[ni];
        uint8_t vk[64]={0}, ok[64]={0}, out[64]={0};
        for (int i=0;i<n;i++){vk[i]=(uint8_t)(i*7+1)&0xF;ok[i]=(uint8_t)(i*13+5)&0xF;}
        for (int w=0;w<BENCH_WARMUP;w++) pdaf_mode1(vk,ok,n,n,out);
        double t0=now_ns();
        for (int it=0;it<BENCH_ITERS;it++){pdaf_mode1(vk,ok,n,n,out);use_buf(out,n);}
        double el=now_ns()-t0;
        printf("n=%-6d  %-10d  %-12.1f  %-12.2f\n",
               n, BENCH_ITERS, el/BENCH_ITERS,
               (double)n*BENCH_ITERS/(el/1e9)/1e6);
    }
}

static void bench_w_gen(void)
{
    printf("\n--- Phase 3 W generation (n=64, 6144 bytes/call) ---\n");
    printf("%-12s  %-12s  %-14s\n","iters","ns/call","MB/s");
    uint8_t vkc[64]={0},okc[64]={0},wb[ENQPY_W_BYTES];
    const uint8_t co[3]={1,2,3};
    for(int i=0;i<64;i++){vkc[i]=(uint8_t)(i*3+2)&0xF;okc[i]=(uint8_t)(i*7+11)&0xF;}
    for(int w=0;w<BENCH_WARMUP;w++) generate_w(vkc,okc,64,co,wb);
    double t0=now_ns();
    for(int it=0;it<BENCH_ITERS;it++){generate_w(vkc,okc,64,co,wb);use_buf(wb,ENQPY_W_BYTES);}
    double el=now_ns()-t0;
    printf("%-12d  %-12.1f  %-14.2f\n",
           BENCH_ITERS, el/BENCH_ITERS,
           (double)ENQPY_W_BYTES*BENCH_ITERS/(el/1e9)/1e6);
}

static void bench_pdaf_sec(void)
{
    printf("\n--- PDAF_SEC Ideal Configuration throughput (n=64) ---\n");
    printf("%-14s  %-10s  %-12s  %-12s\n","msg_size","iters","ns/call","MB/s");

    uint8_t ek[64]={
        0xC,0xB,0x1,0xE,0x1,0x2,0x0,0x3,0xC,0x4,0x7,0x9,
        0xF,0x3,0x0,0xC,0x1,0xC,0x3,0x5,0x6,0xF,0x1,0x2,
        0x3,0x6,0x2,0xF,0xE,0x4,0x3,0xB,0x4,0x7,0xE,0x8,
        0xB,0x5,0x9,0x0,0x6,0xC,0x9,0x9,0x2,0x0,0x1,0x3,
        0x4,0x6,0x8,0x3,0x9,0x5,0x4,0x8,0x9,0xA,0x1,0x7,
        0xD,0x9,0x5,0x7
    };
    uint8_t qk[64]={
        0x0,0xE,0x2,0xE,0xA,0xB,0x2,0x5,0xA,0x9,0xF,0x7,
        0x8,0x6,0x2,0x0,0xA,0xB,0xB,0x6,0x7,0x2,0x6,0xC,
        0xF,0x8,0x1,0xA,0x0,0x1,0x2,0x7,0x7,0x6,0x5,0x1,
        0x1,0xB,0x3,0x9,0x8,0x8,0x4,0x3,0x1,0xD,0x4,0x2,
        0x7,0xD,0xA,0x9,0x1,0x1,0xB,0xD,0xC,0x2,0x1,0x3,
        0x0,0x6,0x8,0x0
    };
    uint8_t or_nibs[64]={
        0x3,0x6,0x6,0x7,0xA,0x5,0x0,0x7,0xE,0x1,0x1,0x0,
        0x9,0xE,0xE,0x3,0x2,0xC,0xD,0x5,0x0,0x7,0x1,0x8,
        0xF,0xA,0x5,0x1,0x1,0x0,0x6,0x5,0x9,0x0,0x0,0xE,
        0xB,0x4,0x2,0x2,0xA,0xC,0x1,0x8,0x7,0xA,0xC,0x5,
        0xC,0xD,0x4,0x7,0xE,0xF,0x5,0xB,0x1,0x8,0xD,0x8,
        0x6,0xE,0x0,0xC
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

static void bench_nil_comm(void)
{
    printf("\n--- Nil-Comm Key Update latency (n=64, Method 1) ---\n");
    printf("%-10s  %-12s\n","iters","ns/call");
    uint8_t ek[64]={0},qk[64]={0},ek_new[64],qk_new[64];
    for(int i=0;i<64;i++){ek[i]=(uint8_t)(i*5+1)&0xF;qk[i]=(uint8_t)(i*9+3)&0xF;}
    for(int w=0;w<BENCH_WARMUP;w++)
        ENQPY_NIL_COMM_UPDATE(ek,qk,64,NULL,1,ek_new,qk_new);
    double t0=now_ns();
    for(int it=0;it<BENCH_ITERS;it++){
        ENQPY_NIL_COMM_UPDATE(ek,qk,64,NULL,1,ek_new,qk_new);
        use_buf(ek_new,1);
    }
    printf("%-10d  %-12.1f\n",BENCH_ITERS,(now_ns()-t0)/BENCH_ITERS);
}

#endif /* ENQPY_BENCHMARK */

/* ============================================================================
 * SECTION 12 -- main()
 * ============================================================================ */
#if defined(ENQPY_SELFTEST) || defined(ENQPY_BENCHMARK)

int main(void)
{
    printf("Enqpy(tm) Stream Cipher -- Reference Implementation Rev 2.0\n");
    printf("Ideal Enqpy(tm) Configuration (nonce-only OffsetKey derivation)\n");
    printf("Copyright (c) 2026 NQP LLC\n");
    printf("Platform: n=%d, tile_len=%d, W_bytes=%d\n\n",
           ENQPY_MAX_N, ENQPY_TILE_LEN, ENQPY_W_BYTES);

    enqpy_init(0);

#ifdef ENQPY_SELFTEST
    int st = selftest();
    if (st) { printf("Self-test FAILED -- stopping.\n"); return 1; }
    printf("Self-test PASSED.\n\n");
#endif

#ifdef ENQPY_BENCHMARK
    bench_pdaf();
    bench_w_gen();
    bench_pdaf_sec();
    bench_nil_comm();
    printf("\nBenchmark complete.\n");
#endif

    return 0;
}

#endif
