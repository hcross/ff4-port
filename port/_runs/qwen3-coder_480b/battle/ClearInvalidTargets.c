// ClearInvalidTargets: processes a list of targets and clears bits in $ad
// for targets that are dead or stone.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Input:  Y = offset into $2003 table (16-bit index)
//         $a9 = target flags (bit 7 set if valid target)
//         $ab = number of targets to process
//         $ad = bitfield to modify
// Output: $ad = updated bitfield
static void ClearInvalidTargets_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    ram[0xAA] = 0; // target index

loop:
    // Check if this is a valid target (bit 7 of $a9 set)
    uint8_t target_flags = ram[0xA9];
    if ((target_flags & 0x80) == 0) { // asl $a9 / bcc (inverted)
        goto next;
    }

    // Check if target is dead or stone (bits 6-7 of $2003,y == 0)
    uint8_t status = ram[0x2003 + cpu->y];
    if ((status & 0xC0) == 0) { // beq @a764 (inverted)
        goto next;
    }

    // Clear bit in $ad for this target
    uint8_t index = ram[0xAA];
    uint8_t bitfield = ram[0xAD];
    cpu->a = bitfield;
    cpu->x = index;
    // Set flags for ClearBit entry (not needed since ClearBit doesn't check)
    bitfield = clear_bit_emu(snes); // jsr ClearBit
    ram[0xAD] = bitfield;

next:
    // Advance to next target (Y += $80)
    cpu->mf = false; // longa
    uint16_t y = cpu->y;
    y += 0x80;
    cpu->y = y;
    cpu->mf = true; // shorta0

    ram[0xAA]++; // inc $aa
    if (ram[0xAA] != ram[0xAB]) { // cmp $ab / bne loop (inverted)
        goto loop;
    }
}

// PITFALLS: 1 (DB=$7E required for correct memory access),
//           6 (A mode switching: longa/shorta0 affects memory ops),
//           7 (arithmetic truncation in 8-bit mode: not directly relevant here)
// HELPERS: clear_bit_emu(snes) — delegates ClearBit @ $03:855A
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16
//   inputs_ram:  0xa9=1, 0xab=1, 0xad=1, 0x2003=1
//   output_ram:  0xad=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::ClearInvalidTargets ($A7:4D)