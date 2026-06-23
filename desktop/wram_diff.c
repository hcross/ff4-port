/* FF4 desktop — WRAM diff probe (throwaway).
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
extern void (*ff4_dispatch_trace)(uint32_t pc);

static const char *g_pass_tag = "A";
static uint64_t g_uctrl_cycles_entry = 0;

/* WRAM write watchpoint — fires on every write to $7E:C1B2 or $7E:C1F2 etc. */
static void wram_watchpoint(uint32_t wram_off, uint8_t val, void *ctx) {
    (void)ctx;
    /* Watch stride-0x40 C1xx divergence addresses and common OAM high area */
    static const uint32_t watch[] = {
        0xC1B2, 0xC1F2, 0xC232, 0xC272, 0xC2B2, 0xC2F2,  /* stride-0x40 C1xx */
        0xC332, 0xC334, 0xC336, 0xC338, 0xC33A,            /* C33x */
        0x181E, 0x181F,                                     /* $181E = state machine */
        0x04A0, 0x04A1, 0x04A2, 0x04A3, 0x04A4, 0x04A5, 0x04A6, 0x04A7, /* OAM high */
        /* F07B family (active flag, 4-byte stride per slot, 6 slots) */
        0xF07B, 0xF07C, 0xF07D, 0xF07E, 0xF07F,
        /* F07C per slot: slot0=$F07C, slot1=$F080, slot2=$F084, slot3=$F088, slot4=$F08C */
        0xF080, 0xF084, 0xF088, 0xF08C,
        /* Source bytes for diverging destinations (via 9989 memmove):
         * dest $C33A = $C1A5 + 0x195 → source $BE65 + 0x195 = $BFFA
         * dest $C338 = $C1A5 + 0x193 → source $BFF8
         * dest $C336 = $C1A5 + 0x191 → source $BFF6
         * dest $C334 = $C1A5 + 0x18F → source $BFF4
         * dest $C332 = $C1A5 + 0x18D → source $BFF2
         * dest $C2F2 = $C1A5 + 0x14D → source $BFB2
         * dest $C1B2 = $C1A5 + 0x0D → source $BE72 */
        0xBFFA, 0xBFF8, 0xBFF6, 0xBFF4, 0xBFF2, 0xBFB2, 0xBE72,
        /* First dest written by 9989 memmove (highest addr, X=0x340 at start) */
        0xC4E5,
        /* Inter-BB0B anchors for slot1: STA $0300,Y after BB0B-1(Y=01E8)->$04E8, BB0B-4(Y=01F4)->$04F4 */
        0x04E8, 0x04E9, 0x04F4, 0x04F5,
        /* EFCF family: DDDC slot 0-4 frame counters */
        0xEFCF, 0xEFD0, 0xEFD1, 0xEFD2, 0xEFD3, 0xEFD4,
        /* EFCF per-slot at stride 16: slot1=$EFDF, slot2=$EFEF, etc.? No: stride is 1 in DDDC */
        /* Actually DDDC uses $EFCF,X where X=slot. So slot1 EFCF = $EFD0, slot2=$EFD1 etc. */
        /* But from the assembly: slots have stride... let me watch all nearby */
        0xEFCE, 0xEFC5, 0xEFC8,
        /* Divergence: EFE0=EFFE=slot2/5 area. Slot2 base = EFC0+2*16=EFE0, slot5 base=EFC0+5*16=F010 */
        /* More precisely: EFE0=EFC0+32 is the start of slot2's 16-byte block. F000=EFC0+64=slot4 start? */
        /* EFC0+0=EFC0, +16=EFD0, +32=EFE0, +48=EFF0, +64=F000, +80=F010 */
        0xEFE0, 0xEFE5, 0xEFEE, 0xEFEF,
        0xF000, 0xF005, 0xF00E, 0xF00F,
        /* DA73 instrumentation: $1C written at DAA4 (STA dp $1C = loop counter set to 32).
         * Hook fires with PC=02:DAA5. Gives B cycles at DA73 loop start.
         * Also: slot-0 first loop write: STA $ED50,Y with Y=$120 → WRAM[$ED70].
         * Last loop write: STA $ED50,Y with Y=$13F → WRAM[$ED8F]. */
        0x001C,
        0xED70, /* DA73 loop first write (slot 0, STA $ED50,Y Y=0x120) */
        0xED8F, /* DA73 loop last write (slot 0, STA $ED51,Y+31 = $ED70+31 = $ED8F) */
        0xF0AD, /* DA73 copy-path gate: if non-zero, skip data copy */
        0xF283, /* DA73 copy-path gate: if non-zero, skip data copy */
        /* DCED early write (DD03: STA $F08F,X) for timing B dispatch per slot */
        0xF08F, 0xF093, 0xF097, 0xF09B, 0xF09F,
        /* DCED last OAM write (iter2 attr byte = STA $0300,Y at DD8C) per slot:
         * slot0: $04A7 (already above), slot1: $04E7, slot2: $0467, slot3: $04C7, slot4: $0487 */
        0x04E7, 0x0467, 0x04C7, 0x0487,
        0
    };
    for (int i = 0; watch[i]; i++) {
        if (wram_off == watch[i]) {
            uint8_t kb = ff4_snes->cpu->k;
            uint16_t pc = ff4_snes->cpu->pc;
            printf("  [WATCH %s] WRAM[%04X] <- %02X | PC=%02X:%04X D=%04X DB=%02X cycles=%llu\n",
                   g_pass_tag, (unsigned)wram_off, val,
                   kb, pc, ff4_snes->cpu->dp, ff4_snes->cpu->db,
                   (unsigned long long)ff4_snes->cycles);
            break;
        }
    }
}

