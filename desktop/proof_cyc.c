/* Throwaway proof: does charging cycles per dispatch hit re-converge native? */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snes/snes.h"

extern bool ff4_init(const uint8_t *rom_bytes, int rom_length);
extern void ff4_step(void);
extern Snes *ff4_snes;
extern void (*ff4_dispatch_trace)(uint32_t pc);
extern void snes_runCycles(Snes* snes, int cycles);

/* per-hit cycle charge (set via env CHARGE, default 0) */
static int g_charge = 0;
static void charge_cb(uint32_t pc) {
    if (ff4_snes) {
        printf("  HIT frame=%3d pc=%06x\n", ff4_snes->frames, pc);
        if (g_charge > 0) snes_runCycles(ff4_snes, g_charge);
    }
}

/* SP-leak detector: log dispatch hits whose SP looks abnormal, and track the
 * SP delta the simulated RTS/RTL produced. We hook the trace (called BEFORE the
 * body) to record SP-in; a separate post check is impossible from trace alone,
 * so instead we log every hit's pc+sp when sp is outside [0x0100,0x0700]. */
static int g_spwatch = 0;
static void spwatch_cb(uint32_t pc) {
    if (!ff4_snes) return;
    uint16_t sp = ff4_snes->cpu->sp;
    if (sp < 0x0100 || sp > 0x0700)
        printf("  HIT pc=%06x sp=%04x ABNORMAL frame=%u\n", pc, sp, ff4_snes->frames);
}

static uint8_t *rf(const char *p, long *n){FILE*f=fopen(p,"rb");if(!f)return 0;fseek(f,0,SEEK_END);*n=ftell(f);fseek(f,0,SEEK_SET);uint8_t*b=malloc(*n);fread(b,1,*n,f);fclose(f);return b;}

int main(int argc, char**argv){
    const char*rom=argv[1]; const char*lss=argv[2]; int frames=atoi(argv[3]);
    if(getenv("CHARGE")) g_charge=atoi(getenv("CHARGE"));
    long rn; uint8_t*rom_b=rf(rom,&rn);
    ff4_init(rom_b,(int)rn);
    long sn; uint8_t*st=rf(lss,&sn);
    snes_loadState(ff4_snes,st,(int)sn);
    extern int ff4_dispatch_enabled;
    if(getenv("NODISP")) ff4_dispatch_enabled = 0;
    if(g_charge>0) ff4_dispatch_trace = charge_cb;
    int trace_pc = getenv("TRACEPC") ? 1 : 0;
    for(int i=0;i<frames;i++){ ff4_step();
        if(trace_pc) {
            Cpu *cpu = ff4_snes->cpu;
            Ppu *ppu = ff4_snes->ppu;
            printf("  frame %3d post pc=%02x:%04x sp=%04x a=%04x x=%04x y=%04x db=%02x dp=%04x mf=%d xf=%d brt=%u fb=%d\n", i+1,
                cpu->k, cpu->pc, cpu->sp, cpu->a, cpu->x, cpu->y, cpu->db, cpu->dp, cpu->mf, cpu->xf, ppu->brightness, ppu->forcedBlank);
        }
    }
    printf("CHARGE=%d frames=%d brightness=%u forcedBlank=%d pc=%02x:%04x cycles=%llu\n",
        g_charge, frames, ff4_snes->ppu->brightness, ff4_snes->ppu->forcedBlank,
        ff4_snes->cpu->k, ff4_snes->cpu->pc, (unsigned long long)ff4_snes->cycles);
    return 0;
}
