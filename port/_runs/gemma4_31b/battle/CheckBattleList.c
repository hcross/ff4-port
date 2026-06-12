// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// This routine checks a list of entities (defined by a pointer at $AB)
// to see if a specific entity (defined by a 16-bit word at $1800) 
// is present in that list.
//
// Logic:
//   Iterates through a list of 16-bit IDs.
//   The list is terminated by $FF (or $FFFF in 16-bit context).
//   If the current ID matches the target ID at $1800, increments counter at $A9.
static void CheckBattleList_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    ram[0xA9] = 0;                // stz $a9
    uint16_t y = 0;               // clr_ay (assuming DP=0)

    // The list pointer is stored at ram[$AB]
    uint16_t list_ptr = read16(ram, 0xAB);

    while (1) {
        // lda [$ab],y -> loads 8-bit value from (list_ptr + y)
        uint8_t val = ram[list_ptr + y];

        if (val == 0xFF) {        // cmp #$ff / beq @8802
            break;
        }

        // Check first byte against target ID low byte (at $1800)
        if (val != ram[0x1800]) { // cmp $1800 / bne @87fe
            y += 2;               // @87fe: iny / @87ff: iny
            continue;
        }

        // First byte matched, check second byte against target ID high byte (at $1801)
        y++;                      // iny
        uint8_t val_high = ram[list_ptr + y]; // lda [$ab],y
        if (val_high != ram[0x1801]) { // cmp $1801 / bne @87ff
            y++;                  // @87ff: iny
            continue;
        }

        // Full 16-bit ID match
        ram[0xA9]++;              // inc $a9
        return;                   // rts
    }
}

// PITFALLS: 6 (Mode A is 8-bit, so lda [$ab],y only loads one byte. 
// The routine manually handles the 16-bit comparison by checking two 
// consecutive bytes and incrementing Y twice).
// HELPERS: read16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xAB=2, 0x1800=1, 0x1801=1, [list_ptr]=1
//   output_ram: 0xA9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckBattleList ($87:E4)