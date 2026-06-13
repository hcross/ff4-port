#include "snes/snes.h"

// LoadBGGfx: Loads background graphics for overworld/map scenes.
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Entry: ram[$0FDD] = map index
// Logic:
//   if (index == 0 || index == 0x0F) → 4bpp path (loads from MapGfx_0000)
//   else → 3bpp path (uses MapGfxPtrs and MapGfxBankTbl)
static void LoadBGGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t index = ram[0x0FDD];         // lda $0fdd
    if (index == 0 || index == 0x0F) {   // beq @b091 / cmp #$0f / bne @b094
        goto bpp4_path;
    }

    // 3bpp path
    clear_bg_gfx_emu(snes);              // jsl ClearBGGfx
    ram[0x420B] = 0;                     // stz $420b
    uint16_t x = (uint16_t)(index << 1); // asl A / tax
    uint16_t ptr = read16(ram, 0x8000 + x); // lda f:MapGfxPtrs,x (assumed fixed bank)
    ram[0x4302] = ptr & 0xFF;
    ram[0x4303] = (ptr >> 8) & 0xFF;
    ram[0x4304] = ram[0x8200 + index];   // lda f:MapGfxBankTbl,x (assumed fixed bank)
    tfr_bg_gfx_emu(snes);                // jsl TfrBGGfx
    return;

bpp4_path:; // 4bpp path
    write16(ram, 0x47, 0);               // ldx #0 / stx $47
    write16(ram, 0x45, 0x2400);          // ldx #$2400 / stx $45
    ram[0x3C] = 0x88;                    // lda #.bankbyte(MapGfx_0000) / sta $3c
    write16(ram, 0x3D, 0x8000);          // lda f:MapGfxPtrs / sta $3d (low)
                                          // lda f:MapGfxPtrs+1 / sta $3e (high)
    ram[0x2115] = 0x80;                  // lda #$80 / sta $2115
    ram[0x420B] = 0;                     // stz hMDMAEN
    ram[0x4300] = 0x01;                  // lda #$01 / sta $4300
    ram[0x4301] = 0x18;                  // lda #$18 / sta $4301
    ram[0x4304] = ram[0x3C];             // lda $3c / sta $4304
    write16(ram, 0x2116, read16(ram, 0x47)); // ldx $47 / stx $2116
    write16(ram, 0x4302, read16(ram, 0x3D)); // ldx $3d / stx $4302
    write16(ram, 0x4305, read16(ram, 0x45)); // ldx $45 / stx $4305
    ram[0x420B] = 1;                     // lda #1 / sta hMDMAEN
}

// PITFALLS: 1 (DB=$7E assumed), 8 (A 8-bit, X/Y 16-bit inherited)
// HELPERS: clear_bg_gfx_emu(snes), tfr_bg_gfx_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=16
//   inputs_ram:  0x0FDD=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadBGGfx ($B0:88)