/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2026 NQP LLC (Paul McGough) */
/* ============================================================================
 * Enqpy(tm)-HKU -- Deployable Single-Window-Per-Credential API  Rev 4.0
 *
 * WHAT THIS FILE IS
 * -----------------
 * This is the DEPLOYABLE wrapper around the Enqpy core (enqpy_reference.c).
 * It exists to enforce, in code, the deployment rule the Rev 4.0 documents make
 * normative:
 *
 *   FCD 8.10.1 (single-window credential rule):  a given (EK,QK) credential
 *   encrypts AT MOST ONE Enqpy window (2,048 bytes at HIGH); Phase 5 MUST NOT
 *   be used to continue a single message beyond one window under the same
 *   credential.  Long messages are segmented at the application layer into
 *   independent single-window messages, each under its own independently
 *   generated (EK_j,QK_j), its own nonce, and its own encrypt-then-MAC tag.
 *
 *   Proof 17.9 / Proposition 2 (unconditional cross-segment non-accumulation):
 *   under single-window segmentation with mutually independent segment keys,
 *   known-plaintext observations do not accumulate across segments.  This path
 *   rests on the ciphertext-only theorems plus key independence and invokes no
 *   reset lemma (Theorem 4 / Lemma 9 are needed only for the alternative
 *   continuing-key-state path).
 *
 * DIVISION OF RESPONSIBILITY
 * --------------------------
 *   enqpy_core (enqpy_reference.c):  MAY expose Phase 5 multi-window
 *       continuation -- it is the ciphertext-only proof artifact and is left
 *       byte-identical.  Direct use of PDAF_SEC on > one window is a
 *       core/research operation, not a deployment operation.
 *   enqpy_hku  (this wrapper):  the deployable API.  It REFUSES a single
 *       segment longer than one window and segments long messages into
 *       independent single-window messages.  It NEVER calls the core in a way
 *       that triggers Phase 5.
 *
 * WHAT THIS WRAPPER DOES NOT DO
 * -----------------------------
 *   It does not save the window in which known plaintext lands -- within-window
 *   determinability is structural (FCD 8.5 / proof 16).  Segmentation confines
 *   an exposure to the single segment it occurs in; it does not recover it.
 * ============================================================================ */
#ifndef ENQPY_HKU_H
#define ENQPY_HKU_H

#include <stdint.h>
#include <stddef.h>

/* HIGH profile parameters (the deployable profile). */
#define ENQPY_HKU_N            64                       /* nibbles per key      */
#define ENQPY_HKU_WINDOW_BYTES 2048                     /* n^2/2 -- one window  */
#define ENQPY_HKU_TAG_BYTES    32                       /* HMAC-SHA-256 tag     */
#define ENQPY_HKU_OR_NIBS      ENQPY_HKU_N              /* nonce length, nibbles*/

/* Return / error codes. */
enum {
    ENQPY_HKU_OK           =  0,
    ENQPY_HKU_ERR_NULL     = -1,   /* null argument                            */
    ENQPY_HKU_ERR_TOO_LONG = -2,   /* single segment > one window: REFUSED     */
    ENQPY_HKU_ERR_CRED     = -3,   /* credential source failed, or EK == QK    */
    ENQPY_HKU_ERR_CORE     = -4,   /* core PDAF_SEC reported an error          */
    ENQPY_HKU_ERR_MAC      = -5,   /* MAC verification failed (open)           */
    ENQPY_HKU_ERR_FORMAT   = -6,   /* malformed frame (open)                   */
    ENQPY_HKU_ERR_BUFFER   = -7    /* output buffer too small                  */
};

/* A FRESH, INDEPENDENT per-segment credential.
 *
 * The caller MUST fill ek/qk from a CSPRNG (or an out-of-band one-time store),
 * INDEPENDENTLY for every segment.  The wrapper never derives one segment's
 * credential from another (no key chain, no counter schedule, no KDF/PRG
 * expansion of a common seed) -- credential independence is exactly the
 * security-bearing property of Proposition 2.  EK MUST differ from QK. */
typedef struct {
    uint8_t  ek[ENQPY_HKU_N];        /* n nibbles, value in low 4 bits         */
    uint8_t  qk[ENQPY_HKU_N];        /* n nibbles; MUST differ from ek         */
    uint8_t  or_nibs[ENQPY_HKU_N];   /* n nibbles, public per-message nonce    */
    uint64_t or_ctr;                 /* monotonic per-credential nonce counter */
} enqpy_hku_cred;

/* Caller-supplied source of FRESH INDEPENDENT credentials, one per segment.
 * Return 0 on success, nonzero on failure.  seg_index is informational only;
 * the returned credential MUST NOT be a deterministic function of it.
 * On the receiver, this callback supplies the matching out-of-band ek/qk. */
typedef int (*enqpy_hku_cred_fn)(uint32_t seg_index, enqpy_hku_cred *out, void *ctx);

/* ---------------------------------------------------------------------------
 * GUARDED SINGLE-SEGMENT PRIMITIVE
 *
 * Seals / opens exactly ONE segment of at most ENQPY_HKU_WINDOW_BYTES under
 * ONE credential, with encrypt-then-MAC.  seal REFUSES pt_len > one window
 * with ENQPY_HKU_ERR_TOO_LONG: the deployable API does not run Phase 5 to
 * continue past a window.  meta/meta_len (may be NULL/0) are authenticated.
 * On seal, ct receives pt_len bytes and tag receives ENQPY_HKU_TAG_BYTES.
 * On open, the tag is verified (constant-time) BEFORE pt is written.
 * --------------------------------------------------------------------------- */
int enqpy_hku_seal_segment(const enqpy_hku_cred *cred,
                           const uint8_t *pt, size_t pt_len,
                           const uint8_t *meta, size_t meta_len,
                           uint8_t *ct, uint8_t *tag);

int enqpy_hku_open_segment(const enqpy_hku_cred *cred,
                           const uint8_t *ct, size_t ct_len,
                           const uint8_t *meta, size_t meta_len,
                           const uint8_t *tag,
                           uint8_t *pt);

/* ---------------------------------------------------------------------------
 * LONG-MESSAGE SEGMENTATION (the recommended default deployment)
 *
 * seal: partitions pt into ceil(pt_len / window) independent single-window
 *       segments, pulling a FRESH INDEPENDENT credential per segment from
 *       cred_fn, and writes a self-describing frame to out.  Per-segment
 *       metadata (segment index, total segments, segment length) is bound by
 *       the MAC, so reordering / truncation / splice are detected.
 * open: parses the frame, pulls the matching credential per segment from
 *       cred_fn, verifies each tag, decrypts, and reassembles into pt.
 *
 * out_cap / pt_cap bound the output buffers; *out_len / *pt_len receive the
 * number of bytes written.  Pass out=NULL to have *out_len return the exact
 * frame size required (sizing pass). */
int enqpy_hku_seal_message(enqpy_hku_cred_fn cred_fn, void *ctx,
                           const uint8_t *pt, size_t pt_len,
                           uint8_t *out, size_t out_cap, size_t *out_len);

int enqpy_hku_open_message(enqpy_hku_cred_fn cred_fn, void *ctx,
                           const uint8_t *frame, size_t frame_len,
                           uint8_t *pt, size_t pt_cap, size_t *pt_len);

#endif /* ENQPY_HKU_H */
