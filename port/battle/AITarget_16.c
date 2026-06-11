// AITarget_16: Effectue (count+1) rotations à droite à travers le carry sur un octet cible.
// Charge count depuis ram[0x361C], l'offset d'adresse depuis ram[0xA6:0xA7], et effectue
// une rotation à droite de l'octet à ram[0x2053 + offset] à travers le carry, avec carry=1 initial.
//
// Entry mode: A 8-bit (mf=true), X/Y 16-bit (xf=false), DB=$7E, DP=0
static void AITarget_16_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    uint8_t count = ram[0x361C];          // lda $361c (A 8-bit)
    uint16_t y = (uint16_t)count;         // tay (extension zéro vers Y 16-bit)
    uint16_t x = read16(ram, 0xA6);       // ldx $a6 (chargement X 16-bit, little-endian)
    
    uint8_t carry = 1;                    // sec
    
    // Boucle: ror $2053,x / dey / bpl
    // S'exécute (count+1) fois, effectuant des rotations à droite à travers carry
    do {
        uint16_t addr = 0x2053 + x;
        uint8_t val = ram[addr];
        uint8_t new_carry = val & 0x01;   // bit 0 → carry
        val = (val >> 1) | (carry << 7);  // rotation droite à travers carry
        ram[addr] = val;
        carry = new_carry;
        
        y--;                              // dey (décrémentation 16-bit)
    } while ((int16_t)y >= 0);            // bpl: branche si Y >= 0 (signé 16-bit)
}

// PITFALLS: 1 (DB=$7E requis pour l'adressage absolu $2053), 
//           8 (mode hérité: mf=true, xf=false de la convention du module battle)
// HELPERS: read16 — accesseur 16-bit little-endian
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x361C=1, 0xA6=2
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
// CUSTOM_SPIKE: yes (adresse de sortie calculée dynamiquement: ram[0x2053 + read16(ram, 0xA6)])
REVERSED_FUNCTION: battle::AITarget_16 ($B9:0A)