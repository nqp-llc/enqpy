/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2026 NQP LLC (Paul McGough) */
/* ============================================================================
 * Enqpy(tm)-HKU -- Deployable Single-Window-Per-Credential API  Rev 4.0
 *
 * Implements enqpy_hku.h.  Links against the Enqpy core (enqpy_reference.c),
 * which is left BYTE-IDENTICAL: compile the core WITHOUT ENQPY_SELFTEST /
 * ENQPY_BENCHMARK so it exposes no main(), then link this translation unit.
 *
 *   cc -c enqpy_reference.c -o enqpy_reference.o
 *   cc -DENQPY_HKU_DEMO enqpy_hku.c enqpy_reference.o -o enqpy_hku_demo
 *
 * The MAC realises the FCD 8.6 encrypt-then-MAC construction:
 *   IK  = HKDF-SHA-256(IKM = EK_bytes, salt = nonce_bytes, info "EnqpyINTEGRITY")
 *   Tag = HMAC-SHA-256(IK, nonce_bytes || ciphertext || metadata)
 * For nonce_bytes this reference uses the transmitted public OR field
 * (or_nibs || or_ctr) rather than the cipher-internal eff_or, so the core stays
 * untouched; both bind the per-message nonce.  IK is a per-segment secret and
 * is zeroized after the tag is computed.
 * ============================================================================ */
#include "enqpy_hku.h"
#include <string.h>

/* ---- Enqpy core (enqpy_reference.c), declared extern; no core edits. ------ */
extern void enqpy_init(uint64_t persisted_or_ctr);
extern int  PDAF_SEC(const uint8_t *ek, const uint8_t *qk,
                     const uint8_t *or_nibs, uint64_t or_ctr,
                     int n, const uint8_t *target,
                     int nTextLen, uint8_t *out);

/* ============================================================================
 * SHA-256 (FIPS 180-4), HMAC-SHA-256 (RFC 2104), HKDF-SHA-256 (RFC 5869).
 * Self-contained so the wrapper's encrypt-then-MAC needs no external library.
 * ============================================================================ */
typedef struct { uint32_t s[8]; uint64_t len; uint8_t buf[64]; size_t n; } sha256_ctx;

static uint32_t ror32(uint32_t x, int r){ return (x >> r) | (x << (32 - r)); }

static const uint32_t SHA_K[64] = {
 0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
 0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
 0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
 0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
 0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
 0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
 0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
 0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2 };

