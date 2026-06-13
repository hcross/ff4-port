#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (Default), DP=0
// This routine initializes the overworld intro state by setting map IDs,
// zoom levels, movement speed, and clearing specific hardware/state registers.
static void LoadOverworldIntro_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1700] = 0;             // stz $1700 (overworld)
    ram[0x1704] = 0x04;          // lda #$04 / sta $1704 (enterprise)
    
    load_overworld_emu(snes);   // jsr LoadOverworld

    ram[0xAD] = 0x20;            // lda #$20 / sta $ad (mode 7 zoom)
    
    // UpdateZoomPal expects input in A
    snes->cpu->a = 0x10;         // lda #$10
    update_zoom_pal_emu(snes);  // jsl UpdateZoomPal
    
    ram[0xAC] = 0x02;            // lda #$02 / sta $ac (movement speed)
    ram[0x4200] = 0x81;          // lda #$81 / sta $4200 (enable nmi)
    ram[0x2100] = 0;             // stz $2100 (screen on, zero brightness)
    ram[0x80] = 0;              // stz $80
    ram[0x7B] = 0;              // stz $7b
    ram[0x7A] = 0;              // stz $7a
}

// PITFALLS: None significant. Routine is a linear sequence of stores and calls.
// HELPERS: load_overworld_emu(snes), update_zoom_pal_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x1700=1, 0x1704=1, 0xAD=1, 0xAC=1, 0x4200=1, 0x2100=1, 0x80=1, 0x7B=1, 0x7A=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadOverworldIntro ($00:D2)