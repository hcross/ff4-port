#include "snes/snes.h"

// This routine sets the ship's visibility status to visible and 
// initializes its coordinates to (222, 56) before waiting for a VBlank event.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$D3 (or $7E for WRAM), DP=0
static void Special_0d_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1728] = 0x01;               // lda #$01 / sta $1728
    
    // ldx #$38de / stx $1729 (X is 16-bit, little-endian store)
    write16(ram, 0x1729, 0x38DE);

    wait_vblank_event_emu(snes);      // jmp WaitVblankEvent (delegated)
}

// PITFALLS: 1 (WRAM access via absolute addresses)
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @ $E35B,
//          write16 — little-endian 16-bit accessor
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1728=1, 0x1729=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_0d ($D3:42)