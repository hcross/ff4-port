#include "snes/snes.h"

// Cutscene routine: sets up DMA channel 1 with length 0 (no-op transfer)
// and sets VRAM address to Y. Preserves DB and A.
// Entry: A = source bank (8-bit), X = source address low word (16-bit),
//        Y = VRAM address low word (16-bit).
// Mode: mf=1 (A 8-bit), xf=0 (X/Y 16-bit), dp=caller's DP, db=caller's DB.
static void _13ddd6_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t *mmio = snes->mmio; // hardware registers

    // Save DB and A on stack
    uint16_t sp = stack_addr(cpu);
    snes->ram[sp] = cpu->db;          // phb (1 byte)
    cpu->sp--;
    // pha: push A (size depends on mf)
    if (cpu->mf) {
        // 8-bit A: push low byte only
        snes->ram[stack_addr(cpu)] = (uint8_t)cpu->a;
        cpu->sp--;
    } else {
        // 16-bit A: push low then high
        snes->ram[stack_addr(cpu)] = (uint8_t)cpu->a;
        cpu->sp--;
        snes->ram[stack_addr(cpu)] = (uint8_t)(cpu->a >> 8);
        cpu->sp--;
    }

    // clr_a: tdc (A = D, 16-bit transfer)
    cpu->a = cpu->dp;

    // pha: push D (same size as before)
    if (cpu->mf) {
        snes->ram[stack_addr(cpu)] = (uint8_t)cpu->a;
        cpu->sp--;
    } else {
        snes->ram[stack_addr(cpu)] = (uint8_t)cpu->a;
        cpu->sp--;
        snes->ram[stack_addr(cpu)] = (uint8_t)(cpu->a >> 8);
        cpu->sp--;
    }

    // plb: pull DB (1 byte)
    cpu->sp++;
    cpu->db = snes->ram[stack_addr(cpu)];

    // pla: pull original A (size depends on mf)
    cpu->sp++;
    if (cpu->mf) {
        cpu->a = (cpu->a & 0xFF00) | snes->ram[stack_addr(cpu)]; // B preserved
    } else {
        uint8_t lo = snes->ram[stack_addr(cpu)];
        cpu->sp++;
        uint8_t hi = snes->ram[stack_addr(cpu)];
        cpu->a = (hi << 8) | lo;
    }

    // Now set hardware registers
    // sty hVMADDL ($2116) - 16-bit store
    mmio[0x2116] = (uint8_t)cpu->y;
    mmio[0x2117] = (uint8_t)(cpu->y >> 8);

    // stx $4352 (A1T low word) - 16-bit store
    mmio[0x4352] = (uint8_t)cpu->x;
    mmio[0x4353] = (uint8_t)(cpu->x >> 8);

    // sta $4354 (A1B) - 8-bit store
    mmio[0x4354] = (uint8_t)cpu->a;

    // lda #$01; sta $4350 (DMAP)
    mmio[0x4350] = 0x01;

    // lda #<hVMDATAL; sta $4351 (BBAD)
    mmio[0x4351] = 0x18; // low byte of $2118

    // ldx $00; stx $4355 (DAS) - 16-bit store, length = 0
    mmio[0x4355] = 0;
    mmio[0x4356] = 0;

    // lda #$20; sta hMDMAEN ($420B)
    mmio[0x420B] = 0x20;

    // plb: restore original DB
    cpu->sp++;
    cpu->db = snes->ram[stack_addr(cpu)];

    // rts
}