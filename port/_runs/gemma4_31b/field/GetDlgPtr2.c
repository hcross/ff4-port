#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B4 (ROM Bank), DP=0
// Logic:
//   1. Resolve the dialog ID via GetDlgID (which updates X).
//   2. Fetch a 16-bit pointer from the EventDlg2Ptrs ROM table using X as index.
//   3. Store the pointer in DP $3D-$3E and absolute WRAM $0772.
//   4. Write value 2 to DP $DD.
static void GetDlgPtr2_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Resolve dialog ID into X
    GetDlgID_emu(snes);
    uint16_t x = snes->cpu->x;

    // EventDlg2Ptrs is a ROM table. Based on 'lda f:EventDlg2Ptrs,x' 
    // and 'lda f:EventDlg2Ptrs+1,x', X is treated as a byte offset.
    // We access the ROM via snes->rom (or mapped memory for the harness).
    extern uint8_t EventDlg2Ptrs[];
    
    uint8_t ptr_low = EventDlg2Ptrs[x];
    uint8_t ptr_high = EventDlg2Ptrs[x + 1];

    // Store to DP $3D and $3E (DP=0)
    ram[0x3D] = ptr_low;
    ram[0x3E] = ptr_high;

    // Write 16-bit value to absolute WRAM $0772
    write16(ram, 0x0772, (uint16_t)(ptr_low | (ptr_high << 8)));

    // Set DP $DD to 2
    ram[0xDD] = 2;
}

// PITFALLS: 1 (DP=0, absolute writes to $0772), 6 (Mode A 8-bit logic)
// HELPERS: GetDlgID_emu(snes), write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0772=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB4
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetDlgPtr2 ($B4:34)