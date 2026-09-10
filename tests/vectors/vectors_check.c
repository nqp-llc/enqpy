/* Verify enqpy-vectors.json v5.1 against the Rev 5.1 reference.
 * gcc -O2 -std=c11 vectors_check.c -o vectors_check && ./vectors_check */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enqpy_reference.c"

static int hex2nib(const char *h, uint8_t *o, int n)
{ for(int i=0;i<n;i++){char c=h[i];int v=(c>='0'&&c<='9')?c-'0':(c>='A'&&c<='F')?c-'A'+10:-1;if(v<0)return -1;o[i]=(uint8_t)v;} return 0; }
static int hex2byt(const char *h, uint8_t *o, int nb)
{ for(int i=0;i<nb;i++){unsigned v;if(sscanf(h+2*i,"%2x",&v)!=1)return -1;o[i]=(uint8_t)v;} return 0; }
static int chk(const char *id, int ok){ printf("  %-28s %s\n", id, ok?"PASS":"*** FAIL ***"); return !ok; }

#define EKH "CB1E1203C479F30C1C356F12362FE43B47E8B5906C992013468395489A17D957"
#define QKH "0E2EAB25A9F78620ABB6726CF81A012776511B3988431D427DA911BDC2130680"
#define ORH "3667A507E1109EE32CD50718FA511065900EB422AC187AC5CD47EF5B18D86E0C"

