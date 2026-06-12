// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// This routine reads a duration value from RAM, ensures it is not negative 
// (clamping to 0 if the sign bit is set), and writes the result to the 
// timer duration register.
static void SetTimerDur_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // ldy $ab (Y is 16-bit per battle module convention xf=0)
    uint16_t y = read16(ram, 0xAB);
    
    // bpl @9fd5: Branch if Plus. 
    // In 65816, BPL checks the N flag. For a 16-bit load, N is set if bit 15 is 1.
    if ((int16_t)y >= 0) { 
        // @9fd5: sty $d4
        write16(ram, 0xD4, y);
    } else {
        // clr_ay: tdc / tay (DP=0, so Y = 0)
        write16(ram, 0xD4, 0);
    }
}

// PITFALLS: 1 (DB=$7E), 8 (Inherited mode: Y is 16-bit/xf=0)
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xAB=2
//   output_ram:  0xD4=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::SetTimerDur ($9F:CF)