static void sha256_init(sha256_ctx *c){
    static const uint32_t iv[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
                                 0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    memcpy(c->s, iv, sizeof iv); c->len=0; c->n=0;
}
static void sha256_block(sha256_ctx *c, const uint8_t *p){
    uint32_t w[64];
    for (int i=0;i<16;i++)
        w[i]=((uint32_t)p[i*4]<<24)|((uint32_t)p[i*4+1]<<16)|((uint32_t)p[i*4+2]<<8)|p[i*4+3];
    for (int i=16;i<64;i++){
        uint32_t s0=ror32(w[i-15],7)^ror32(w[i-15],18)^(w[i-15]>>3);
        uint32_t s1=ror32(w[i-2],17)^ror32(w[i-2],19)^(w[i-2]>>10);
        w[i]=w[i-16]+s0+w[i-7]+s1;
    }
    uint32_t a=c->s[0],b=c->s[1],d=c->s[2],e=c->s[3],f=c->s[4],g=c->s[5],h=c->s[6],k=c->s[7];
    for (int i=0;i<64;i++){
        uint32_t S1=ror32(f,6)^ror32(f,11)^ror32(f,25);
        uint32_t ch=(f&g)^((~f)&h);
        uint32_t t1=k+S1+ch+SHA_K[i]+w[i];
        uint32_t S0=ror32(a,2)^ror32(a,13)^ror32(a,22);
        uint32_t maj=(a&b)^(a&d)^(b&d);
        uint32_t t2=S0+maj;
        k=h; h=g; g=f; f=e+t1; e=d; d=b; b=a; a=t1+t2;
    }
    c->s[0]+=a; c->s[1]+=b; c->s[2]+=d; c->s[3]+=e;
    c->s[4]+=f; c->s[5]+=g; c->s[6]+=h; c->s[7]+=k;
}
static void sha256_update(sha256_ctx *c, const uint8_t *p, size_t n){
    c->len += (uint64_t)n*8;
    while (n){
        size_t take = 64 - c->n; if (take>n) take=n;
        memcpy(c->buf + c->n, p, take); c->n += take; p += take; n -= take;
        if (c->n==64){ sha256_block(c, c->buf); c->n=0; }
    }
}
static void sha256_final(sha256_ctx *c, uint8_t out[32]){
    uint64_t bits=c->len; uint8_t pad=0x80;
    sha256_update(c,&pad,1);
    uint8_t z=0; while (c->n!=56) sha256_update(c,&z,1);
    uint8_t lb[8]; for(int i=0;i<8;i++) lb[i]=(uint8_t)(bits>>(56-i*8));
    sha256_update(c,lb,8);
    for (int i=0;i<8;i++){ out[i*4]=(uint8_t)(c->s[i]>>24); out[i*4+1]=(uint8_t)(c->s[i]>>16);
                           out[i*4+2]=(uint8_t)(c->s[i]>>8); out[i*4+3]=(uint8_t)c->s[i]; }
}

/* HMAC-SHA-256 over up to two input chunks (msg = a||b). */
static void hmac_sha256_2(const uint8_t *key, size_t klen,
                          const uint8_t *a, size_t alen,
                          const uint8_t *b, size_t blen, uint8_t out[32]){
    uint8_t k0[64]; memset(k0,0,64);
    if (klen>64){ sha256_ctx t; sha256_init(&t); sha256_update(&t,key,klen); sha256_final(&t,k0); }
    else memcpy(k0,key,klen);
    uint8_t ip[64],op[64];
    for (int i=0;i<64;i++){ ip[i]=k0[i]^0x36; op[i]=k0[i]^0x5c; }
    uint8_t inner[32]; sha256_ctx c;
    sha256_init(&c); sha256_update(&c,ip,64);
    if (a&&alen) sha256_update(&c,a,alen);
    if (b&&blen) sha256_update(&c,b,blen);
    sha256_final(&c,inner);
    sha256_init(&c); sha256_update(&c,op,64); sha256_update(&c,inner,32); sha256_final(&c,out);
    memset(k0,0,64); memset(ip,0,64); memset(op,0,64); memset(inner,0,32);
}
static void hmac_sha256(const uint8_t *key,size_t klen,const uint8_t *m,size_t mlen,uint8_t out[32]){
    hmac_sha256_2(key,klen,m,mlen,NULL,0,out);
}

/* HKDF-SHA-256, L = 32 (one expand block). */
static void hkdf_sha256_32(const uint8_t *salt,size_t slen,
                           const uint8_t *ikm,size_t ilen,
                           const uint8_t *info,size_t inlen, uint8_t out[32]){
    uint8_t prk[32];
    hmac_sha256(salt,slen,ikm,ilen,prk);                 /* extract */
    uint8_t one=0x01;
    hmac_sha256_2(prk,32,info,inlen,&one,1,out);          /* expand T(1) */
    memset(prk,0,32);
}

/* ============================================================================
 * Helpers: nibble<->byte packing, big-endian writers, constant-time compare.
 * ============================================================================ */
static void nibs_to_bytes(const uint8_t *nibs, int n, uint8_t *out){ /* n even */
    for (int i=0;i<n/2;i++) out[i]=(uint8_t)(((nibs[2*i]&0xF)<<4)|(nibs[2*i+1]&0xF));
}
static void bytes_to_nibs(const uint8_t *b, int n, uint8_t *out){    /* n even */
    for (int i=0;i<n/2;i++){ out[2*i]=(uint8_t)(b[i]>>4); out[2*i+1]=(uint8_t)(b[i]&0xF); }
}
static void wr_be32(uint8_t *p, uint32_t v){ p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v; }
static void wr_be64(uint8_t *p, uint64_t v){ for(int i=0;i<8;i++) p[i]=(uint8_t)(v>>(56-i*8)); }
static uint32_t rd_be32(const uint8_t *p){ return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3]; }
static uint64_t rd_be64(const uint8_t *p){ uint64_t v=0; for(int i=0;i<8;i++) v=(v<<8)|p[i]; return v; }
static int ct_equal(const uint8_t *a, const uint8_t *b, size_t n){
    uint8_t d=0; for(size_t i=0;i<n;i++) d|=(uint8_t)(a[i]^b[i]); return d==0;
}

