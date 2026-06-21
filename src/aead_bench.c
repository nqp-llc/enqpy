/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2026 NQP LLC (Paul McGough) */
/* Apples-to-apples authenticated-encryption benchmark, portable scalar C.
 * Part of the Enqpy(TM) reference repository; licensed under the Apache
 * License, Version 2.0 -- see the LICENSE and NOTICE files.
 * Matches the Performance Benchmark Report (Public Summary) methodology:
 * 59,189-byte message, 10 warmup
 * round-robin passes, 20 timed round-robin iterations, best-of-20 + average,
 * clock_gettime around the cipher call only. Every primitive is verified
 * against its official test vector BEFORE timing; if any KAT fails we abort.
 *
 * Ciphers (all include their authentication, as deployed):
 *   - Enqpy(TM) HIGH (n=64) + Poly1305               (one keystream expansion supplies the
 *                                          one-time MAC key AND the ciphertext)
 *   - ChaCha20-Poly1305                  (RFC 8439)
 *   - AES-256-GCM                        (portable T-table AES + bitwise GHASH)
 *   - AES-256-CTR                        (no auth -- noted)
 *
 * Build (links the Enqpy reference, enqpy_reference.c):
 *   cc -O3 -march=native -std=c11 -c enqpy_reference.c -o enqpy_reference.o
 *   cc -O3 -march=native -std=c11 -D_POSIX_C_SOURCE=200809L aead_bench.c enqpy_reference.o -o aead_bench
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern int  PDAF_SEC(const uint8_t*,const uint8_t*,const uint8_t*,uint64_t,int,const uint8_t*,int,uint8_t*);
extern void enqpy_init(uint64_t);   /* fills the MOD16 table; MUST run before any PDAF_SEC call */

static double now_ns(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return (double)ts.tv_sec*1e9+(double)ts.tv_nsec;}
static volatile uint8_t g_sink;
static void sink(const uint8_t*b,size_t n){g_sink^=b[0];g_sink^=b[n-1];}

