#include "snes/snes.h"

// GetDlgPtr1H: loads a 16-bit dialog pointer from a ROM table indexed by
// the result of GetDlgID, stores it to $0772, and sets $DD = 1.
//
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DP=0, DB=$7E
// (inherited from field module convention; no explicit size directives)
static void GetDlgPtr1H_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t *ram = snes->ram;

    // Delegate GetDlgID — sets X to the dialog index
    cpu->db = 0x7E;          // field module DB convention
    get_dlg_id_emu(snes);

    uint16_t index = cpu->x;

    // ROM table base: EventDlg1HPtrs = EventDlg1Ptrs + 512 (bank $B4)
    // The auto-spike generator will replace EVENT_DLG1_HPTRS_OFFSET with
    // the actual numeric offset.
    const uint32_t base = 0xB40000 + EVENT_DLG1_HPTRS_OFFSET;

    // Read the 16-bit pointer bytewise (A is 8-bit)
    uint8_t lo = snes->rom[base + index];
    uint8_t hi = snes->rom[base + index + 1];

    // Store to $3D/$3E (direct page, DP=0)
    ram[0x3D] = lo;
    ram[0x3E] = hi;

    // ldx $3d / stx $0772  →  copy the 16-bit pointer to $0772
    uint16_t ptr = read16(ram, 0x3D);
    write16(ram, 0x0772, ptr);

    // lda #1 / sta $dd
    ram[0xDD] = 1;
}

// PITFALLS: none (no conditional branches, no flag-sensitive entry,
//          no mode-dependent arithmetic that would trigger truncation)
// HELPERS: get_dlg_id_emu(snes) — delegates GetDlgID @ $B4:4D
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x0772=2, 0x00DD=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
// REVERSED_FUNCTION: field::GetDlgPtr1H ($B4:1B)