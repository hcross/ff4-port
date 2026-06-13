#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B3, DP=0
// Logic:
// 1. Computes an index from $1702 (shifted) and $1701 (offset).
// 2. Retrieves a pointer from MapDlgPtrs table based on that index.
// 3. Searches through MapDlg for a null terminator (0x00).
// 4. Skips "pseudo-terminators" 0x03 and 0x04, up to a count specified in $B2.
// 5. Stores final index in $0772 and clears $DD.
static void GetDlgPtr0_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Calculate index from $1702 and $1701
    uint8_t val1702 = ram[0x1702];
    uint8_t carry = (val1702 & 0x80) ? 1 : 0; // asl (carry bit)
    uint8_t x_low = (uint8_t)(val1702 << 1);   // Pitfall 7: 8-bit truncation
    
    ram[0x3E] = 0; // stz $3e
    ram[0x3E] = carry; // rol $3e (moves carry into $3e)
    ram[0x3D] = x_low; // sta $3d

    if (ram[0x1701] != 0) { // beq @b3cc
        ram[0x3E]++; // inc $3e
        ram[0x3E]++; // inc $3e
    }

    uint8_t idx = ram[0x3D]; // ldx $3d
    
    // MapDlgPtrs is in bank $F (ROM). We use run_emulated_func or 
    // specific memory access if known. Since we need the ROM data, 
    // we assume the harness provides access to ROM via the snes instance.
    // However, the previous attempt used snes->rom which failed.
    // We use the emulator's read mechanism for ROM bank $F.
    
    // MapDlgPtrs is at bank $F: <address>. 
    // Note: In a native reimplementation, these pointers are usually 
    // resolved to absolute addresses in the ROM image.
    uint16_t ptr_lo = snes->rom[0xMapDlgPtrs + idx]; 
    uint16_t ptr_hi = snes->rom[0xMapDlgPtrs + idx + 1];
    uint16_t current_x = (uint16_t)(ptr_lo | (ptr_hi << 8));
    
    // Actually, the ASM does:
    // ldx $3d (idx)
    // lda MapDlgPtrs,x -> sta $3d (ptr_lo)
    // lda MapDlgPtrs+1,x -> sta $3e (ptr_hi)
    // ldx $3d (X = ptr_lo) <--- CRITICAL: X is loaded with the 8-bit value of $3d
    
    uint16_t x_reg = ram[0x3D]; // ldx $3d
    uint8_t limit = ram[0xB2];
    if (limit == 0) { // beq @b3fc
        goto finish;
    }

    uint8_t y_reg = limit; // tay
    
    while (1) {
        x_reg++; // inx
        uint8_t entry = snes->rom[0xMapDlg + x_reg]; // lda f:MapDlg,x
        if (entry != 0) { // bne @b3e1
            // The original ASM logic jumps to @b3e1 on BNE.
            // If it hits 0 (BEQ), it checks the previous byte.
            uint8_t prev = snes->rom[0xMapDlg + x_reg - 1];
            if (prev == 0x03 || prev == 0x04) {
                // beq @b3e1
            } else {
                if (y_reg == 0) break; // dey / bne check
                y_reg--;
                continue; // bne @b3e1
            }
        } else {
            // entry == 0
            uint8_t prev = snes->rom[0xMapDlg + x_reg - 1];
            if (prev == 0x03 || prev == 0x04) {
                // beq @b3e1
            } else {
                if (y_reg == 0) break; 
                y_reg--;
                continue;
            }
        }
        // If the loop didn't break or continue, we effectively hit @b3e1 (inx)
    }
    x_reg++; // inx

finish:
    write16(ram, 0x0772, x_reg); // stx $0772
    write16(ram, 0x00DD, 0);     // stz $dd
}

// PITFALLS: 7 (asl/rol 8-bit truncation), 8 (mf=true, xf=false inherited)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1701=1, 0x1702=1, 0x00B2=1, MapDlgPtrs=2, MapDlg=1
//   output_ram:  0x0772=2, 0x00DD=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetDlgPtr0 ($B3:B9)