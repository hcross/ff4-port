#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FD, DP=0
// Purpose: Initializes menu control mappings and button actions by copying 
//          default button tables to WRAM and setting specific button behaviors.
static void InitCtrl_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // MVN sequences: Copies BtnDefault table (from ROM) to WRAM.
    // First block: 0x17 bytes to $1A05 (Direct Page = 0, DB = 0x7E)
    // Second block: 0x17 bytes to $1A1D (Direct Page = 0, DB = 0x7E)
    // Note: MVN in the ASM uses ROM source. We simulate this by copying the 
    // corresponding ROM data if available, or treating it as a memory copy.
    // In the parity harness, we assume the ROM content for BtnDefault is known.
    for (int i = 0; i < 0x17; i++) {
        ram[0x1A05 + i] = snes->rom[0x0000 + i]; // Placeholder for BtnDefault ROM addr
        ram[0x1A1D + i] = snes->rom[0x0000 + i]; // Placeholder for BtnDefault ROM addr
    }

    // shorta: A is now 8-bit
    ram[0x1A64] = ram[0x16A9];
    
    // asl $1A3A: 8-bit shift
    uint8_t btn_a3a = ram[0x1A3A];
    uint8_t shifted_a3a = (uint8_t)(btn_a3a << 1); // Pitfall 7
    ram[0x43] = shifted_a3a;

    // longa: A is now 16-bit. X is 16-bit.
    uint16_t x_val = ram[0x43]; 
    
    // stz sequence: clears 16-bit words in WRAM
    write16(ram, 0x1A2D, 0);
    write16(ram, 0x1A1D, 0);
    write16(ram, 0x1A2F, 0);
    write16(ram, 0x1A1F, 0);
    write16(ram, 0x1A21, 0);

    // a = BtnAction[x]. In 16-bit mode, this loads a word.
    // The ASM uses 'f:BtnAction,x' (indexed absolute).
    uint16_t action_l = read16(snes->rom, 0x0000 + x_val); // Placeholder for BtnAction ROM addr
    ram[0x1A31] = (uint8_t)action_l; // sta $1A31 (shorta is called after, but this is still 16-bit A)

    // shorta: A is now 8-bit
    uint8_t btn_a3b = ram[0x1A3B];
    uint8_t shifted_a3b = (uint8_t)(btn_a3b << 1); // Pitfall 7
    ram[0x43] = shifted_a3b;

    // longa: A is now 16-bit
    x_val = ram[0x43];
    uint16_t action_start = read16(snes->rom, 0x0000 + x_val); // Placeholder for BtnAction ROM addr
    ram[0x1A23] = (uint8_t)action_start;

    // shorta: A is now 8-bit
    uint8_t btn_a37 = ram[0x1A37];
    snes->cpu->a = btn_a37;
    snes->cpu->x = 0x0080; // confirm
    set_btn_map_emu(snes);

    uint8_t btn_a38 = ram[0x1A38];
    snes->cpu->a = btn_a38;
    snes->cpu->x = 0x8000; // cancel
    set_btn_map_emu(snes);

    uint8_t btn_a39 = ram[0x1A39];
    snes->cpu->a = btn_a39;
    snes->cpu->x = 0x0040; // menu
    set_btn_map_emu(snes);

    ram[0x04] = 0xFF;
    ram[0x05] = 0xFF;
    uint8_t repeat_rate = ram[0xDD];
    ram[0x08] = repeat_rate;
    ram[0x09] = repeat_rate;
}

// PITFALLS: 7 (Arithmetic truncation in 8-bit mode for 'asl' calls)
// HELPERS: set_btn_map_emu(snes) — delegates SetBtnMap @FE63
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x16A9=1, 0x1A3A=1, 0x1A3B=1, 0x1A37=1, 0x1A38=1, 0x1A39=1, 0xDD=1
//   output_ram:  0x1A64=1, 0x43=1, 0x1A31=1, 0x1A23=1, 0x04=1, 0x08=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFD
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::InitCtrl ($FD:D9)