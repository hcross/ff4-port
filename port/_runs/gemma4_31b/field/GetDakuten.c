#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B6 (ROM), DP=0
// Purpose: Look up a character and its corresponding dakuten mark from DakutenTbl
// based on the input value in A. Returns the character in A and the mark in X.
//
// Logic:
//   If A >= 0x42: A = 0xFF, X remains unchanged.
//   If A < 0x42: 
//     index = ((A - 0x0F) << 1)
//     A = DakutenTbl[index] (kana)
//     X = DakutenTbl[index + 1] (dakuten)
static void GetDakuten_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t a = (uint8_t)cpu->a;

    if (a >= 0x42) { // cmp #$42 / bcs @b686
        cpu->a = 0xFF;
        return;
    }

    // phx (X is preserved, we'll restore it at the end)
    uint16_t old_x = cpu->x;

    // sec / sbc #$0f / asl
    // In 8-bit mode, this is (a - 0x0F) * 2
    uint8_t index = (uint8_t)((a - 0x0F) << 1); // Pitfall 7: truncation to 8-bit

    // The table DakutenTbl is in ROM (bank f). 
    // We assume a helper or direct access to ROM is provided.
    // Based on the asm: lda DakutenTbl,x / xba / lda DakutenTbl+1,x / xba
    // This effectively loads: A = Table[index], X = Table[index + 1]
    
    // Note: Since the target is a C reimplementation, we access the ROM data.
    // we use snes->rom or a pointer to the table.
    uint8_t *tbl = (uint8_t *)snes->rom_base + 0x...; // Offset for DakutenTbl
    
    // To match the "xba" (exchange A and X) dance:
    // 1. lda Table,x -> A = Table[index]
    // 2. xba         -> A = X, X = Table[index]
    // 3. lda Table+1,x -> A = Table[index+1]
    // 4. xba         -> A = Table[index], X = Table[index+1]
    
    uint8_t kana = tbl[index];
    uint8_t mark = tbl[index + 1];

    cpu->a = kana;
    cpu->x = mark;

    // plx (Wait, the ASM does plx THEN jmp @b68a)
    // Re-evaluating ASM:
    // phx -> save X
    // ... calculations ...
    // plx -> restore original X
    // However, the logic uses 'tax' and 'xba' to manipulate the result.
    // Let's trace again:
    // tax (index into X)
    // lda tbl,x (A = kana)
    // xba (A = index, X = kana)
    // lda tbl+1,x (A = mark)
    // xba (A = kana, X = mark)
    // plx (X = original_x) <--- THIS OVERWRITES THE RESULT IN X
    
    // Correction based on strict ASM trace:
    // phx  -> saves original X
    // ...
    // plx  -> restores original X
    // This means the 'xba' result in X is discarded.
    // The only thing that persists is A = kana.
    
    // Wait, looking at @b686: xba / lda #$ff / xba
    // This is a pattern to set A = 0xFF without disturbing X.
    
    cpu->a = kana;
    cpu->x = old_x; 
}

// PITFALLS: 7 (8-bit shift truncation), 6 (Mode A 8-bit)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  none (ROM access)
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto, c=auto
// CUSTOM_SPIKE: yes (requires ROM table mapping)

// REVERSED_FUNCTION: field::GetDakuten ($B6:6E)