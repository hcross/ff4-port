#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C3, DP=0
// Logic:
//   If ram[0x1704] != 6, return immediately.
//   Otherwise, use ram[0x7A] to index into the WhalePal table.
//   The index is (ram[0x7A] >> 1) & 0x0E.
//   Two bytes from the table are written to ram[0x0EC7] and ram[0x0EC8].
static void UpdateWhalePal_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0x1704] != 0x06) { // cmp #$06 / bne @c3d9
        return;
    }

    // lsr2 is a macro for two LSR instructions (>> 2)
    // then and #$0e
    uint8_t index = (ram[0x7A] >> 2) & 0x0E; 
    
    // f:WhalePal is a ROM address. 
    // Accessing it as snes->rom[WhalePal + index]
    // Based on FF4 mapping, we assume a helper or direct ROM access.
    // Since ROM access isn't explicitly in the API, we treat f:WhalePal as an external table.
    
    // Mapping f:WhalePal based on disassembly symbols (assuming rom pointer)
    // For parity purposes, we access the ROM bytes.
    uint8_t *rom = snes->rom; 
    uint32_t whale_pal_addr = 0x00C3BD; // This is the routine addr; actual table is in ROM.
    // In a real implementation, WhalePal would be a defined constant.
    extern uint8_t WhalePal[]; 

    ram[0x0EC7] = WhalePal[index];       // lda WhalePal,x / sta $0ec7
    ram[0x0EC8] = WhalePal[index + 1];   // lda WhalePal+1,x / sta $0ec8
}

// PITFALLS: 7 (8-bit shift truncation handled by uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1704=1, 0x7A=1
//   output_ram:  0x0EC7=1, 0x0EC8=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateWhalePal ($C3:BD)