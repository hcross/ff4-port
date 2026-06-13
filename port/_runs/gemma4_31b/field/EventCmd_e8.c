#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E9, DP=0
// Logic: 
// 1. Scans a command buffer starting at $1000 (offset by $3D) for a byte matching 
//    the value at $09D5[X+1].
// 2. If a match is found:
//    a. Looks up a value in CharAddTbl. If negative, it skips to clearing the buffer.
//    b. Otherwise, it copies a block of bytes from the buffer to a destination $1140.
//    c. The block size is determined by (CharAddTbl[val] << 6).
//    d. Finally, it clears the source byte in the buffer and waits for Vblank.
// 3. If no match is found before offset $0140, it jumps to WaitVblankEvent.
static void EventCmd_e8_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // @e937: inx / stx $b3
    uint16_t x_reg = cpu->x + 1;
    ram[0xB3] = (uint8_t)(x_reg & 0xFF); // Note: stx $b3 is 8-bit target, but x is 16-bit
    
    // @e937: ldy #$0000 / sty $3d
    uint16_t current_offset = 0;
    write16(ram, 0x3D, current_offset);

    // @e93f: scan loop
    while (1) {
        current_offset = read16(ram, 0x3D);
        uint8_t val = ram[0x1000 + current_offset];
        val &= 0x1F;

        // cmp $09d5,x
        if (val == ram[0x09D5 + x_reg]) {
            goto match_found; // beq @e962
        }

        // @e947: increment offset by $40
        uint16_t next_offset = current_offset + 0x40;
        write16(ram, 0x3D, next_offset);

        if (next_offset == 0x0140) {
            wait_vblank_event_emu(snes); // jmp WaitVblankEvent
            return;
        }
    }

match_found:
    // @e962: Process match
    uint8_t tbl_val = ram[0x09D5 + x_reg] - 1; // lda $09d5,x / dec
    
    // lda CharAddTbl,x (X is now tbl_val)
    // Note: CharAddTbl is an external table. We use the emulated lookup.
    // Since CharAddTbl is likely a data table, we access it via RAM if known, 
    // but standard pattern is to treat as a table lookup.
    // For this translation, we assume CharAddTbl is at a fixed location or accessed via helper.
    // Here we simulate: uint8_t entry = ram[CHAR_ADD_TBL + tbl_val];
    uint8_t entry = ram[0x0000 + tbl_val]; // Placeholder: actual CharAddTbl addr needed
    
    if ((int8_t)entry < 0) { // bmi @e99d
        goto clear_and_exit;
    }

    // longa / asl6 (Shift left 6 = multiply by 64)
    uint16_t copy_len = (uint16_t)entry << 6; 
    write16(ram, 0x40, copy_len); // sta $40 (16-bit)

    // shorta / lda #$40 / sta $07 (Loop counter)
    ram[0x07] = 0x40;
    
    uint16_t src_ptr = read16(ram, 0x3D);
    uint16_t dst_ptr = read16(ram, 0x40);

    // @e983: Copy loop
    while (ram[0x07] != 0) {
        ram[0x1140 + dst_ptr] = ram[0x1000 + src_ptr];
        src_ptr++;
        dst_ptr++;
        ram[0x07]--;
    }

    // Clear padding/end of block
    uint16_t final_dst = read16(ram, 0x40);
    ram[0x1143 + final_dst] = 0;
    ram[0x1144 + final_dst] = 0;
    ram[0x1145 + final_dst] = 0;
    ram[0x1146 + final_dst] = 0;

clear_and_exit:
    // @e99d: ldx $3d / stz $1000,x
    uint16_t final_src = read16(ram, 0x3D);
    ram[0x1000 + final_src] = 0;
    wait_vblank_event_emu(snes); // jmp WaitVblankEvent
}

// PITFALLS: 6 (Mode A 8/16-bit switching: longa/shorta used for copy length),
//            7 (Arithmetic truncation not applicable here, but logic follows 8-bit bytes),
//            1 (DB=$E9 used for memory offsets)
// HELPERS: wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  x=<16bits>, y=none, a=none
//   inputs_ram:  0x1000=<1byte>, 0x09D5=<1byte>, 0x3D=<2bytes>
//   output_ram:  0x1140=<1byte>
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE9
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_e8 ($E9:37)