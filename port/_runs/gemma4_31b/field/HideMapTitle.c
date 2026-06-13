#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B8, DP=0
// This routine hides the map title by triggering a DMA fill of a specific 
// VRAM area. It only executes if the state in ram[0xEA] is exactly 0x01.
static void HideMapTitle_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0xEA] != 0x01) {    // lda $ea / cmp #$01 / beq @b8b0
        return;
    }

    ram[0xEA]++;                 // inc $ea
    
    // Setup DMA for VRAM clear
    // Note: hVMAINC and hVMADDL are macros for specific memory-mapped I/O
    // based on the assembly, we treat them as writes to the provided addresses.
    
    snes->cpu->a = 0x80;
    ram[0x4301] = 0x80;          // sta hVMAINC (0x4301)
    
    InitDMA_emu(snes);           // jsr InitDMA (delegated)
    
    snes->cpu->a = 0x09;
    ram[0x4300] = 0x09;          // sta $4300
    
    snes->cpu->x = 0x2840;
    write16(ram, 0x4301, 0x2840); // stx hVMADDL (0x4301)
    
    snes->cpu->a = 0;            // stz $10
    ram[0x10] = 0;
    
    snes->cpu->x = 0x0610;
    write16(ram, 0x4302, 0x0610); // stx $4302
    
    snes->cpu->x = 0x0100;
    write16(ram, 0x4305, 0x0100); // stx $4305
    
    ExecDMA_emu(snes);           // jsr ExecDMA (delegated)
}

// PITFALLS: 1 (DB=$B8), 6 (A 8-bit), 8 (Inherited mf=true)
// HELPERS: InitDMA_emu(snes), ExecDMA_emu(snes), write16()
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xEA=1
//   output_ram:  0xEA=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB8
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::HideMapTitle ($B8:A9)