/* Build the per-segment authenticated metadata: seg_index||total_segs||seg_len. */
#define ENQPY_HKU_META_LEN 12
static void build_seg_meta(uint8_t meta[ENQPY_HKU_META_LEN],
                           uint32_t seg_index, uint32_t total_segs, uint32_t seg_len){
    wr_be32(meta+0, seg_index); wr_be32(meta+4, total_segs); wr_be32(meta+8, seg_len);
}

/* Compute the per-segment tag over the nonce, ciphertext, and metadata.
 * MAC input = nonce_bytes(32) || or_ctr(8) || ct || meta.  IK is derived and
 * zeroized here so it never lives beyond one tag. */
static void seg_tag(const enqpy_hku_cred *cred,
                    const uint8_t *ct, size_t ct_len,
                    const uint8_t *meta, size_t meta_len, uint8_t tag[32]){
    uint8_t nonce_b[ENQPY_HKU_N/2], ek_b[ENQPY_HKU_N/2];
    nibs_to_bytes(cred->or_nibs, ENQPY_HKU_N, nonce_b);
    nibs_to_bytes(cred->ek,      ENQPY_HKU_N, ek_b);
    static const uint8_t INFO[] = "EnqpyINTEGRITY";
    uint8_t ik[32];
    hkdf_sha256_32(nonce_b, sizeof nonce_b, ek_b, sizeof ek_b, INFO, sizeof INFO - 1, ik);

    /* MAC over: nonce_bytes || or_ctr || ct || meta.  Assemble a small prefix,
     * then stream ct and meta. */
    uint8_t pre[ENQPY_HKU_N/2 + 8];
    memcpy(pre, nonce_b, sizeof nonce_b);
    wr_be64(pre + sizeof nonce_b, cred->or_ctr);

    /* HMAC streaming: inner = H(ipad || pre || ct || meta). Use the 2-chunk
     * helper twice is awkward; do a direct streaming HMAC here. */
    uint8_t k0[64]; memset(k0,0,64);
    if (32>64) {} memcpy(k0, ik, 32);
    uint8_t ip[64], op[64];
    for (int i=0;i<64;i++){ ip[i]=k0[i]^0x36; op[i]=k0[i]^0x5c; }
    sha256_ctx c; uint8_t inner[32];
    sha256_init(&c); sha256_update(&c,ip,64);
    sha256_update(&c,pre,sizeof pre);
    if (ct&&ct_len)     sha256_update(&c,ct,ct_len);
    if (meta&&meta_len) sha256_update(&c,meta,meta_len);
    sha256_final(&c,inner);
    sha256_init(&c); sha256_update(&c,op,64); sha256_update(&c,inner,32); sha256_final(&c,tag);

    memset(ik,0,32); memset(k0,0,64); memset(ip,0,64); memset(op,0,64);
    memset(inner,0,32); memset(ek_b,0,sizeof ek_b);
}

/* ============================================================================
 * GUARDED SINGLE-SEGMENT PRIMITIVE
 * ============================================================================ */
int enqpy_hku_seal_segment(const enqpy_hku_cred *cred,
                           const uint8_t *pt, size_t pt_len,
                           const uint8_t *meta, size_t meta_len,
                           uint8_t *ct, uint8_t *tag){
    if (!cred || (!pt && pt_len) || !ct || !tag) return ENQPY_HKU_ERR_NULL;
    if (pt_len > ENQPY_HKU_WINDOW_BYTES) return ENQPY_HKU_ERR_TOO_LONG;   /* one window only */
    /* enforce EK != QK (FCD 7.1): identical master keys collapse key separation */
    if (ct_equal(cred->ek, cred->qk, ENQPY_HKU_N)) return ENQPY_HKU_ERR_CRED;

    /* Core encrypt -- pt_len <= one window, so PDAF_SEC's loop runs once and
     * Phase 5 is never reached. */
    int r = PDAF_SEC(cred->ek, cred->qk, cred->or_nibs, cred->or_ctr,
                     ENQPY_HKU_N, pt, (int)pt_len, ct);
    if (r != (int)pt_len) return ENQPY_HKU_ERR_CORE;

    seg_tag(cred, ct, pt_len, meta, meta_len, tag);       /* encrypt-then-MAC */
    return ENQPY_HKU_OK;
}

