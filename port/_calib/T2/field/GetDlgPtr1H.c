#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Input: none (reads internal state via GetDlgID)
// Output: ram[$0772] = pointer (16-bit), ram[$dd] = 1
static void GetDlgPtr1H_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // jsr GetDlgID (returns dialog ID in A, used as index X)
    uint16_t x = get_dlg_id_emu(snes);  // X = dialog ID * 2 (word index)
    // Read pointer from EventDlg1HPtrs table (hardcoded offset in ROM)
    uint8_t ptr_lo = snes->rom[0x1B41D + x];      // f:@EventDlg1HPtrs,x
    uint8_t ptr_hi = snes->rom[0x1B41D + 1 + x];  // f:@EventDlg1HPtrs+1,x
    ram[0x3D] = ptr_lo;
    ram[0x3E] = ptr_hi;
    // Transfer pointer to $0772
    ram[0x0772] = ptr_lo;
    ram[0x0773] = ptr_hi;
    // Set $dd = 1
    ram[0xdd] = 1;
}

// PITFALLS: 1 (DB=$7E assumed), 9 (upper byte B of A preserved in X if
// GetDlgID returns in 8-bit mode — but since X is used as a full 16-bit
// index, we delegate GetDlgID to ensure precision)
// HELPERS: get_dlg_id_emu(snes) — delegates GetDlgID @ $B4:4D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0772=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetDlgPtr1H ($B4:1B)