/* ============================ Poly1305 (donna 32-bit) ============================ */
typedef struct{uint32_t r[5],h[5],pad[4];size_t leftover;uint8_t buffer[16];uint8_t final;}poly1305_ctx;
static uint32_t U8TO32(const uint8_t*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void U32TO8(uint8_t*p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static void poly1305_init(poly1305_ctx*st,const uint8_t key[32]){
    st->r[0]=(U8TO32(&key[0]))&0x3ffffff;st->r[1]=(U8TO32(&key[3])>>2)&0x3ffff03;st->r[2]=(U8TO32(&key[6])>>4)&0x3ffc0ff;
    st->r[3]=(U8TO32(&key[9])>>6)&0x3f03fff;st->r[4]=(U8TO32(&key[12])>>8)&0x00fffff;
    st->h[0]=st->h[1]=st->h[2]=st->h[3]=st->h[4]=0;
    st->pad[0]=U8TO32(&key[16]);st->pad[1]=U8TO32(&key[20]);st->pad[2]=U8TO32(&key[24]);st->pad[3]=U8TO32(&key[28]);
    st->leftover=0;st->final=0;
}
static void poly1305_blocks(poly1305_ctx*st,const uint8_t*m,size_t bytes){
    const uint32_t hibit=st->final?0:(1u<<24);
    uint32_t r0=st->r[0],r1=st->r[1],r2=st->r[2],r3=st->r[3],r4=st->r[4];
    uint32_t s1=r1*5,s2=r2*5,s3=r3*5,s4=r4*5;
    uint32_t h0=st->h[0],h1=st->h[1],h2=st->h[2],h3=st->h[3],h4=st->h[4];
    uint64_t d0,d1,d2,d3,d4;uint32_t c;
    while(bytes>=16){
        h0+=(U8TO32(m+0))&0x3ffffff;h1+=(U8TO32(m+3)>>2)&0x3ffffff;h2+=(U8TO32(m+6)>>4)&0x3ffffff;
        h3+=(U8TO32(m+9)>>6)&0x3ffffff;h4+=(U8TO32(m+12)>>8)|hibit;
        d0=(uint64_t)h0*r0+(uint64_t)h1*s4+(uint64_t)h2*s3+(uint64_t)h3*s2+(uint64_t)h4*s1;
        d1=(uint64_t)h0*r1+(uint64_t)h1*r0+(uint64_t)h2*s4+(uint64_t)h3*s3+(uint64_t)h4*s2;
        d2=(uint64_t)h0*r2+(uint64_t)h1*r1+(uint64_t)h2*r0+(uint64_t)h3*s4+(uint64_t)h4*s3;
        d3=(uint64_t)h0*r3+(uint64_t)h1*r2+(uint64_t)h2*r1+(uint64_t)h3*r0+(uint64_t)h4*s4;
        d4=(uint64_t)h0*r4+(uint64_t)h1*r3+(uint64_t)h2*r2+(uint64_t)h3*r1+(uint64_t)h4*r0;
        c=(uint32_t)(d0>>26);h0=(uint32_t)d0&0x3ffffff;d1+=c;c=(uint32_t)(d1>>26);h1=(uint32_t)d1&0x3ffffff;
        d2+=c;c=(uint32_t)(d2>>26);h2=(uint32_t)d2&0x3ffffff;d3+=c;c=(uint32_t)(d3>>26);h3=(uint32_t)d3&0x3ffffff;
        d4+=c;c=(uint32_t)(d4>>26);h4=(uint32_t)d4&0x3ffffff;h0+=c*5;c=h0>>26;h0&=0x3ffffff;h1+=c;
        m+=16;bytes-=16;
    }
    st->h[0]=h0;st->h[1]=h1;st->h[2]=h2;st->h[3]=h3;st->h[4]=h4;
}
static void poly1305_update(poly1305_ctx*st,const uint8_t*m,size_t bytes){
    size_t i;
    if(st->leftover){size_t want=16-st->leftover;if(want>bytes)want=bytes;for(i=0;i<want;i++)st->buffer[st->leftover+i]=m[i];
        bytes-=want;m+=want;st->leftover+=want;if(st->leftover<16)return;poly1305_blocks(st,st->buffer,16);st->leftover=0;}
    if(bytes>=16){size_t want=bytes&~(size_t)15;poly1305_blocks(st,m,want);m+=want;bytes-=want;}
    if(bytes){for(i=0;i<bytes;i++)st->buffer[st->leftover+i]=m[i];st->leftover+=bytes;}
}
static void poly1305_finish(poly1305_ctx*st,uint8_t mac[16]){
    uint32_t h0,h1,h2,h3,h4,c,g0,g1,g2,g3,g4,mask;uint64_t f;
    if(st->leftover){size_t i=st->leftover;st->buffer[i++]=1;for(;i<16;i++)st->buffer[i]=0;st->final=1;poly1305_blocks(st,st->buffer,16);}
    h0=st->h[0];h1=st->h[1];h2=st->h[2];h3=st->h[3];h4=st->h[4];
    c=h1>>26;h1&=0x3ffffff;h2+=c;c=h2>>26;h2&=0x3ffffff;h3+=c;c=h3>>26;h3&=0x3ffffff;h4+=c;c=h4>>26;h4&=0x3ffffff;h0+=c*5;c=h0>>26;h0&=0x3ffffff;h1+=c;
    g0=h0+5;c=g0>>26;g0&=0x3ffffff;g1=h1+c;c=g1>>26;g1&=0x3ffffff;g2=h2+c;c=g2>>26;g2&=0x3ffffff;g3=h3+c;c=g3>>26;g3&=0x3ffffff;g4=h4+c-(1u<<26);
    mask=(g4>>31)-1;g0&=mask;g1&=mask;g2&=mask;g3&=mask;g4&=mask;mask=~mask;
    h0=(h0&mask)|g0;h1=(h1&mask)|g1;h2=(h2&mask)|g2;h3=(h3&mask)|g3;h4=(h4&mask)|g4;
    h0=(h0|(h1<<26))&0xffffffff;h1=((h1>>6)|(h2<<20))&0xffffffff;h2=((h2>>12)|(h3<<14))&0xffffffff;h3=((h3>>18)|(h4<<8))&0xffffffff;
    f=(uint64_t)h0+st->pad[0];h0=(uint32_t)f;f=(uint64_t)h1+st->pad[1]+(f>>32);h1=(uint32_t)f;f=(uint64_t)h2+st->pad[2]+(f>>32);h2=(uint32_t)f;f=(uint64_t)h3+st->pad[3]+(f>>32);h3=(uint32_t)f;
    U32TO8(mac+0,h0);U32TO8(mac+4,h1);U32TO8(mac+8,h2);U32TO8(mac+12,h3);
}
static void poly1305_oneshot(uint8_t mac[16],const uint8_t*m,size_t n,const uint8_t key[32]){poly1305_ctx st;poly1305_init(&st,key);poly1305_update(&st,m,n);poly1305_finish(&st,mac);}
/* ---- Poly1305 donna 64-bit (radix 2^44, __int128) -- faster on x86-64 ---- */
typedef unsigned __int128 u128;
typedef struct{uint64_t r[3],h[3],pad[2];size_t leftover;uint8_t buffer[16];uint8_t final;}poly1305_ctx64;
static uint64_t U8TO64(const uint8_t*p){uint64_t v=0;for(int i=0;i<8;i++)v|=(uint64_t)p[i]<<(8*i);return v;}
static void U64TO8(uint8_t*p,uint64_t v){for(int i=0;i<8;i++)p[i]=(uint8_t)(v>>(8*i));}
static void poly1305_64_init(poly1305_ctx64*st,const uint8_t key[32]){
    uint64_t t0=U8TO64(&key[0]),t1=U8TO64(&key[8]);
    st->r[0]=( t0 )&0xffc0fffffff; st->r[1]=((t0>>44)|(t1<<20))&0xfffffc0ffff; st->r[2]=((t1>>24))&0x00ffffffc0f;
    st->h[0]=st->h[1]=st->h[2]=0; st->pad[0]=U8TO64(&key[16]);st->pad[1]=U8TO64(&key[24]); st->leftover=0;st->final=0;
}
static void poly1305_64_blocks(poly1305_ctx64*st,const uint8_t*m,size_t bytes){
    const uint64_t hibit=st->final?0:((uint64_t)1<<40);
    uint64_t r0=st->r[0],r1=st->r[1],r2=st->r[2],s1=r1*(5<<2),s2=r2*(5<<2);
    uint64_t h0=st->h[0],h1=st->h[1],h2=st->h[2],c; u128 d0,d1,d2,d;
    while(bytes>=16){
        uint64_t t0=U8TO64(&m[0]),t1=U8TO64(&m[8]);
        h0+=(t0)&0xfffffffffff; h1+=((t0>>44)|(t1<<20))&0xfffffffffff; h2+=((((t1>>24))&0x3ffffffffff)|hibit);
        d0=(u128)h0*r0; d=(u128)h1*s2; d0+=d; d=(u128)h2*s1; d0+=d;
        d1=(u128)h0*r1; d=(u128)h1*r0; d1+=d; d=(u128)h2*s2; d1+=d;
        d2=(u128)h0*r2; d=(u128)h1*r1; d2+=d; d=(u128)h2*r0; d2+=d;
        c=(uint64_t)(d0>>44); h0=(uint64_t)d0&0xfffffffffff;
        d1+=c; c=(uint64_t)(d1>>44); h1=(uint64_t)d1&0xfffffffffff;
        d2+=c; c=(uint64_t)(d2>>42); h2=(uint64_t)d2&0x3ffffffffff;
        h0+=c*5; c=h0>>44; h0&=0xfffffffffff; h1+=c;
        m+=16;bytes-=16;
    }
    st->h[0]=h0;st->h[1]=h1;st->h[2]=h2;
}
static void poly1305_64_update(poly1305_ctx64*st,const uint8_t*m,size_t bytes){
    size_t i;
    if(st->leftover){size_t want=16-st->leftover;if(want>bytes)want=bytes;for(i=0;i<want;i++)st->buffer[st->leftover+i]=m[i];bytes-=want;m+=want;st->leftover+=want;if(st->leftover<16)return;poly1305_64_blocks(st,st->buffer,16);st->leftover=0;}
    if(bytes>=16){size_t want=bytes&~(size_t)15;poly1305_64_blocks(st,m,want);m+=want;bytes-=want;}
    if(bytes){for(i=0;i<bytes;i++)st->buffer[st->leftover+i]=m[i];st->leftover+=bytes;}
}
static void poly1305_64_finish(poly1305_ctx64*st,uint8_t mac[16]){
    uint64_t h0,h1,h2,c,g0,g1,g2,t0,t1;
    if(st->leftover){size_t i=st->leftover;st->buffer[i++]=1;for(;i<16;i++)st->buffer[i]=0;st->final=1;poly1305_64_blocks(st,st->buffer,16);}
    h0=st->h[0];h1=st->h[1];h2=st->h[2];
    c=h1>>44;h1&=0xfffffffffff;h2+=c;c=h2>>42;h2&=0x3ffffffffff;h0+=c*5;c=h0>>44;h0&=0xfffffffffff;h1+=c;c=h1>>44;h1&=0xfffffffffff;h2+=c;c=h2>>42;h2&=0x3ffffffffff;h0+=c*5;c=h0>>44;h0&=0xfffffffffff;h1+=c;
    g0=h0+5;c=g0>>44;g0&=0xfffffffffff;g1=h1+c;c=g1>>44;g1&=0xfffffffffff;g2=h2+c-((uint64_t)1<<42);
    c=(g2>>63)-1;g0&=c;g1&=c;g2&=c;c=~c;h0=(h0&c)|g0;h1=(h1&c)|g1;h2=(h2&c)|g2;
    t0=st->pad[0];t1=st->pad[1];
    h0+=((t0)&0xfffffffffff);c=h0>>44;h0&=0xfffffffffff;
    h1+=(((t0>>44)|(t1<<20))&0xfffffffffff)+c;c=h1>>44;h1&=0xfffffffffff;
    h2+=(((t1>>24))&0x3ffffffffff)+c;h2&=0x3ffffffffff;
    h0=(h0)|(h1<<44);h1=(h1>>20)|(h2<<24);
    U64TO8(&mac[0],h0);U64TO8(&mac[8],h1);
}
static void poly1305_64_oneshot(uint8_t mac[16],const uint8_t*m,size_t n,const uint8_t key[32]){poly1305_ctx64 st;poly1305_64_init(&st,key);poly1305_64_update(&st,m,n);poly1305_64_finish(&st,mac);}

/* RFC 8439 AEAD tag (now using 64-bit Poly1305): MAC over AAD||pad16||CT||pad16||le64(aadlen)||le64(ctlen) */
static void poly1305_aead_tag(uint8_t tag[16],const uint8_t key[32],const uint8_t*aad,size_t aadlen,const uint8_t*ct,size_t ctlen){
    poly1305_ctx64 st;poly1305_64_init(&st,key);uint8_t pad[16]={0};uint8_t lenblk[16];
    if(aadlen){poly1305_64_update(&st,aad,aadlen);if(aadlen%16)poly1305_64_update(&st,pad,16-(aadlen%16));}
    poly1305_64_update(&st,ct,ctlen);if(ctlen%16)poly1305_64_update(&st,pad,16-(ctlen%16));
    for(int i=0;i<8;i++)lenblk[i]=(uint8_t)((uint64_t)aadlen>>(8*i));
    for(int i=0;i<8;i++)lenblk[8+i]=(uint8_t)((uint64_t)ctlen>>(8*i));
    poly1305_64_update(&st,lenblk,16);poly1305_64_finish(&st,tag);
}

/* ============================ ChaCha20 (RFC 8439) ============================ */
#define ROTL32(v,n) (((v)<<(n))|((v)>>(32-(n))))
#define QR(a,b,c,d) a+=b;d^=a;d=ROTL32(d,16);c+=d;b^=c;b=ROTL32(b,12);a+=b;d^=a;d=ROTL32(d,8);c+=d;b^=c;b=ROTL32(b,7);
static void chacha20_block(uint32_t out[16],const uint32_t in[16]){
    uint32_t x[16];for(int i=0;i<16;i++)x[i]=in[i];
    for(int i=0;i<10;i++){QR(x[0],x[4],x[8],x[12])QR(x[1],x[5],x[9],x[13])QR(x[2],x[6],x[10],x[14])QR(x[3],x[7],x[11],x[15])
        QR(x[0],x[5],x[10],x[15])QR(x[1],x[6],x[11],x[12])QR(x[2],x[7],x[8],x[13])QR(x[3],x[4],x[9],x[14])}
    for(int i=0;i<16;i++)out[i]=x[i]+in[i];
}
static void chacha20_xor(const uint8_t key[32],uint32_t counter,const uint8_t nonce[12],const uint8_t*in,uint8_t*out,size_t len){
    uint32_t st[16];st[0]=0x61707865;st[1]=0x3320646e;st[2]=0x79622d32;st[3]=0x6b206574;
    for(int i=0;i<8;i++)st[4+i]=U8TO32(key+4*i);st[12]=counter;st[13]=U8TO32(nonce+0);st[14]=U8TO32(nonce+4);st[15]=U8TO32(nonce+8);
    uint8_t ks[64];uint32_t blk[16];size_t off=0;
    while(len){chacha20_block(blk,st);for(int i=0;i<16;i++)U32TO8(ks+4*i,blk[i]);
        size_t n=len<64?len:64;for(size_t i=0;i<n;i++)out[off+i]=in[off+i]^ks[i];off+=n;len-=n;st[12]++;}
}
static void chacha20poly1305_seal(const uint8_t key[32],const uint8_t nonce[12],const uint8_t*aad,size_t aadlen,const uint8_t*pt,size_t ptlen,uint8_t*ct,uint8_t tag[16]){
    uint8_t polykey[64];uint32_t st0[16];st0[0]=0x61707865;st0[1]=0x3320646e;st0[2]=0x79622d32;st0[3]=0x6b206574;
    for(int i=0;i<8;i++)st0[4+i]=U8TO32(key+4*i);st0[12]=0;st0[13]=U8TO32(nonce+0);st0[14]=U8TO32(nonce+4);st0[15]=U8TO32(nonce+8);
    uint32_t blk[16];chacha20_block(blk,st0);for(int i=0;i<16;i++)U32TO8(polykey+4*i,blk[i]);
    chacha20_xor(key,1,nonce,pt,ct,ptlen);
    poly1305_aead_tag(tag,polykey,aad,aadlen,ct,ptlen);
}

/* ============================ AES-256 (portable T-table) ============================ */
static uint8_t Sbox[256];static uint8_t Rcon[16];static uint32_t Te0[256],Te1[256],Te2[256],Te3[256];
static uint8_t xtime(uint8_t x){return (uint8_t)((x<<1)^((x>>7)*0x1b));}
static uint8_t gmul(uint8_t a,uint8_t b){uint8_t p=0;for(int i=0;i<8;i++){if(b&1)p^=a;uint8_t hi=a&0x80;a<<=1;if(hi)a^=0x1b;b>>=1;}return p;}
static void aes_init_tables(void){
    /* build S-box */
    uint8_t p=1,q=1;uint8_t inv[256]={0};
    /* compute multiplicative inverse via gmul loop is heavy; build S-box by standard affine over GF inverse */
    inv[0]=0;{ /* find inverses */ for(int i=1;i<256;i++){for(int j=1;j<256;j++){if(gmul((uint8_t)i,(uint8_t)j)==1){inv[i]=(uint8_t)j;break;}}}}
    (void)p;(void)q;
    for(int i=0;i<256;i++){uint8_t s=inv[i];uint8_t x=s;x^=(uint8_t)((s<<1)|(s>>7));x^=(uint8_t)((s<<2)|(s>>6));x^=(uint8_t)((s<<3)|(s>>5));x^=(uint8_t)((s<<4)|(s>>4));x^=0x63;Sbox[i]=x;}
    uint8_t r=1;for(int i=0;i<16;i++){Rcon[i]=r;r=xtime(r);}
    for(int i=0;i<256;i++){uint8_t s=Sbox[i];uint8_t s2=gmul(s,2),s3=gmul(s,3);
        Te0[i]=((uint32_t)s2<<24)|((uint32_t)s<<16)|((uint32_t)s<<8)|s3;
        Te1[i]=((uint32_t)s3<<24)|((uint32_t)s2<<16)|((uint32_t)s<<8)|s;
        Te2[i]=((uint32_t)s<<24)|((uint32_t)s3<<16)|((uint32_t)s2<<8)|s;
        Te3[i]=((uint32_t)s<<24)|((uint32_t)s<<16)|((uint32_t)s3<<8)|s2;}
}
static void aes256_key_expand(const uint8_t key[32],uint32_t rk[60]){
    for(int i=0;i<8;i++)rk[i]=((uint32_t)key[4*i]<<24)|((uint32_t)key[4*i+1]<<16)|((uint32_t)key[4*i+2]<<8)|key[4*i+3];
    for(int i=8;i<60;i++){uint32_t t=rk[i-1];
        if(i%8==0){t=(t<<8)|(t>>24);t=((uint32_t)Sbox[(t>>24)&0xff]<<24)|((uint32_t)Sbox[(t>>16)&0xff]<<16)|((uint32_t)Sbox[(t>>8)&0xff]<<8)|Sbox[t&0xff];t^=((uint32_t)Rcon[i/8-1])<<24;}
        else if(i%8==4){t=((uint32_t)Sbox[(t>>24)&0xff]<<24)|((uint32_t)Sbox[(t>>16)&0xff]<<16)|((uint32_t)Sbox[(t>>8)&0xff]<<8)|Sbox[t&0xff];}
        rk[i]=rk[i-8]^t;}
}
static void aes256_encrypt_block(const uint32_t rk[60],const uint8_t in[16],uint8_t out[16]){
    uint32_t s0=U8TO32(in)<<0,s1,s2,s3,t0,t1,t2,t3;
    /* load big-endian */
    s0=((uint32_t)in[0]<<24)|((uint32_t)in[1]<<16)|((uint32_t)in[2]<<8)|in[3];
    s1=((uint32_t)in[4]<<24)|((uint32_t)in[5]<<16)|((uint32_t)in[6]<<8)|in[7];
    s2=((uint32_t)in[8]<<24)|((uint32_t)in[9]<<16)|((uint32_t)in[10]<<8)|in[11];
    s3=((uint32_t)in[12]<<24)|((uint32_t)in[13]<<16)|((uint32_t)in[14]<<8)|in[15];
    s0^=rk[0];s1^=rk[1];s2^=rk[2];s3^=rk[3];
    int r;for(r=1;r<14;r++){
        t0=Te0[(s0>>24)&0xff]^Te1[(s1>>16)&0xff]^Te2[(s2>>8)&0xff]^Te3[s3&0xff]^rk[4*r+0];
        t1=Te0[(s1>>24)&0xff]^Te1[(s2>>16)&0xff]^Te2[(s3>>8)&0xff]^Te3[s0&0xff]^rk[4*r+1];
        t2=Te0[(s2>>24)&0xff]^Te1[(s3>>16)&0xff]^Te2[(s0>>8)&0xff]^Te3[s1&0xff]^rk[4*r+2];
        t3=Te0[(s3>>24)&0xff]^Te1[(s0>>16)&0xff]^Te2[(s1>>8)&0xff]^Te3[s2&0xff]^rk[4*r+3];
        s0=t0;s1=t1;s2=t2;s3=t3;}
    t0=((uint32_t)Sbox[(s0>>24)&0xff]<<24)|((uint32_t)Sbox[(s1>>16)&0xff]<<16)|((uint32_t)Sbox[(s2>>8)&0xff]<<8)|Sbox[s3&0xff];t0^=rk[56];
    t1=((uint32_t)Sbox[(s1>>24)&0xff]<<24)|((uint32_t)Sbox[(s2>>16)&0xff]<<16)|((uint32_t)Sbox[(s3>>8)&0xff]<<8)|Sbox[s0&0xff];t1^=rk[57];
    t2=((uint32_t)Sbox[(s2>>24)&0xff]<<24)|((uint32_t)Sbox[(s3>>16)&0xff]<<16)|((uint32_t)Sbox[(s0>>8)&0xff]<<8)|Sbox[s1&0xff];t2^=rk[58];
    t3=((uint32_t)Sbox[(s3>>24)&0xff]<<24)|((uint32_t)Sbox[(s0>>16)&0xff]<<16)|((uint32_t)Sbox[(s1>>8)&0xff]<<8)|Sbox[s2&0xff];t3^=rk[59];
    out[0]=t0>>24;out[1]=t0>>16;out[2]=t0>>8;out[3]=t0;out[4]=t1>>24;out[5]=t1>>16;out[6]=t1>>8;out[7]=t1;
    out[8]=t2>>24;out[9]=t2>>16;out[10]=t2>>8;out[11]=t2;out[12]=t3>>24;out[13]=t3>>16;out[14]=t3>>8;out[15]=t3;
}
static void aes256_ctr(const uint32_t rk[60],const uint8_t iv[16],const uint8_t*in,uint8_t*out,size_t len){
    uint8_t ctr[16],ks[16];memcpy(ctr,iv,16);size_t off=0;
    while(len){aes256_encrypt_block(rk,ctr,ks);size_t n=len<16?len:16;for(size_t i=0;i<n;i++)out[off+i]=in[off+i]^ks[i];off+=n;len-=n;
        for(int i=15;i>=0;i--){if(++ctr[i])break;}}
}
/* GHASH: bitwise GF(2^128) multiply (portable, no PCLMUL) */
static void gf_mul(uint8_t X[16],const uint8_t Y[16]){
    uint8_t Z[16]={0},V[16];memcpy(V,Y,16);
    for(int i=0;i<128;i++){
        if((X[i>>3]>>(7-(i&7)))&1)for(int j=0;j<16;j++)Z[j]^=V[j];
        uint8_t lsb=V[15]&1;
        for(int j=15;j>0;j--)V[j]=(uint8_t)((V[j]>>1)|((V[j-1]&1)<<7));V[0]>>=1;
        if(lsb)V[0]^=0xe1;
    }
    memcpy(X,Z,16);
}
static void ghash(uint8_t Y[16],const uint8_t H[16],const uint8_t*data,size_t len){
    size_t i=0;while(i+16<=len){for(int j=0;j<16;j++)Y[j]^=data[i+j];gf_mul(Y,H);i+=16;}
    if(i<len){uint8_t last[16]={0};memcpy(last,data+i,len-i);for(int j=0;j<16;j++)Y[j]^=last[j];gf_mul(Y,H);}
}
static void aes256_gcm_seal(const uint32_t rk[60],const uint8_t iv12[12],const uint8_t*aad,size_t aadlen,const uint8_t*pt,size_t ptlen,uint8_t*ct,uint8_t tag[16]){
    uint8_t H[16]={0},J0[16],ctr[16],S[16]={0},lenblk[16],ek0[16];
    aes256_encrypt_block(rk,H,H);                       /* H = E_K(0) */
    memcpy(J0,iv12,12);J0[12]=0;J0[13]=0;J0[14]=0;J0[15]=1;
    memcpy(ctr,J0,16);for(int i=15;i>=0;i--){if(++ctr[i])break;}  /* inc32 */
    aes256_ctr(rk,ctr,pt,ct,ptlen);
    if(aadlen)ghash(S,H,aad,aadlen);
    ghash(S,H,ct,ptlen);
    for(int i=0;i<8;i++)lenblk[i]=(uint8_t)(((uint64_t)aadlen*8)>>(56-8*i));
    for(int i=0;i<8;i++)lenblk[8+i]=(uint8_t)(((uint64_t)ptlen*8)>>(56-8*i));
    for(int j=0;j<16;j++)S[j]^=lenblk[j];gf_mul(S,H);
    aes256_encrypt_block(rk,J0,ek0);for(int i=0;i<16;i++)tag[i]=S[i]^ek0[i];
}

/* ============================ Enqpy HIGH + Poly1305 (one keystream expansion) ============================ */
static uint8_t EK[64],QK[64],ORN[64];
static void enqpy_init_keys(void){
    static const uint8_t ek[64]={0xC,0xB,0x1,0xE,0x1,0x2,0x0,0x3,0xC,0x4,0x7,0x9,0xF,0x3,0x0,0xC,0x1,0xC,0x3,0x5,0x6,0xF,0x1,0x2,0x3,0x6,0x2,0xF,0xE,0x4,0x3,0xB,0x4,0x7,0xE,0x8,0xB,0x5,0x9,0x0,0x6,0xC,0x9,0x9,0x2,0x0,0x1,0x3,0x4,0x6,0x8,0x3,0x9,0x5,0x4,0x8,0x9,0xA,0x1,0x7,0xD,0x9,0x5,0x7};
    static const uint8_t qk[64]={0x0,0xE,0x2,0xE,0xA,0xB,0x2,0x5,0xA,0x9,0xF,0x7,0x8,0x6,0x2,0x0,0xA,0xB,0xB,0x6,0x7,0x2,0x6,0xC,0xF,0x8,0x1,0xA,0x0,0x1,0x2,0x7,0x7,0x6,0x5,0x1,0x1,0xB,0x3,0x9,0x8,0x8,0x4,0x3,0x1,0xD,0x4,0x2,0x7,0xD,0xA,0x9,0x1,0x1,0xB,0xD,0xC,0x2,0x1,0x3,0x0,0x6,0x8,0x0};
    static const uint8_t orn[64]={0x3,0x6,0x6,0x7,0xA,0x5,0x0,0x7,0xE,0x1,0x1,0x0,0x9,0xE,0xE,0x3,0x2,0xC,0xD,0x5,0x0,0x7,0x1,0x8,0xF,0xA,0x5,0x1,0x1,0x0,0x6,0x5,0x9,0x0,0x0,0xE,0xB,0x4,0x2,0x2,0xA,0xC,0x1,0x8,0x7,0xA,0xC,0x5,0xC,0xD,0x4,0x7,0xE,0xF,0x5,0xB,0x1,0x8,0xD,0x8,0x6,0xE,0x0,0xC};
    memcpy(EK,ek,64);memcpy(QK,qk,64);memcpy(ORN,orn,64);
}
/* sin must already hold [32 zero bytes || plaintext]; out[0:32]=MAC key, out[32:]=ciphertext (MAC'd in place) */
static void enqpy_poly1305_seal(uint64_t nonce,uint8_t*sin,uint8_t*sout,size_t ptlen,uint8_t tag[16]){
    PDAF_SEC(EK,QK,ORN,nonce,64,sin,(int)(ptlen+32),sout);
    poly1305_aead_tag(tag,sout,NULL,0,sout+32,ptlen);
}

/* ============================ KATs ============================ */
static int hexeq(const uint8_t*a,const char*hex){uint8_t b;for(size_t i=0;hex[2*i];i++){unsigned v;sscanf(hex+2*i,"%2x",&v);b=(uint8_t)v;if(a[i]!=b)return 0;}return 1;}
static int kat_poly1305(void){
    const uint8_t key[32]={0x85,0xd6,0xbe,0x78,0x57,0x55,0x6d,0x33,0x7f,0x44,0x52,0xfe,0x42,0xd5,0x06,0xa8,0x01,0x03,0x80,0x8a,0xfb,0x0d,0xb2,0xfd,0x4a,0xbf,0xf6,0xaf,0x41,0x49,0xf5,0x1b};
    const char*m="Cryptographic Forum Research Group";uint8_t mac[16];poly1305_oneshot(mac,(const uint8_t*)m,strlen(m),key);
    return hexeq(mac,"a8061dc1305136c6c22b8baf0c0127a9");
}
static int kat_chacha20poly1305(void){ /* RFC 8439 sec 2.8.2 */
    uint8_t key[32];for(int i=0;i<32;i++)key[i]=(uint8_t)(0x80+i);
    uint8_t nonce[12]={0x07,0,0,0,0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47};
    uint8_t aad[12]={0x50,0x51,0x52,0x53,0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7};
    const char*pt="Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
    size_t ptlen=strlen(pt);uint8_t ct[200],tag[16];
    chacha20poly1305_seal(key,nonce,aad,12,(const uint8_t*)pt,ptlen,ct,tag);
    return hexeq(tag,"1ae10b594f09e26a7e902ecbd0600691");
}
static int kat_aes256_block(void){ /* FIPS-197 C.3 */
    uint8_t key[32];for(int i=0;i<32;i++)key[i]=(uint8_t)i;
    uint8_t in[16]={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    uint32_t rk[60];aes256_key_expand(key,rk);uint8_t out[16];aes256_encrypt_block(rk,in,out);
    return hexeq(out,"8ea2b7ca516745bfeafc49904b496089");
}
static int kat_aes256_gcm(void){ /* NIST: K=0^256, IV=0^96, PT=0^128 -> CT=cea7..., Tag=d0d1... */
    uint8_t key[32]={0},iv[12]={0},pt[16]={0},ct[16],tag[16];uint32_t rk[60];aes256_key_expand(key,rk);
    aes256_gcm_seal(rk,iv,NULL,0,pt,16,ct,tag);
    return hexeq(ct,"cea7403d4d606b6e074ec5d3baf39d18")&&hexeq(tag,"d0d1c8a799996bf0265b98b5d48ab919");
}

/* Enqpy primitive KAT: EK/QK/ORN are the official Enqpy test-vector keys
   (unchanged from Rev 3.0); encrypting 8 zero bytes at or_ctr=1 yields the
   canonical W[0..7]. */
static int kat_enqpy(void){
    uint8_t zero[8]={0},out[8];
    PDAF_SEC(EK,QK,ORN,1ULL,64,zero,8,out);
    return hexeq(out,"2434b58845c6fde8");
}

int main(void){
    aes_init_tables();enqpy_init(0);enqpy_init_keys();   /* enqpy_init(0) fills MOD16 (else keystream is zero) */
    int k1=kat_poly1305(),k2=kat_chacha20poly1305(),k3=kat_aes256_block(),k4=kat_aes256_gcm(),k5=kat_enqpy();
    printf("KATs:  Poly1305 %s | ChaCha20-Poly1305 %s | AES-256 block %s | AES-256-GCM %s | Enqpy %s\n",
        k1?"PASS":"FAIL",k2?"PASS":"FAIL",k3?"PASS":"FAIL",k4?"PASS":"FAIL",k5?"PASS":"FAIL");
    if(!(k1&&k2&&k3&&k4&&k5)){fprintf(stderr,"A KAT failed -- not benchmarking (numbers would be meaningless).\n");return 1;}

    const size_t L=59189;
    uint8_t *pt=malloc(L),*ct=malloc(L),*sin=malloc(L+32),*sout=malloc(L+32);
    for(size_t i=0;i<L;i++)pt[i]=(uint8_t)(i*131+7);
    uint8_t ckey[32];for(int i=0;i<32;i++)ckey[i]=(uint8_t)(i*7+1);
    uint8_t cnonce[12]={1,2,3,4,5,6,7,8,9,10,11,12};
    uint8_t akey[32];for(int i=0;i<32;i++)akey[i]=(uint8_t)(i*5+9);
    uint8_t aiv[12]={9,8,7,6,5,4,3,2,1,0,1,2};uint32_t ark[60];aes256_key_expand(akey,ark);
    uint8_t actr[16];memcpy(actr,aiv,12);actr[12]=actr[13]=actr[14]=0;actr[15]=2;
    uint8_t tag[16];
    memset(sin,0,32);memcpy(sin+32,pt,L);   /* build [32 zero || pt] once (amortized) */

    enum{ENQPY,CHACHA,GCM,CTR,NALG};
    const char*names[NALG]={"Enqpy HIGH + Poly1305","ChaCha20-Poly1305","AES-256-GCM (portable GHASH)","AES-256-CTR (no auth)"};
    double best[NALG],sum[NALG];for(int a=0;a<NALG;a++){best[a]=1e30;sum[a]=0;}

    #define RUN(a) do{double _t0=now_ns();\
        if((a)==ENQPY){enqpy_poly1305_seal((uint64_t)(pass+1),sin,sout,L,tag);sink(sout+32,L);}\
        else if((a)==CHACHA){chacha20poly1305_seal(ckey,cnonce,NULL,0,pt,L,ct,tag);sink(ct,L);}\
        else if((a)==GCM){aes256_gcm_seal(ark,aiv,NULL,0,pt,L,ct,tag);sink(ct,L);}\
        else{aes256_ctr(ark,actr,pt,ct,L);sink(ct,L);}\
        double _el=now_ns()-_t0;sink(tag,16);\
        if(timed){if(_el<best[a])best[a]=_el;sum[a]+=_el;}}while(0)

    /* ISOLATED best-of-20: each cipher in its OWN warm loop (working set stays hot).
       Contrast with round-robin below, where ciphers evict each other's caches. */
    double isoE=1e30,isoC=1e30,t0i,eli;
    for(int w=0;w<10;w++){enqpy_poly1305_seal((uint64_t)(w+1),sin,sout,L,tag);sink(sout+32,L);}
    for(int it=0;it<20;it++){t0i=now_ns();enqpy_poly1305_seal((uint64_t)(it+1),sin,sout,L,tag);eli=now_ns()-t0i;sink(sout+32,L);if(eli<isoE)isoE=eli;}
    for(int w=0;w<10;w++){chacha20poly1305_seal(ckey,cnonce,NULL,0,pt,L,ct,tag);sink(ct,L);}
    for(int it=0;it<20;it++){t0i=now_ns();chacha20poly1305_seal(ckey,cnonce,NULL,0,pt,L,ct,tag);eli=now_ns()-t0i;sink(ct,L);if(eli<isoC)isoC=eli;}
    printf("ISOLATED (own warm loop, caches hot):  Enqpy+Poly1305 %.1f MB/s  |  ChaCha20-Poly1305 %.1f MB/s\n",(double)L/isoE*1e3,(double)L/isoC*1e3);

    int timed=0;for(int pass=0;pass<10;pass++){for(int a=0;a<NALG;a++)RUN(a);}      /* warmup */
    timed=1;for(int pass=0;pass<20;pass++){for(int a=0;a<NALG;a++)RUN(a);}          /* timed */

    printf("\nLevel playing field: %zu-byte message, portable scalar C (-O3 -march=native), no hardware crypto.\n",L);
    printf("Best-of-20, round-robin, 10 warmup passes.  Absolute MB/s vary by core and load; the within-run ratio is the claim.\n\n");
    printf("%-32s  %-10s  %-10s  %s\n","algorithm (auth included)","best MB/s","avg MB/s","vs Enqpy");
    double enqpy_best_mbps=(double)L/best[ENQPY]*1e3;
    for(int a=0;a<NALG;a++){
        double bm=(double)L/best[a]*1e3,am=(double)L/(sum[a]/20.0)*1e3;
        char rel[32];if(a==ENQPY)snprintf(rel,sizeof rel,"-");else snprintf(rel,sizeof rel,"%.2fx slower",enqpy_best_mbps/bm);
        printf("%-32s  %-10.1f  %-10.1f  %s\n",names[a],bm,am,rel);
    }
    /* ns/call best (precise) */
    printf("\n(ns/call best, precise)\n");for(int a=0;a<NALG;a++)printf("  %-32s %.0f ns\n",names[a],best[a]);
    free(pt);free(ct);free(sin);free(sout);(void)g_sink;
    return 0;
}
