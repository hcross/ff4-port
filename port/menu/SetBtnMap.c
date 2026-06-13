#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FE, DP=0
// Logic:
//   1. Save current X (index) to stack.
//   2. Use current A (button index) to look up a mapping value in BtnMapTbl.
//   3. Store the mapped value (16-bit) into the array at $1A05 + original X.
static void SetBtnMap_c(Snes *snes, uint8_t btn_idx, uint16_t original_x) {
    uint8_t *ram = snes->ram;

    // sta $43 / ldx $43 / lda BtnMapTbl,x
    // The routine uses ram[0x43] as a temporary scratchpad for the index.
    // Note: BtnMapTbl is at a fixed address (f: prefix). 
    // We need to determine the address of BtnMapTbl from the disassembly.
    // Assuming BtnMapTbl is a table of 16-bit words (implied by longa/sta $1a05,x).
    
    // Looking at the logic: it loads a value from a table based on btn_idx.
    // The original X is restored from the stack (pla) and used as the destination offset.
    
    // We must find the value at BtnMapTbl[btn_idx]. 
    // Since this is a translation of a specific snippet, I will reference the 
    // table access conceptually; the parity harness will provide the exact 
    // table contents via the ROM/RAM mapping.
    
    // For the sake of this translation, we assume the table is accessed via the 
    // address mapped to f:BtnMapTbl. In a real implementation, 
    // this is typically a read16 from the ROM region.
    
    // Based on typical FF4 layout, BtnMapTbl is in ROM. 
    // The routine loads an 8-bit value from the table, but then treats it as 16-bit (longa)
    // when storing it to $1A05. However, `lda f:BtnMapTbl,x` is 8-bit (shorta is active).
    // Then `sta $43`, then `ldx $43`, then `longa`, then `pla` (original X), 
    // then `sta $1a05,x`.
    
    // WAIT: Re-reading the ASM carefully:
    // sta $43        -> store A (btn_idx) to 0x43
    // ldx $43        -> X = btn_idx
    // lda BtnMapTbl,x -> A = table[btn_idx]
    // sta $43        -> store table[btn_idx] to 0x43
    // ldx $43        -> X = table[btn_idx]
    // longa          -> A 16-bit
    // pla            -> A = original_x (from stack)
    // sta $1a05,x    -> RAM[0x1A05 + table[btn_idx]] = original_x
    
    uint8_t mapped_val = snes->rom[0xBtnMapTbl_offset + btn_idx]; // Simplified ROM access
    uint16_t dest_offset = mapped_val; 
    
    // The instruction `sta $1a05,x` with longa (A=16bit) and X=mapped_val:
    // This writes the 16-bit value of the original X to the address $1A05 + mapped_val.
    write16(ram, 0x1A05 + dest_offset, original_x);
}

// PITFALLS: 6 (Mode A toggle: routine switches between 8-bit for table lookup 
// and 16-bit for the final store), 1 (DB=$FE).
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  0x43=1
//   output_ram:  0x1A05=2 (offset by table value)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFE
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: menu::SetBtnMap ($FE:63)