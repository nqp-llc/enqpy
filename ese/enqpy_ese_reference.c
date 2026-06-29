/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2026 NQP LLC (Paul McGough) */
#define _POSIX_C_SOURCE 200809L
/* =============================================================================
 * enqpy_ese_reference.c -- Enqpy(tm) ESE KPA-Hardening reference  (Rev 5.0)
 *
 * OPTIONAL hardening layer for known-plaintext-sensitive deployments. It wraps
 * the base cipher's ciphertext-only guarantee with a secret, fresh, per-record
 * keyed permutation:
 *
 *      C = S( P (+) W1 ) (+) W2        (+) = mod-16 nibble add
 *
 *   W1, W2 : two INDEPENDENT Enqpy keystreams (PDAF_SEC under separate per-record
 *            credentials -- see enqpy_reference.c). Supplied here as inputs, which
 *            is exactly the interface of the silicon module enqpy_ese_hardening.vhd.
 *   KSW    : switch bits for S, a third independent keystream.
 *   S      : the CANONICAL permutation = a keyed butterfly / Benes conditional-
 *            exchange network (PASSES passes x LOGN stages) over the NB-byte window.
 *            Chosen as canonical because it is bit-identical in C and HDL, is built
 *            only from Enqpy keystream (no foreign primitive), and is the construction
 *            validated by the A3 falsification sweep. Decryption reuses the same
 *            switch bits with the stage order reversed.
 *
 * SECURITY SCOPE (read this): the base cipher proof is ciphertext-only. This layer
 * RAISES the known-plaintext threshold (secret-S equivocation held past known-S in
 * the n=4 determinacy sweep); it does NOT restore information-theoretic security
 * under KP. The strong, proven claim remains the base cipher's ciphertext-only
 * plaintext equivocation. This file is the SW counterpart of enqpy_ese_hardening.vhd
 * and produces the shared interop KAT that the VHDL reproduces byte-for-byte.
 * ===========================================================================*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ---- mod-16 (nibble) add/sub: byte = two independent nibbles, carry-free ---- */
static inline uint8_t z16_add(uint8_t a, uint8_t b){
    return (uint8_t)((((a>>4)+(b>>4))&0xF)<<4 | (((a&0xF)+(b&0xF))&0xF));
}
static inline uint8_t z16_sub(uint8_t a, uint8_t b){
    return (uint8_t)((((a>>4)-(b>>4))&0xF)<<4 | (((a&0xF)-(b&0xF))&0xF));
}

/* ---- canonical S: keyed butterfly network (matches enqpy_ese_hardening.vhd) ----
 * fwd=1 applies stages forward (encrypt S); fwd=0 reverses stage order (S^-1).
 * ksw is one bit per pair per stage: length PASSES*LOGN*(NB/2). */
static void butterfly(uint8_t *s, int NB, int LOGN, int PASSES,
                      const uint8_t *ksw, int fwd){
    const int NSTAGE = PASSES*LOGN;
    for(int n=0; n<NSTAGE; n++){
        int st   = fwd ? n : (NSTAGE-1-n);
        int dist = 1 << (st % LOGN);
        int base = st * (NB/2);
        for(int i=0; i<NB; i++){
            if(((i/dist) & 1) == 0){                       /* lower of the pair  */
                int r = (i/(2*dist))*dist + (i % dist);    /* compacted ctrl idx */
                if(ksw[base + r]){
                    uint8_t t = s[i]; s[i] = s[i+dist]; s[i+dist] = t;
                }
            }
        }
    }
}

