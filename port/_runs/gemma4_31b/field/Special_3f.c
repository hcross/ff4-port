#include "snes/snes.h"

// This routine iterates through the party characters.
// If a character is Paladin Cecil (ID 0x0B), it sets HP to 1 and clears status 1.
// All other characters are marked as dead (status 1 = 0x80) and HP set to 0.
// It continues until X reaches 0x0140.
static void Special_3f_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = 0;

    do {
        // The ASM uses indexed addressing $1000,x. 
        // In 65816, the index X is added to the base address.
        // The base $1000 is likely relative to the current Data Bank (DB).
        // We assume DB = $7E for party data in this module.
        uint16_t base = 0x1000 + x;

        uint8_t char_id = ram[base] & 0x1F; // lda $1000,x / and #$1f
        if (char_id == 0x0B) {              // cmp #$0b / bne @c4e3
            ram[base + 3] = 0;              // stz $1003,x (Clear Status 1)
            uint8_t hp_val = 0x01;          // lda #$01
            ram[base + 7] = hp_val;         // sta $1007,x
        } else {
            ram[base + 3] = 0x80;           // sta $1003,x (Mark Dead)
            ram[base + 7] = 0x00;           // lda #$00 / sta $1007,x
        }

        ram[base + 8] = 0;                  // stz $1008,x
        ram[base + 4] = 0;                  // stz $1004,x (Clear Status 2)

        next_char_emu(snes);                // jsr NextChar
        
        // The NextChar_emu must update the CPU's X register.
        // We synchronize our local x with the emulator's X.
        x = snes->cpu->x;

    } while (x != 0x0140);                  // cpx #$0140 / bne @c4d2

    wait_vblank_event_emu(snes);            // jmp WaitVblankEvent
}

// PITFALLS: 1 (DB=$7E assumed for party RAM), 6 (A is 8-bit, X is 16-bit),
//           8 (Inherited mode: mf=true, xf=false)
// HELPERS: next_char_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Loop iterates over party array; multiple RAM writes)
// REVERSED_FUNCTION: field::Special_3f ($C4:CF)