int enqpy_hku_open_segment(const enqpy_hku_cred *cred,
                           const uint8_t *ct, size_t ct_len,
                           const uint8_t *meta, size_t meta_len,
                           const uint8_t *tag,
                           uint8_t *pt){
    if (!cred || (!ct && ct_len) || !tag || (!pt && ct_len)) return ENQPY_HKU_ERR_NULL;
    if (ct_len > ENQPY_HKU_WINDOW_BYTES) return ENQPY_HKU_ERR_TOO_LONG;
    if (ct_equal(cred->ek, cred->qk, ENQPY_HKU_N)) return ENQPY_HKU_ERR_CRED;

    uint8_t want[ENQPY_HKU_TAG_BYTES];
    seg_tag(cred, ct, ct_len, meta, meta_len, want);
    if (!ct_equal(want, tag, ENQPY_HKU_TAG_BYTES)) return ENQPY_HKU_ERR_MAC; /* verify BEFORE decrypt */

    int r = PDAF_SEC(cred->ek, cred->qk, cred->or_nibs, cred->or_ctr,
                     ENQPY_HKU_N, ct, (int)ct_len, pt);
    if (r != (int)ct_len) return ENQPY_HKU_ERR_CORE;
    return ENQPY_HKU_OK;
}

/* ============================================================================
 * LONG-MESSAGE SEGMENTATION
 *
 * Frame layout (big-endian integers):
 *   "EHK1"        4   magic
 *   n             1   profile nibble count (64 at HIGH)
 *   total_segs    4
 *   total_pt_len  8
 *   per segment:
 *     or_ctr      8
 *     or_nibs     n/2 (32) packed nonce
 *     seg_len     4
 *     ct          seg_len
 *     tag         32
 * ============================================================================ */
#define HDR_LEN   (4 + 1 + 4 + 8)
#define SEG_HDR   (8 + (ENQPY_HKU_N/2) + 4)

static size_t frame_size(size_t pt_len, uint32_t *out_segs){
    uint32_t segs = (uint32_t)((pt_len + ENQPY_HKU_WINDOW_BYTES - 1) / ENQPY_HKU_WINDOW_BYTES);
    if (pt_len == 0) segs = 0;
    if (out_segs) *out_segs = segs;
    return (size_t)HDR_LEN + (size_t)segs * (SEG_HDR + ENQPY_HKU_TAG_BYTES) + pt_len;
}