int main(void)
{
    int fail = 0;
    enqpy_init(0);
    uint8_t ek[64], qk[64], orn[64];
    hex2nib(EKH, ek, 64); hex2nib(QKH, qk, 64); hex2nib(ORH, orn, 64);

    puts("enqpy-vectors.json v5.1 vs Rev 5.1 reference");
    puts("===========================================");

    { uint8_t in[6], out[8]; hex2nib("FCB578", in, 6);
      int r = OWC(in, 6, 1, out); char g[9]={0};
      for (int i=0;i<r&&i<8;i++) g[i]="0123456789ABCDEF"[out[i]];
      fail += chk("owc-0001", strcmp(g,"B0F")==0); }

    { uint8_t vk[10], ok_[10], out[30]; hex2nib("FB382C001A",vk,10); hex2nib("CC69100AB4",ok_,10);
      const char *exp[2]={"B7913C0ACE7FEBD00B53F4851014AF","7DD02C010CDF74C01B5BF8D811B92B"};
      const char *id[2]={"pdaf0-0001","pdaf1-0001"};
      for (int m=0;m<2;m++){ PDAF(vk,ok_,10,m,30,out); char g[31]={0};
        for(int i=0;i<30;i++) g[i]="0123456789ABCDEF"[out[i]];
        fail += chk(id[m], strcmp(g,exp[m])==0); } }

    { uint8_t pt[8]={0}, ct[8], rt[8], e[8]; hex2byt("2434B58845C6FDE8", e, 8);
      PDAF_SEC(ek,qk,orn,1ULL,64,pt,8,ct);
      fail += chk("sec-0001-zeroPT ct", memcmp(ct,e,8)==0);
      PDAF_SEC(ek,qk,orn,1ULL,64,ct,8,rt);
      int ok=1; for(int i=0;i<8;i++) if(rt[i]) ok=0;
      fail += chk("sec-0001 roundtrip", ok);
      uint8_t ek8[64],qk8[64],c1[8],c2[8],c3[8];
      for(int i=0;i<64;i++){ek8[i]=(ek[i]+8)&0xF;qk8[i]=(qk[i]+8)&0xF;}
      PDAF_SEC(ek8,qk ,orn,1ULL,64,pt,8,c1);
      PDAF_SEC(ek ,qk8,orn,1ULL,64,pt,8,c2);
      PDAF_SEC(ek8,qk8,orn,1ULL,64,pt,8,c3);
      fail += chk("coset_ek_plus_8",    memcmp(c1,e,8)==0);
      fail += chk("coset_qk_plus_8",    memcmp(c2,e,8)==0);
      fail += chk("coset_ek_qk_plus_8", memcmp(c3,e,8)==0); }

    { uint8_t pt[8]={0}, ct[8], rt[8], e[8]; hex2byt("B8DCA43E483718B3", e, 8);
      int r = PDAF_SEC(ek,ek,orn,1ULL,64,pt,8,ct);
      fail += chk("sec-0002 accepted (rc=8)", r==8);
      fail += chk("sec-0002 ct", memcmp(ct,e,8)==0);
      PDAF_SEC(ek,ek,orn,1ULL,64,ct,8,rt);
      int ok=1; for(int i=0;i<8;i++) if(rt[i]) ok=0;
      fail += chk("sec-0002 roundtrip", ok); }

    { uint8_t ct[1]; int r = PDAF_SEC(ek,qk,orn,1ULL,64,(const uint8_t*)"",0,ct);
      fail += chk("sec-0003 emptyPT (rc=0)", r==0); }

    { uint8_t *pt = calloc(2050,1), *ct = calloc(2050,1), *rt = calloc(2050,1);
      PDAF_SEC(ek,qk,orn,1ULL,64,pt,2048,ct);
      fail += chk("sec-0004 tail == 5428", ct[2046]==0x54 && ct[2047]==0x28);
      PDAF_SEC(ek,qk,orn,1ULL,64,pt,2050,ct);
      PDAF_SEC(ek,qk,orn,1ULL,64,ct,2050,rt);
      int ok=1; for(int i=0;i<2050;i++) if(rt[i]) { ok=0; break; }
      fail += chk("sec-0004 2050B roundtrip", ok);
      free(pt); free(ct); free(rt); }

    /* LOW (n=32) and MEDIUM (n=48): keys are the LAST n nibbles of the canonical values */
    { struct { const char *name; int n; const char *zero, *eq, *tail; int win; } P[2] = {
        { "low",    32, "880D1D26F1A183AD", "A26E1C021AD0230E", "5FD3", 512  },
        { "medium", 48, "AD3855C36B7D4B41", "7DF090DDBA60C8CC", "9664", 1152 } };
      for (int k = 0; k < 2; k++) {
        int n = P[k].n; char id[64];
        uint8_t e[64], q[64], o[64], pt[8]={0}, ct[8], rt[8], exp[8];
        hex2nib(EKH + (64-n), e, n); hex2nib(QKH + (64-n), q, n); hex2nib(ORH + (64-n), o, n);

        hex2byt(P[k].zero, exp, 8);
        PDAF_SEC(e,q,o,1ULL,n,pt,8,ct);
        snprintf(id,sizeof id,"sec-%s-zeroPT ct", P[k].name);
        fail += chk(id, memcmp(ct,exp,8)==0);
        PDAF_SEC(e,q,o,1ULL,n,ct,8,rt);
        { int ok=1; for(int i=0;i<8;i++) if(rt[i]) ok=0;
          snprintf(id,sizeof id,"sec-%s roundtrip", P[k].name); fail += chk(id, ok); }

        { uint8_t e8[64],q8[64],c1[8],c2[8],c3[8];
          for(int i=0;i<n;i++){e8[i]=(e[i]+8)&0xF;q8[i]=(q[i]+8)&0xF;}
          PDAF_SEC(e8,q ,o,1ULL,n,pt,8,c1);
          PDAF_SEC(e ,q8,o,1ULL,n,pt,8,c2);
          PDAF_SEC(e8,q8,o,1ULL,n,pt,8,c3);
          snprintf(id,sizeof id,"sec-%s cosets", P[k].name);
          fail += chk(id, !memcmp(c1,exp,8)&&!memcmp(c2,exp,8)&&!memcmp(c3,exp,8)); }

        hex2byt(P[k].eq, exp, 8);
        { int r = PDAF_SEC(e,e,o,1ULL,n,pt,8,ct);
          snprintf(id,sizeof id,"sec-%s ekEqualsQk", P[k].name);
          fail += chk(id, r==8 && memcmp(ct,exp,8)==0); }

        { int win = P[k].win; uint8_t t[2]={0}; hex2byt(P[k].tail, t, 2);
          uint8_t *wp=calloc(win+2,1), *wc=calloc(win+2,1), *wr=calloc(win+2,1);
          PDAF_SEC(e,q,o,1ULL,n,wp,win,wc);
          snprintf(id,sizeof id,"sec-%s window tail", P[k].name);
          fail += chk(id, wc[win-2]==t[0] && wc[win-1]==t[1]);
          PDAF_SEC(e,q,o,1ULL,n,wp,win+2,wc);
          PDAF_SEC(e,q,o,1ULL,n,wc,win+2,wr);
          int ok=1; for(int i=0;i<win+2;i++) if(wr[i]){ok=0;break;}
          snprintf(id,sizeof id,"sec-%s window roundtrip", P[k].name);
          fail += chk(id, ok);
          free(wp); free(wc); free(wr); }
      } }

    /* profiles must disagree -- a port that ignores n must not pass */
    { uint8_t pt[8]={0}, c32[8], c48[8], c64[8], e[64], q[64], o[64];
      hex2nib(EKH+32,e,32); hex2nib(QKH+32,q,32); hex2nib(ORH+32,o,32);
      PDAF_SEC(e,q,o,1ULL,32,pt,8,c32);
      hex2nib(EKH+16,e,48); hex2nib(QKH+16,q,48); hex2nib(ORH+16,o,48);
      PDAF_SEC(e,q,o,1ULL,48,pt,8,c48);
      hex2nib(EKH,e,64); hex2nib(QKH,q,64); hex2nib(ORH,o,64);
      PDAF_SEC(e,q,o,1ULL,64,pt,8,c64);
      fail += chk("profiles are distinct", memcmp(c32,c48,8) && memcmp(c32,c64,8) && memcmp(c48,c64,8)); }

    /* pdaf_mode1_selfref: Phase-1 OR_EXP, FCD 13.2a */
    { struct { const char *id; unsigned long long ctr; const char *out; } S[3] = {
        { "pdaf1sr-0001", 1ULL,      "0000000000000011" },
        { "pdaf1sr-0002", 2ULL,      "0000000000000022" },
        { "pdaf1sr-0003", 0xA5C3ULL, "00000000000AA5C3" } };
      for (int k = 0; k < 3; k++) {
        uint8_t cn[64]={0}, out[64];
        for (int i = 0; i < 16; i++) cn[i] = (uint8_t)((S[k].ctr >> (60 - i*4)) & 0xF);
        PDAF(cn, cn, 64, 1, 64, out);
        int ok = 1;
        for (int i = 0; i < 16; i++) {
          int e = (S[k].out[i] >= '0' && S[k].out[i] <= '9') ? S[k].out[i]-'0' : S[k].out[i]-'A'+10;
          if (out[i] != e) ok = 0; }
        for (int i = 16; i < 64; i++) if (out[i]) ok = 0;   /* zero tail */
        fail += chk(S[k].id, ok); } }

    /* nil_comm_update: rotation policy */
    { uint8_t e[64], q[64], ex[64], en[64], qn[64];
      hex2nib(EKH, e, 64); hex2nib(QKH, q, 64);
      for (int i = 0; i < 64; i++) ex[i] = (uint8_t)((i*11 + 3) & 0xF);

      fail += chk("nil-0001 method1 rejected",
                  ENQPY_NIL_COMM_UPDATE(e,q,64,NULL,1,en,qn) == -1);

      int r = ENQPY_NIL_COMM_UPDATE(e,q,64,ex,2,en,qn);
      uint8_t ekn[64], qkn[64];
      hex2nib("072142F4DA9C51C7FA6975F0190D369D0A90BEB4A6A9BD4A131CDF19AFE4F816", ekn, 64);
      hex2nib("9EC9FDAB010DF627ABA7DA614A1AB2A8F86BAE443A502D627D6FC3BFEA13BE28", qkn, 64);
      fail += chk("nil-0002 method2 accepted",
                  r == 0 && !memcmp(en,ekn,64) && !memcmp(qn,qkn,64));

      { uint8_t e8[64], q8[64], en8[64], qn8[64];
        for (int i = 0; i < 64; i++) { e8[i]=(e[i]+8)&0xF; q8[i]=(q[i]+8)&0xF; }
        ENQPY_NIL_COMM_UPDATE(e8,q8,64,ex,2,en8,qn8);
        fail += chk("nil-0002 coset -> one pair",
                    !memcmp(en,en8,64) && !memcmp(qn,qn8,64)); }

      fail += chk("nil-0003 method2 no entropy",
                  ENQPY_NIL_COMM_UPDATE(e,q,64,NULL,2,en,qn) == -1); }

    printf("\n%s\n", fail ? "*** MISMATCH -- do not publish ***"
                          : "All v5.1 published vectors reproduce exactly.");
    return fail ? 1 : 0;
}
