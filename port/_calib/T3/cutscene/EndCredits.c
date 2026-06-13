#include "snes/snes.h"

static void EndCredits_c(Snes *snes) {
    Cpu *c = snes->cpu;
    uint8_t *ram = snes->ram;
    
    // longi: X/Y 16-bit
    c->xf = false;
    // shorta: A 8-bit
    c->mf = true;
    
    // PHP: push processor flags
    c->sp--;
    ram[c->sp] = get_p_register(c);  // need to construct P byte
    
    // PHB: push data bank
    c->sp--;
    ram[c->sp] = c->db;
    
    // PHD: push direct page
    c->sp -= 2;
    ram[c->sp] = c->dp & 0xFF;
    ram[c->sp + 1] = (c->dp >> 8) & 0xFF;
    
    // lda #2 / sta f:$000064
    ram[0x0064] = 2;
    
    // lda #$20 / sta f:$00006a
    ram[0x006A] = 0x20;
    
    // lda #$0a / sta f:$00006b
    ram[0x006B] = 0x0A;
    
    // bra _d66b — delegate to emulator
    _d66b_emu(snes);
}