int enqpy_hku_seal_message(enqpy_hku_cred_fn cred_fn, void *ctx,
                           const uint8_t *pt, size_t pt_len,
                           uint8_t *out, size_t out_cap, size_t *out_len){
    if (!cred_fn || (!pt && pt_len) || !out_len) return ENQPY_HKU_ERR_NULL;
    uint32_t total_segs;
    size_t need = frame_size(pt_len, &total_segs);
    *out_len = need;
    if (!out) return ENQPY_HKU_OK;                 /* sizing pass */
    if (out_cap < need) return ENQPY_HKU_ERR_BUFFER;

    uint8_t *p = out;
    memcpy(p, "EHK1", 4); p += 4;
    *p++ = (uint8_t)ENQPY_HKU_N;
    wr_be32(p, total_segs); p += 4;
    wr_be64(p, (uint64_t)pt_len); p += 8;

    size_t off = 0;
    for (uint32_t s = 0; s < total_segs; s++){
        uint32_t seg_len = (uint32_t)(pt_len - off);
        if (seg_len > ENQPY_HKU_WINDOW_BYTES) seg_len = ENQPY_HKU_WINDOW_BYTES;

        enqpy_hku_cred cred;
        if (cred_fn(s, &cred, ctx) != 0) return ENQPY_HKU_ERR_CRED;

        /* write per-segment public nonce */
        wr_be64(p, cred.or_ctr); p += 8;
        nibs_to_bytes(cred.or_nibs, ENQPY_HKU_N, p); p += ENQPY_HKU_N/2;
        wr_be32(p, seg_len); p += 4;

        uint8_t meta[ENQPY_HKU_META_LEN];
        build_seg_meta(meta, s, total_segs, seg_len);

        uint8_t *ct_dst  = p;            p += seg_len;
        uint8_t *tag_dst = p;            p += ENQPY_HKU_TAG_BYTES;

        int rc = enqpy_hku_seal_segment(&cred, pt + off, seg_len,
                                        meta, sizeof meta, ct_dst, tag_dst);
        memset(&cred, 0, sizeof cred);
        if (rc != ENQPY_HKU_OK) return rc;
        off += seg_len;
    }
    return ENQPY_HKU_OK;
}

int enqpy_hku_open_message(enqpy_hku_cred_fn cred_fn, void *ctx,
                           const uint8_t *frame, size_t frame_len,
                           uint8_t *pt, size_t pt_cap, size_t *pt_len){
    if (!cred_fn || !frame || !pt_len) return ENQPY_HKU_ERR_NULL;
    if (frame_len < HDR_LEN) return ENQPY_HKU_ERR_FORMAT;
    const uint8_t *p = frame, *end = frame + frame_len;
    if (memcmp(p, "EHK1", 4) != 0) return ENQPY_HKU_ERR_FORMAT;
    p += 4;
    if (*p++ != (uint8_t)ENQPY_HKU_N) return ENQPY_HKU_ERR_FORMAT;
    uint32_t total_segs = rd_be32(p); p += 4;
    uint64_t total_pt   = rd_be64(p); p += 8;
    *pt_len = (size_t)total_pt;
    if (pt) { if (pt_cap < total_pt) return ENQPY_HKU_ERR_BUFFER; }
    else    { return ENQPY_HKU_OK; }               /* sizing pass */

    size_t off = 0;
    for (uint32_t s = 0; s < total_segs; s++){
        if ((size_t)(end - p) < SEG_HDR) return ENQPY_HKU_ERR_FORMAT;
        uint64_t or_ctr = rd_be64(p); p += 8;
        uint8_t  or_b[ENQPY_HKU_N/2]; memcpy(or_b, p, ENQPY_HKU_N/2); p += ENQPY_HKU_N/2;
        uint32_t seg_len = rd_be32(p); p += 4;
        if (seg_len > ENQPY_HKU_WINDOW_BYTES) return ENQPY_HKU_ERR_FORMAT;
        if ((size_t)(end - p) < (size_t)seg_len + ENQPY_HKU_TAG_BYTES) return ENQPY_HKU_ERR_FORMAT;
        const uint8_t *ct  = p;  p += seg_len;
        const uint8_t *tag = p;  p += ENQPY_HKU_TAG_BYTES;
        if (off + seg_len > total_pt) return ENQPY_HKU_ERR_FORMAT;

        enqpy_hku_cred cred;
        if (cred_fn(s, &cred, ctx) != 0) return ENQPY_HKU_ERR_CRED;
        /* public nonce comes from the frame, not the credential source */
        cred.or_ctr = or_ctr;
        bytes_to_nibs(or_b, ENQPY_HKU_N, cred.or_nibs);

        uint8_t meta[ENQPY_HKU_META_LEN];
        build_seg_meta(meta, s, total_segs, seg_len);

        int rc = enqpy_hku_open_segment(&cred, ct, seg_len, meta, sizeof meta, tag, pt + off);
        memset(&cred, 0, sizeof cred);
        if (rc != ENQPY_HKU_OK) return rc;          /* ERR_MAC on any tamper */
        off += seg_len;
    }
    if (off != total_pt) return ENQPY_HKU_ERR_FORMAT;
    return ENQPY_HKU_OK;
}

