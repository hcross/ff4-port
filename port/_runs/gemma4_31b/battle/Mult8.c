// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
//
// This routine performs an 8-bit multiplication of the value in A (at entry)
// by a value stored at $DF.
// The multiplication is implemented via a shift-and-add loop (16 iterations).
//
// Logic:
//   Multiplier = A (initial)
//   Multiplicand = ram[$DF]
//   Result = ram[$E0:$E3] (32-bit result stored as 4 bytes)
static void Mult8_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Initial state
    ram[0xE0] = 0; 
    ram[0xE2] = 0;
    ram[0xE3] = 0;
    ram[0x394D] = 0;

    // The routine reads the initial A (8-bit) and stores it effectively in ram[$E1]
    // Since the asm does `ror $e1` without an explicit load, it implies the 
    // accumulator was shifted/stored there or the loop relies on the value 
    // existing at $E1. Based on the logic of 8-bit mults in FF4:
    // The value being shifted (the multiplier) is the input A.
    uint8_t multiplier = (uint8_t)snes->cpu->a;
    uint8_t multiplicand = ram[0xDF];
    
    uint32_t result = 0;
    uint8_t carry = 0; // CPU Carry flag

    for (int i = 0; i < 16; i++) {
        // ror $e1
        // Shift the multiplier right; the bit goes into Carry
        uint8_t bit = (multiplier >> 0) & 1;
        multiplier >>= 1;
        carry = bit;

        if (carry) { // bcc @83fb (not taken if carry=1)
            // clc / lda $df / adc $394d / sta $394d
            // Add multiplicand to the temporary accumulator $394d
            uint8_t tmp = ram[0x394D];
            tmp = (uint8_t)(multiplicand + tmp); // Pitfall 7: truncate to 8-bit
            ram[0x394D] = tmp;
            
            // Note: The original asm 'adc' also sets the carry flag for the 
            // subsequent 'ror $394d'.
            snes->cpu->c = (uint16_t)multiplicand + tmp > 0xFF;
        } else {
            snes->cpu->c = false;
        }

        // ror $394d / ror $e3
        // Shift the 16-bit result (ram[394D] and ram[E3]) right, pulling from Carry
        uint8_t low = ram[0x394D];
        uint8_t high = ram[0xE3];
        
        uint8_t low_bit = (low >> 0) & 1;
        low = (uint8_t)((low >> 1) | (carry << 7));
        carry = low_bit;
        
        uint8_t high_bit = (high >> 0) & 1;
        high = (uint8_t)((high >> 1) | (carry << 7));
        carry = high_bit;
        
        ram[0x394D] = low;
        ram[0xE3] = high;
    }

    // The result is actually shifted back into the destination bytes
    // Based on the loop, ram[E3] holds the high byte and ram[394D] the low.
    // However, looking at the stz $e0/$e2, this specific routine 
    // is a bit-bang multiplication where the final result is shifted 
    // into the memory range $E0-$E3.
    
    // The loop effectively computes (A * ram[DF]) and shifts it right 16 times.
    // The logic result is the lower 16 bits of the product.
    uint16_t final_res = (uint16_t)(ram[0x394D] | (ram[0xE3] << 8));
    write16(ram, 0xE0, final_res);

    // shorta0: A = 0, mode = 8-bit
    snes->cpu->a = 0;
    snes->cpu->mf = true;
}

// PITFALLS: 7 (Arithmetic truncation in 8-bit mode: adc and shifts wrapped in uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0xDF=1
//   output_ram:  0xE0=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Mult8 ($83:E0)