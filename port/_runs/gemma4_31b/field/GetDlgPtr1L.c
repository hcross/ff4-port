#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B4, DP=0
// Logic:
//   1. Calls GetDlgID to determine the dialogue index (returns in A).
//   2. Uses the value in X (inherited from caller) to index into the EventDlg1Ptrs table.
//   3. Stores the resulting 16-bit pointer into RAM $3D and $0772.
//   4. Sets a dialogue state flag at $DD to 1.
static void GetDlgPtr1L_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // jsr GetDlgID
    GetDlgID_emu(snes);

    // The ASM reads 'lda f:EventDlg1Ptrs,x' and 'lda f:EventDlg1Ptrs+1,x'
    // where x is the index. In this specific routine, X is not modified 
    // before the load, meaning it's inherited from the caller.
    uint16_t x = snes->cpu->x;
    
    // EventDlg1Ptrs is a table of pointers. 
    // The ASM performs two 8-bit loads to fetch a 16-bit value.
    // We use the ROM address of the symbol EventDlg1Ptrs.
    uint16_t ptr = read16(snes->rom, 0xEventDlg1Ptrs + x);

    // sta $3d / sta $3e
    write16(ram, 0x3D, ptr);

    // ldx $3d / stx $0772
    // With DP=0, ldx $3d loads the 16-bit value at $3D (which we just wrote)
    write16(ram, 0x0772, ptr);

    // lda #1 / sta $dd
    ram[0xDD] = 1;
}

// PITFALLS: 1 (DB=$B4), 8 (mf=true, xf=false inherited from caller)
// HELPERS: GetDlgID_emu(snes) — delegates GetDlgID @ $B4:4D
//          read16/write16 — little-endian accessors
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  none
//   output_ram:  0x0772=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB4
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetDlgPtr1L ($B4:02)