/* ============================================================================
 * SELF-TEST / DEMO  (compile with -DENQPY_HKU_DEMO)
 * ============================================================================ */
#ifdef ENQPY_HKU_DEMO
#include <stdio.h>

/* SplitMix64 -- TEST-ONLY deterministic stream so the KAT is reproducible.
 * PRODUCTION MUST draw each segment credential from an independent CSPRNG;
 * a deterministic generator like this MUST NOT be used to key real traffic. */
static uint64_t sm_state;
static uint64_t sm_next(void){
    uint64_t z = (sm_state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}
static void sm_fill_nibs(uint8_t *nibs, int n){
    for (int i=0;i<n;i++) nibs[i] = (uint8_t)(sm_next() & 0xF);
}

/* Pre-generated array of FRESH INDEPENDENT credentials (one per segment),
 * representing out-of-band shared per-segment material.  The cred_fn returns
 * the stored credential; it is NOT a function of seg_index through the cipher. */
#define MAX_SEGS 8
static enqpy_hku_cred g_creds[MAX_SEGS];
static int            g_ncreds = 0;
static int            g_cred_calls = 0;

static void make_independent_creds(int nsegs){
    g_ncreds = nsegs;
    for (int s=0; s<nsegs; s++){
        sm_fill_nibs(g_creds[s].ek, ENQPY_HKU_N);
        sm_fill_nibs(g_creds[s].qk, ENQPY_HKU_N);
        if (ct_equal(g_creds[s].ek, g_creds[s].qk, ENQPY_HKU_N)) g_creds[s].qk[0] ^= 0x1; /* EK!=QK */
        sm_fill_nibs(g_creds[s].or_nibs, ENQPY_HKU_N);
        g_creds[s].or_ctr = sm_next();
    }
}
static int test_cred_fn(uint32_t seg_index, enqpy_hku_cred *out, void *ctx){
    (void)ctx; g_cred_calls++;
    if (seg_index >= (uint32_t)g_ncreds) return -1;
    *out = g_creds[seg_index];
    return 0;
}

int main(void){
    enqpy_init(0);
    int pass=0, fail=0;
    #define CHECK(cond, name) do{ if(cond){pass++; printf("  PASS  %s\n",name);} \
                                  else {fail++; printf("  FAIL  %s\n",name);} }while(0)

    printf("Enqpy(tm)-HKU deployable wrapper -- self-test\n");
    printf("window=%d bytes, tag=%d bytes, n=%d\n\n",
           ENQPY_HKU_WINDOW_BYTES, ENQPY_HKU_TAG_BYTES, ENQPY_HKU_N);

    /* 0. SHA-256("abc") known-answer (hash sanity) */
    {
        uint8_t h[32]; sha256_ctx c; sha256_init(&c);
        sha256_update(&c,(const uint8_t*)"abc",3); sha256_final(&c,h);
        static const uint8_t exp[32]={
            0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
            0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad};
        CHECK(ct_equal(h,exp,32), "SHA-256(\"abc\") KAT");
    }

    /* Deterministic test stream (fixed seed for reproducible KAT) */
    sm_state = 0xC0FFEE1234567890ULL;

    /* 1. GUARD: single segment > one window must be REFUSED */
    {
        make_independent_creds(1);
        static uint8_t big[ENQPY_HKU_WINDOW_BYTES+1];
        static uint8_t ct[ENQPY_HKU_WINDOW_BYTES+1]; uint8_t tag[32];
        int rc = enqpy_hku_seal_segment(&g_creds[0], big, sizeof big, NULL,0, ct, tag);
        CHECK(rc==ENQPY_HKU_ERR_TOO_LONG, "guard refuses >1 window (single segment)");
    }

    /* 2. Single-segment round trip at exactly one window */
    {
        make_independent_creds(1);
        static uint8_t msg[ENQPY_HKU_WINDOW_BYTES], ct[ENQPY_HKU_WINDOW_BYTES];
        static uint8_t rt[ENQPY_HKU_WINDOW_BYTES]; uint8_t tag[32];
        for (size_t i=0;i<sizeof msg;i++) msg[i]=(uint8_t)(sm_next());
        uint8_t meta[ENQPY_HKU_META_LEN]; build_seg_meta(meta,0,1,(uint32_t)sizeof msg);
        int r1=enqpy_hku_seal_segment(&g_creds[0],msg,sizeof msg,meta,sizeof meta,ct,tag);
        int r2=enqpy_hku_open_segment (&g_creds[0],ct,sizeof ct,meta,sizeof meta,tag,rt);
        CHECK(r1==ENQPY_HKU_OK && r2==ENQPY_HKU_OK && memcmp(msg,rt,sizeof msg)==0,
              "single-window seal/open round trip");
    }

    /* 3. Long message -> segmentation round trip (5,000 bytes -> 3 segments) */
    {
        const size_t L = 5000;
        make_independent_creds(3);
        static uint8_t msg[5000], frame[8192], rt[5000];
        for (size_t i=0;i<L;i++) msg[i]=(uint8_t)(i*131 + 7);
        size_t flen=0, plen=0;
        g_cred_calls=0;
        int rs=enqpy_hku_seal_message(test_cred_fn,NULL,msg,L,frame,sizeof frame,&flen);
        int seal_calls=g_cred_calls;
        g_cred_calls=0;
        int ro=enqpy_hku_open_message(test_cred_fn,NULL,frame,flen,rt,sizeof rt,&plen);
        CHECK(rs==ENQPY_HKU_OK && ro==ENQPY_HKU_OK && plen==L && memcmp(msg,rt,L)==0,
              "5,000-byte message segmented + reassembled");
        CHECK(seal_calls==3 && g_cred_calls==3,
              "exactly 3 independent credentials drawn (one per segment)");
        /* verify total_segs field == 3 */
        CHECK(rd_be32(frame+5)==3, "frame declares 3 segments");
    }

    /* 4. MAC tamper: flip one ciphertext byte in the frame -> open must fail */
    {
        const size_t L = 3000;
        make_independent_creds(2);
        static uint8_t msg[3000], frame[6144], rt[3000];
        for (size_t i=0;i<L;i++) msg[i]=(uint8_t)(i ^ 0x5A);
        size_t flen=0, plen=0;
        enqpy_hku_seal_message(test_cred_fn,NULL,msg,L,frame,sizeof frame,&flen);
        /* flip a byte inside the first segment's ciphertext region */
        frame[HDR_LEN + SEG_HDR + 10] ^= 0x80;
        int ro=enqpy_hku_open_message(test_cred_fn,NULL,frame,flen,rt,sizeof rt,&plen);
        CHECK(ro==ENQPY_HKU_ERR_MAC, "ciphertext tamper detected (ERR_MAC)");
    }

    /* 5. Reorder tamper: swap two segments' nonces -> metadata binding fails */
    {
        const size_t L = 4096;                 /* exactly 2 full windows -> 2 segs */
        make_independent_creds(2);
        static uint8_t msg[4096], frame[6144], rt[4096];
        for (size_t i=0;i<L;i++) msg[i]=(uint8_t)(i*7);
        size_t flen=0, plen=0;
        enqpy_hku_seal_message(test_cred_fn,NULL,msg,L,frame,sizeof frame,&flen);
        /* swap the two segment blocks entirely (nonce+len+ct+tag) */
        size_t blk = SEG_HDR + ENQPY_HKU_WINDOW_BYTES + ENQPY_HKU_TAG_BYTES;
        static uint8_t tmp[3000];
        memcpy(tmp,            frame+HDR_LEN,        blk);
        memmove(frame+HDR_LEN, frame+HDR_LEN+blk,    blk);
        memcpy(frame+HDR_LEN+blk, tmp,               blk);
        int ro=enqpy_hku_open_message(test_cred_fn,NULL,frame,flen,rt,sizeof rt,&plen);
        CHECK(ro==ENQPY_HKU_ERR_MAC, "segment reorder detected (metadata-bound MAC)");
    }

    printf("\n%d passed, %d failed\n", pass, fail);
    return fail ? 1 : 0;
}
#endif /* ENQPY_HKU_DEMO */
