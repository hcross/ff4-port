// Entry mode: mf=true, xf=false, dp=0, db=0x7E (battle convention)
// Logic: 16-bit unsigned division. 
// Dividend: ram[0x3945] (16-bit), Divisor: ram[0x3947] (16-bit)
// Result: Quotient in ram[0x3949] (16-bit), Remainder in ram[0x394B] (16-bit)
// Implementation: Shift-and-subtract (restoring division) algorithm.
static void Div16_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // longa: All A operations and memory accesses at 0x394x are 16-bit
    write16(ram, 0x3949, 0); // stz $3949
    write16(ram, 0x394B, 0); // stz $394b

    uint16_t dividend = read16(ram, 0x3945);
    uint16_t divisor = read16(ram, 0x3947);

    if (dividend == 0 || divisor == 0) { // beq @843f (checks both)
        return;
    }

    uint16_t quotient = 0;
    uint16_t remainder = 0;
    uint16_t current_dividend = dividend;

    // ldx #$0010 loop
    for (int i = 16; i > 0; i--) {
        // rol $3945 / rol $394b
        // This shifts the dividend into the remainder.
        // We simulate the 16-bit ROL chain:
        uint16_t carry = (current_dividend & 0x8000) ? 1 : 0;
        current_dividend = (uint16_t)((current_dividend << 1) | 0); // Carry from ROL $3945 is discarded here, but we need the bit for $394b
        
        // The assembly does: ROL $3945 (sets C), then ROL $394b (uses C, sets C)
        // Since $394b starts at 0, it's effectively shifting the high bit of dividend into remainder.
        uint16_t rem_carry = (remainder & 0x8000) ? 1 : 0;
        remainder = (uint16_t)((remainder << 1) | carry);
        
        // sec / lda $394b / sbc $3947 / sta $394b
        // Subtract divisor from the current remainder
        uint16_t temp_rem = remainder - divisor;
        
        if (remainder >= divisor) { // bcs @8439 (SBC didn't borrow)
            remainder = temp_rem;
        } else {
            // lda $394b / adc $3947 / sta $394b (Restore remainder)
            // The 'adc' restores the value because the borrow was set.
            // remainder remains unchanged.
        }

        // rol $3949 (Shift result bit into quotient)
        // If the subtraction was successful (remainder >= divisor), the carry is 1.
        uint16_t q_carry = (remainder >= divisor) ? 0 : 0; // This logic is slightly different in ASM
        // Let's re-evaluate the ROL $3949 logic based on 65816 flags:
        // If BCS @8439 was taken, the Carry flag was SET by the SBC result (no borrow).
        // If the block was entered, the Carry flag was CLEARED by the ADC.
        // Wait, the ASM is: sbc $3947 -> bcs (C=1 if no borrow) -> rol $3949.
        // If subtraction is successful, C=1, so quotient gets a 1.
    }
}