/* ---- ESE encrypt / decrypt (one window) ---- */
void ese_encrypt(const uint8_t *P, const uint8_t *W1, const uint8_t *W2,
                 const uint8_t *KSW, int NB, int LOGN, int PASSES, uint8_t *CT){
    uint8_t buf[2048];
    for(int i=0;i<NB;i++) buf[i] = z16_add(P[i], W1[i]);   /* a = P + W1     */
    butterfly(buf, NB, LOGN, PASSES, KSW, 1);              /* b = S(a)       */
    for(int i=0;i<NB;i++) CT[i] = z16_add(buf[i], W2[i]);  /* C = b + W2     */
}
void ese_decrypt(const uint8_t *CT, const uint8_t *W1, const uint8_t *W2,
                 const uint8_t *KSW, int NB, int LOGN, int PASSES, uint8_t *P){
    uint8_t buf[2048];
    for(int i=0;i<NB;i++) buf[i] = z16_sub(CT[i], W2[i]);  /* b = C - W2     */
    butterfly(buf, NB, LOGN, PASSES, KSW, 0);              /* a = S^-1(b)    */
    for(int i=0;i<NB;i++) P[i] = z16_sub(buf[i], W1[i]);   /* P = a - W1     */
}

/* ---- deterministic test vectors (replicated bit-for-bit in the VHDL xcheck tb) ---- */
static void gen_vectors(int NB, int LOGN, int PASSES,
                        uint8_t *P, uint8_t *W1, uint8_t *W2, uint8_t *KSW){
    for(int i=0;i<NB;i++){
        P[i]  = (uint8_t)(( 7*i + 1) & 0xFF);
        W1[i] = (uint8_t)((11*i + 3) & 0xFF);
        W2[i] = (uint8_t)((13*i + 5) & 0xFF);
    }
    int klen = PASSES*LOGN*(NB/2);
    for(int k=0;k<klen;k++)
        KSW[k] = (uint8_t)((k ^ (k>>1) ^ (k>>2)) & 1);     /* simple, reproducible */
}

static int run_profile(const char *name, int NB, int LOGN, int PASSES){
    uint8_t P[2048], W1[2048], W2[2048], CT[2048], RT[2048];
    uint8_t KSW[3*11*1024];
    gen_vectors(NB, LOGN, PASSES, P, W1, W2, KSW);

    ese_encrypt(P, W1, W2, KSW, NB, LOGN, PASSES, CT);
    ese_decrypt(CT, W1, W2, KSW, NB, LOGN, PASSES, RT);

    int rt_ok = (memcmp(P, RT, NB) == 0);

    /* permutation validity: W1=W2=0, tagged window -> S(P) is a bijection */
    uint8_t Z[2048]={0}, TAG[2048], CT2[2048]; int seen[2048]={0}, perm_ok=1;
    for(int i=0;i<NB;i++) TAG[i]=(uint8_t)(i & 0xFF);
    ese_encrypt(TAG, Z, Z, KSW, NB, LOGN, PASSES, CT2);
    for(int i=0;i<NB;i++) seen[CT2[i]]++;
    for(int i=0;i<NB;i++) if(NB<=256 && seen[i]!=1) perm_ok=0;

    /* interop checksum + first 8 ciphertext bytes (the shared KAT) */
    uint8_t cks=0; for(int i=0;i<NB;i++) cks ^= CT[i];
    printf("  %-22s NB=%-4d LOGN=%-2d PASSES=%d  roundtrip=%s  ",
           name, NB, LOGN, PASSES, rt_ok?"PASS":"FAIL");
    if(NB<=256) printf("perm=%s  ", perm_ok?"PASS":"FAIL"); else printf("perm=n/a  ");
    printf("KAT_cksum=%02X  CT[0..7]=", cks);
    for(int i=0;i<8;i++) printf("%02X", CT[i]);
    printf("\n");
    return rt_ok && (NB>256 || perm_ok);
}

int main(void){
    printf("Enqpy(tm) ESE KPA-Hardening reference (Rev 5.0) -- Apache-2.0\n");
    printf("C = S(P + W1) + W2 ; S = keyed butterfly network\n");
    int ok = 1;
    ok &= run_profile("xcheck (vs VHDL)", 64,   6, 3);   /* matches enqpy_ese_tb.vhd */
    ok &= run_profile("HIGH window",      2048, 11, 3);  /* production HIGH profile  */
    printf("%s\n", ok ? "ALL ESE SELF-TESTS PASS" : "SELF-TEST FAILURE");
    return ok ? 0 : 1;
}
