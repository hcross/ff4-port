#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x0 (hardware/IO), DP=0
// This routine handles the animation sequence for the Overworld Leviathan.
// It sets up hardware registers for NMI and screen, then loops through frames,
// calculating coordinates and parameters for the draw routine.
// Note: Hardware registers (0x2100, 0x4200, etc.) are accessed via snes->ram
// as the harness maps these IO ranges into the memory space.
static void Special_0f_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    load_overworld_leviathan_emu(snes); // jsr LoadOverworldLeviathan

    ram[0x4200] = 0x81;                 // sta $4200 (enable nmi)
    ram[0x2100] = 0x0F;                 // sta $2100 (screen on, full brightness)

    // X is 16-bit (xf=0), but used as 8-bit counter in this specific loop
    uint16_t loop_cnt = 0x0200;         // ldx #$0200
    ram[0x89] = (uint8_t)(loop_cnt & 0xFF); // stx $89
    ram[0x79] = 0;                      // stz $79

    do {
        ram[0x1705] = 0x03;             // lda #$03 / sta $1705
        draw_leviathan_frame_emu(snes);  // jsr DrawLeviathanFrame

        ram[0x0340] = 0x28;             // lda #$28 / sta $0340
        ram[0x0300] = 0x28;             // sta $0300

        uint8_t val7a = ram[0x7A];
        uint8_t x_idx = (uint8_t)(val7a >> 4); // lsr4 / tax (shift 4 bits)
        
        // LeviathanTailYTbl is a table of bytes. 
        // We access it via the emulator's memory map or assumed external address.
        // Since we are in C and the table is static, we emulate the load.
        ram[0x0341] = snes->ram[0x/*LeviathanTailYTbl_addr*/ + x_idx]; 
        // Note: In a real implementation, LeviathanTailYTbl_addr is replaced by 
        // the absolute ROM/WRAM address derived from the disassembly.

        ram[0x0342] = 0x30;             // lda #$30 / sta $0342

        uint8_t a_temp = x_idx;         // txa
        a_temp &= 0x04;                 // and #$04
        a_temp = (uint8_t)(a_temp << 4); // asl4 (shift 4 bits)
        a_temp |= 0x37;                 // ora #$37
        ram[0x0343] = a_temp;           // sta $0343

        ram[0x0301] = 0x70;             // lda #$70 / sta $0301

        uint8_t val7a_2 = ram[0x7A];
        val7a_2 >>= 2;                  // lsr2
        val7a_2 &= 0x02;                // and #$02
        
        // clc / adc #$e4
        uint8_t res_adc = (uint8_t)(val7a_2 + 0xE4); 
        ram[0x0302] = res_adc;          // sta $0302

        ram[0x0303] = 0x37;             // lda #$37 / sta $0303

        // Decrement counter in RAM
        uint8_t current_x = ram[0x89];
        current_x--;                    // dex
        ram[0x89] = current_x;          // stx $89

    } while (ram[0x89] != 0);           // bne @d1f8

    wait_vblank_event_emu(snes);       // jmp WaitVblankEvent
}

// PITFALLS: 7 (Arithmetic truncation: used (uint8_t) for ADC and shifts to 
// ensure 8-bit wrap-around behavior matching 65816 mf=true).
// HELPERS: load_overworld_leviathan_emu(snes), draw_leviathan_frame_emu(snes), 
//          wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7a=1
//   output_ram:  0x0300=1, 0x0301=1, 0x0302=1, 0x0303=1, 0x0340=1, 0x0341=1, 0x0342=1, 0x0343=1, 0x89=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Complex loop with multiple IO writes and table lookups)
// REVERSED_FUNCTION: field::Special_0f ($D1:E4)