/* FF4 desktop — WRAM diff probe.
 *
 * From a seed, snapshot, run N frames native (ported C) capturing WRAM, restore,
 * run N frames pure-interpreter (ground truth) capturing WRAM, then print every
 * WRAM offset that differs. Localises exactly which bytes a ported routine gets
 * wrong, beyond the CRC the oracle reports.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes/snes.h"
#include "snes/ppu.h"

extern bool ff4_init(const uint8_t *rom_bytes, int rom_length);
extern void ff4_shutdown(void);
extern Snes *ff4_snes;
extern int  ff4_dispatch_enabled;
extern int  (*ff4_dispatch_filter)(uint32_t pc);

#define EXCL_MAX 16
static uint32_t excl[EXCL_MAX]; static int excl_n = 0;
static int filt(uint32_t pc){ for(int i=0;i<excl_n;i++) if(excl[i]==pc) return 0; return 1; }

static void ppu_state(const char *tag) {
    Ppu *p = ff4_snes->ppu;
    int cgnz = 0; for (int i = 0; i < 0x100; i++) if (p->cgram[i]) cgnz++;
    printf("  [%s] brightness=%u forcedBlank=%d cgram_nonzero=%d/256 mode=%u\n",
           tag, p->brightness, p->forcedBlank, cgnz, p->mode);
}

static uint8_t *read_file(const char *p, long *n){
    FILE *f=fopen(p,"rb"); if(!f) return NULL;
    fseek(f,0,SEEK_END); *n=ftell(f); fseek(f,0,SEEK_SET);
    uint8_t *b=malloc(*n); if(fread(b,1,*n,f)!=(size_t)*n){free(b);return NULL;} fclose(f); return b;
}

int main(int argc,char**argv){
    if(argc<3){fprintf(stderr,"usage: %s <rom> <seed.lss> [--frames N] [--exclude PC]\n",argv[0]);return 2;}
    const char *rom=argv[1], *seed=argv[2]; int frames=1;
    for(int i=3;i<argc;i++){
        if(!strcmp(argv[i],"--frames")&&i+1<argc) frames=atoi(argv[++i]);
        else if(!strcmp(argv[i],"--exclude")&&i+1<argc&&excl_n<EXCL_MAX) excl[excl_n++]=(uint32_t)strtoul(argv[++i],0,16);
    }
    long rl=0,sl=0; uint8_t *rb=read_file(rom,&rl); if(!rb||!ff4_init(rb,(int)rl)){fprintf(stderr,"init failed\n");return 1;}
    uint8_t *sb=read_file(seed,&sl); if(!sb||!snes_loadState(ff4_snes,sb,(int)sl)){fprintf(stderr,"seed load failed\n");return 1;}
    if(excl_n) ff4_dispatch_filter=filt;

    int sz=snes_saveState(ff4_snes,NULL); uint8_t*s0=malloc(sz); snes_saveState(ff4_snes,s0);
    static uint8_t wramA[0x20000], wramB[0x20000];

    ff4_dispatch_enabled=1;
    for(int i=0;i<frames;i++) snes_runFrameBounded(ff4_snes, 8000000);
    memcpy(wramA, ff4_snes->ram, 0x20000);
    uint8_t kA=ff4_snes->cpu->k; uint16_t pcA=ff4_snes->cpu->pc;
    ppu_state("native A");

    snes_loadState(ff4_snes,s0,sz);
    ff4_dispatch_enabled=0;
    for(int i=0;i<frames;i++) snes_runFrameBounded(ff4_snes, 8000000);
    memcpy(wramB, ff4_snes->ram, 0x20000);
    uint8_t kB=ff4_snes->cpu->k; uint16_t pcB=ff4_snes->cpu->pc;
    ppu_state("interp B");

    printf("frames=%d  A(native) pc=%02X:%04X  B(interp) pc=%02X:%04X\n", frames,kA,pcA,kB,pcB);
    int ndiff=0;
    for(int a=0;a<0x20000;a++){
        if(a>=0x0100 && a<0x0300) continue;  /* mask CPU stack scratch (SP top $02FF) */
        if(wramA[a]!=wramB[a]){
            if(ndiff<256)
                printf("  off $%05X = $%02X:%04X : A=%02X B=%02X\n",
                       a, 0x7E + (a>>16), a & 0xFFFF, wramA[a], wramB[a]);
            ndiff++;
        }
    }
    printf("total differing WRAM bytes: %d%s\n", ndiff, ndiff>256?" (first 256 shown)":"");
    free(s0); free(rb); free(sb); ff4_shutdown(); return 0;
}
