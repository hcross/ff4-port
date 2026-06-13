#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C1, DP=0
// This routine iterates through a list of 12 NPCs, fetching a property 
// from the NPCProp table and loading the corresponding graphics.
static void ReloadNPCs_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    ram[0xAE] = 0;                                 // stz $ae
    uint16_t index = read16(ram, 0x09D1);          // ldx $09d1
    write16(ram, 0x09CF, index);                   // stx $09cf

    do {
        // lda f:NPCProp,x
        // NPCProp is an absolute address in the field data bank.
        // In the disassembly, NPCProp is defined as 0x2014.
        uint32_t prop_addr = 0x002014 + index; 
        cpu->a = ram[prop_addr];

        LoadNPCGfx_emu(snes);                       // jsr LoadNPCGfx

        index = read16(ram, 0x09CF);               // ldx $09cf
        index += 4;                                // inx4
        write16(ram, 0x09CF, index);               // stx $09cf

        ram[0xAE]++;                                // inc $ae
    } while (ram[0xAE] != 12);                     // lda $ae / cmp #12 / bne @c127
}

// PITFALLS: 1 (DB=$C1), 8 (Inherited mode mf=true, xf=false)
// HELPERS: LoadNPCGfx_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x09D1=2, 0x2014=1
//   output_ram:  0xAE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC1
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::ReloadNPCs ($C1:1F)