static void trace_cb(uint32_t pc) {
    uint8_t *ram = ff4_snes->ram;
    uint16_t sp = ff4_snes->cpu->sp;
    uint16_t ret = ram[sp + 1] | (ram[sp + 2] << 8);
    printf("  [TRACE %s] dispatch hit: %06X (called from %02X:%04X) | A=%04X X=%04X Y=%04X | cycles=%llu\n",
           g_pass_tag, pc, ff4_snes->cpu->k, ret + 1,
           ff4_snes->cpu->a, ff4_snes->cpu->x, ff4_snes->cpu->y,
           (unsigned long long)ff4_snes->cycles);
    if (pc == 0x14FD03) g_uctrl_cycles_entry = ff4_snes->cycles;
    /* Extra diagnostics for the monster gfx routines */
    {
        uint16_t x = ff4_snes->cpu->x;
        uint16_t dp = ff4_snes->cpu->dp;
        (void)x; (void)dp;
        /* (diagnostic disabled) */
    }
    if (pc == 0x029989) {
        printf("  [DIAG  %s] 9989(memmove): cycles=%llu 181E=%02X\n",
               g_pass_tag, (unsigned long long)ff4_snes->cycles, ram[0x181E]);
    }
    if (pc == 0x029681 || pc == 0x029682) {
        /* $9681/$9682: jump table entry */
        printf("  [DIAG  %s] 9682(jt-entry): cycles=%llu 181E=%02X 181F=%02X\n",
               g_pass_tag, (unsigned long long)ff4_snes->cycles, ram[0x181E], ram[0x181F]);
    }
    if (pc == 0x0282A1) {
        printf("  [DIAG  %s] 82A1(memmove-caller): cycles=%llu 181E=%02X ram[4A]=%02X\n",
               g_pass_tag, (unsigned long long)ff4_snes->cycles, ram[0x181E], ram[0x4A]);
    }
    if (pc == 0x02dddc || pc == 0x02dced || pc == 0x0285d2 || pc == 0x02bb0b) {
        uint16_t x = ff4_snes->cpu->x;
        uint16_t y = ff4_snes->cpu->y;
        uint16_t dp = ff4_snes->cpu->dp;
        const char *name = (pc==0x02dddc)?"DDDC":(pc==0x02dced)?"DCED":(pc==0x0285d2)?"HardMult":"BB0B";
        printf("  [DIAG  %s] %s: A=%02X X=%04X Y=%04X D=%04X ram[$47]=%02X EFBA=%02X EFC4+slot*16=%02X 1C=%02X 1E=%02X $22=%02X $16=%02X\n",
               g_pass_tag, name,
               (uint8_t)ff4_snes->cpu->a, x, y, dp,
               ram[0x47], ram[0xEFBA], ram[0xEFC4 + (uint8_t)x], ram[0x1C], ram[0x1E],
               ram[0x22], ram[0x16]);
    }
}

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

    ff4_dispatch_trace = trace_cb;
    snes_wram_write_hook = wram_watchpoint;

    int sz=snes_saveState(ff4_snes,NULL); uint8_t*s0=malloc(sz); snes_saveState(ff4_snes,s0);
    /* Dump seed WRAM values before any pass runs */
    printf("  [SEED] F07B[0..5]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xF07B+i*4]); printf("\n");
    printf("  [SEED] F07C[0..5]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xF07C+i*4]); printf("\n");
    printf("  [SEED] EFC4[slots]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xEFC4+i*16]); printf("\n");
    printf("  [SEED] EFCF[slots]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xEFCF+i*16]); printf("\n");
    printf("  [SEED] EFCE[slots]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xEFCE + i*16]); printf("\n");
    printf("  [SEED] EFC5[slots]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xEFC5+i*16]); printf("\n");
    /* Dump F015..F018 for all 6 slots (stride=4) */
    printf("  [SEED] F015[0..5]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xF015+i*4]); printf("\n");
    printf("  [SEED] F016[0..5]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xF016+i*4]); printf("\n");
    printf("  [SEED] F017[0..5]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xF017+i*4]); printf("\n");
    printf("  [SEED] F018[0..5]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xF018+i*4]); printf("\n");
    printf("  [SEED] F0AF[0..5]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xF0AF+i]); printf("\n");
    printf("  [SEED] F2BC[0..5]=");
    for(int i=0;i<6;i++) printf("%02X ",ff4_snes->ram[0xF2BC+i]); printf("\n");
    printf("  [SEED] F014=%02X ram[47]=%02X\n", ff4_snes->ram[0xF014], ff4_snes->ram[0x47]);
    printf("  [SEED] BFF2-BFFA: "); for(int i=0;i<9;i++) printf("%02X ",ff4_snes->ram[0xBFF2+i]); printf("\n");
    printf("  [SEED] C332-C33A: "); for(int i=0;i<9;i++) printf("%02X ",ff4_snes->ram[0xC332+i]); printf("\n");
    printf("  [SEED] C1A5-C1AF: "); for(int i=0;i<11;i++) printf("%02X ",ff4_snes->ram[0xC1A5+i]); printf("\n");
    printf("  [SEED] BFB2: %02X\n", ff4_snes->ram[0xBFB2]);
    printf("  [SEED] 6CC0=%02X 352D=%02X 64=%02X\n", ff4_snes->ram[0x6CC0], ff4_snes->ram[0x352D], ff4_snes->ram[0x64]);
    printf("  [SEED] F0AD=%02X F283=%02X\n", ff4_snes->ram[0xF0AD], ff4_snes->ram[0xF283]);
    printf("  [SEED] 181E=%02X 181D=%02X 181F=%02X\n", ff4_snes->ram[0x181E], ff4_snes->ram[0x181D], ff4_snes->ram[0x181F]);
    printf("  [SEED] EFC5[0..4 stride16]=%02X %02X %02X %02X %02X\n",
           ff4_snes->ram[0xEFC5], ff4_snes->ram[0xEFC5+16], ff4_snes->ram[0xEFC5+32],
           ff4_snes->ram[0xEFC5+48], ff4_snes->ram[0xEFC5+64]);
    static uint8_t wramA[0x20000], wramB[0x20000];

    ff4_dispatch_enabled=1;
    g_pass_tag = "A";
    printf("  [PRE-A] 181D-181F: %02X %02X %02X\n",
           ff4_snes->ram[0x181D], ff4_snes->ram[0x181E], ff4_snes->ram[0x181F]);
    for(int i=0;i<frames;i++) snes_runFrameBounded(ff4_snes, 8000000);
    printf("  [POST-A] 181D-181F: %02X %02X %02X cycles=%llu\n",
           ff4_snes->ram[0x181D], ff4_snes->ram[0x181E], ff4_snes->ram[0x181F],
           (unsigned long long)ff4_snes->cycles);
    memcpy(wramA, ff4_snes->ram, 0x20000);
    uint8_t kA=ff4_snes->cpu->k; uint16_t pcA=ff4_snes->cpu->pc;
    ppu_state("native A");

    snes_loadState(ff4_snes,s0,sz);
    ff4_dispatch_enabled=0;
    g_pass_tag = "B";
    printf("  [PRE-B] C332-C33A: "); for(int i=0;i<9;i++) printf("%02X ",ff4_snes->ram[0xC332+i]); printf("\n");
    printf("  [PRE-B] BFF2-BFFA: "); for(int i=0;i<9;i++) printf("%02X ",ff4_snes->ram[0xBFF2+i]); printf("\n");
    printf("  [PRE-B] BE65+195: %02X BE65+1: %02X\n", ff4_snes->ram[0xBFF2+0x188], ff4_snes->ram[0xBE66]);
    for(int i=0;i<frames;i++) snes_runFrameBounded(ff4_snes, 8000000);
    printf("  [POST-B] cycles=%llu\n", (unsigned long long)ff4_snes->cycles);
    printf("  [POST-B] C332-C33A: "); for(int i=0;i<9;i++) printf("%02X ",ff4_snes->ram[0xC332+i]); printf("\n");
    printf("  [POST-B] BFF2-BFFA: "); for(int i=0;i<9;i++) printf("%02X ",ff4_snes->ram[0xBFF2+i]); printf("\n");
    memcpy(wramB, ff4_snes->ram, 0x20000);
    uint8_t kB=ff4_snes->cpu->k; uint16_t pcB=ff4_snes->cpu->pc;
    ppu_state("interp B");

    printf("frames=%d  A(native) pc=%02X:%04X  B(interp) pc=%02X:%04X\n", frames,kA,pcA,kB,pcB);
    /* Dump key state after each pass for monster gfx debugging */
    printf("  [DBG] F078 A=%02X B=%02X  F077 A=%02X B=%02X\n",
           wramA[0xF078], wramB[0xF078], wramA[0xF077], wramB[0xF077]);
    printf("  [DBG] F08F[0..5] A="); for(int i=0;i<6;i++) printf("%02X ",wramA[0xF08F+i]); printf("\n");
    printf("  [DBG] F08F[0..5] B="); for(int i=0;i<6;i++) printf("%02X ",wramB[0xF08F+i]); printf("\n");
    printf("  [DBG] EFCF[slots] A="); for(int i=0;i<6;i++) printf("%02X ",wramA[0xEFCF+i*16]); printf("\n");
    printf("  [DBG] EFCF[slots] B="); for(int i=0;i<6;i++) printf("%02X ",wramB[0xEFCF+i*16]); printf("\n");
    printf("  [DBG] OAM $0300[0..7] A="); for(int i=0;i<8;i++) printf("%02X ",wramA[0x0300+i]); printf("\n");
    printf("  [DBG] OAM $0300[0..7] B="); for(int i=0;i<8;i++) printf("%02X ",wramB[0x0300+i]); printf("\n");
    printf("  [DBG] F07B[0..5] A="); for(int i=0;i<6;i++) printf("%02X ",wramA[0xF07B+i*4]); printf("\n");
    printf("  [DBG] F07B[0..5] B="); for(int i=0;i<6;i++) printf("%02X ",wramB[0xF07B+i*4]); printf("\n");
    printf("  [DBG] F07C[0..5] A="); for(int i=0;i<6;i++) printf("%02X ",wramA[0xF07C+i*4]); printf("\n");
    printf("  [DBG] F07C[0..5] B="); for(int i=0;i<6;i++) printf("%02X ",wramB[0xF07C+i*4]); printf("\n");
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
