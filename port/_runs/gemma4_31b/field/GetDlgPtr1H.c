#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B4, DP=0
// Purpose: Retrieve a pointer from the high-priority dialog table based on 
//          the ID found by GetDlgID, store it in temporary WRAM, and update $0772.
static void GetDlgPtr1H_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Determine dialog ID (returns value in X)
    GetDlgID_emu(snes);
    uint16_t x = snes->cpu->x;

    // EventDlg1HPtrs = EventDlg1Ptrs + 512
    // The table resides in ROM bank $B4. 
    // Based on the disassembly and typical FF4 mapping, we calculate the 
    // ROM address. Note: In a native C reimplementation, the ROM is 
    // accessed via snes->rom.
    // @EventDlg1HPtrs is a label in bank $B4.
    uint32_t table_addr = 0x00B40000 + 0x1B + 512; // Adjusted for label @b41b context
    
    // lda f:@EventDlg1HPtrs,x / sta $3d
    // A is 8-bit, so we read a single byte from ROM.
    uint8_t ptr_lo = snes->rom[table_addr + x];
    ram[0x3D] = ptr_lo;

    // lda f:@EventDlg1HPtrs+1,x / sta $3e
    uint8_t ptr_hi = snes->rom[table_addr + 1 + x];
    ram[0x3E] = ptr_hi;

    // ldx $3d / stx $0772
    // X is 16-bit (xf=0). Loading $3D pulls a word (3D and 3E).
    uint16_t ptr_val = read16(ram, 0x3D);
    write16(ram, 0x0772, ptr_val);

    // lda #1 / sta $dd
    ram[0xDD] = 1;
}

// PITFALLS: 6 (Mode A: 8-bit for ROM reads, Mode X: 16-bit for ldx $3d)
// HELPERS: GetDlgID_emu(snes), read16, write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0772=2, 0x3D=1, 0x3E=1, 0xDD=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0xB4
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetDlgPtr1H ($B4:1B)