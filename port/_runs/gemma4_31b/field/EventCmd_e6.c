#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$EC, DP=0
// This routine implements Event Command 0xE6.
// It reads a byte, multiplies it by 100, and subtracts a 24-bit value (from $30-$32)
// from the result. If the subtraction underflows, the result is clamped to 0.
// The final result is stored in the coordinate/timer at $16A0-$16A2.
static void EventCmd_e6_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // jsr GetNextEventByte
    uint8_t event_byte = get_next_event_byte_emu(snes);
    
    // sta $18, stz $19 (Input A for Mult16)
    ram[0x18] = event_byte;
    ram[0x19] = 0;

    // lda #100, sta $1a, stz $1b (Input B for Mult16)
    ram[0x1A] = 100;
    ram[0x1B] = 0;

    // jsl Mult16
    mult16_emu(snes);

    // The result of Mult16 is in $16A0-$16A3 (typically 32-bit)
    // We perform 24-bit subtraction: [16A2:16A1:16A0] - [32:31:30]
    uint8_t a = ram[0x16A0];
    uint8_t b = ram[0x16A1];
    uint8_t c = ram[0x16A2];

    uint8_t s0 = ram[0x30];
    uint8_t s1 = ram[0x31];
    uint8_t s2 = ram[0x32];

    // sec / sbc $30
    uint8_t res0 = (uint8_t)(a - s0);
    bool carry = (a < s0); // Carry set if borrow occurs

    // sbc $31
    uint8_t res1 = (uint8_t)(b - s1 - carry);
    bool carry1 = (b < s1) || (b == s1 && carry); // Simplified borrow logic

    // sbc $32
    uint8_t res2 = (uint8_t)(c - s2 - carry1);
    bool carry2 = (c < s2) || (c == s2 && carry1);

    // bcs @ec3b: Branch if Carry is clear (C=0 means No Borrow)
    // Wait, in 65816 SBC, Carry is CLEAR if a borrow occurred.
    // So BCS branches if No Borrow occurred (A >= mem).
    if (!carry2) {
        ram[0x16A0] = res0;
        ram[0x16A1] = res1;
        ram[0x16A2] = res2;
    } else {
        // Underflow: clear the destination
        ram[0x16A0] = 0;
        ram[0x16A1] = 0;
        ram[0x16A2] = 0;
    }

    // jmp WaitVblankEvent
    wait_vblank_event_emu(snes);
}

// PITFALLS: 3 (SBC/BCS logic: in 65816, SBC clears carry on borrow. 
// BCS branches if carry=1, meaning NO borrow occurred. 
// If carry=0, a borrow occurred and we clamp to 0).
// HELPERS: get_next_event_byte_emu(snes), mult16_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x30=1, 0x31=1, 0x32=1
//   output_ram:  0x16A0=1, 0x16A1=1, 0x16A2=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEC
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_e6 ($EC:06)