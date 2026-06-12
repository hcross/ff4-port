// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Checks if the timer for the currently selected object has expired
// and enables a pending action if so.
static void CheckTimer_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Set up object selection for SelectObj
    ram[0xD2] = ram[0xA9];
    select_obj_emu(snes);

    // Calculate timer index: (ram[0xAD] * 2) + ram[0xAD] = ram[0xAD] * 3
    uint8_t obj_id = ram[0xAD];
    ram[0xD3] = obj_id;
    uint8_t val = (uint8_t)(obj_id << 1); // asl (Pitfall 7)
    val = (uint8_t)(val + obj_id);       // clc / adc (Pitfall 7)
    ram[0xAF] = val;

    // Get pointer to timer data based on ram[0xAF]
    // GetTimerPtr expects input in A and typically returns via WRAM or register
    // based on the asm flow: lda $af / jsr GetTimerPtr / ldx $3598
    snes->cpu->a = ram[0xAF];
    get_timer_ptr_emu(snes);

    uint16_t ptr = read16(ram, 0x3598); // x = timer data offset
    
    // The ASM uses indexed addressing: lda $2a04,x
    // Since DP=0 and this is an absolute-indexed load, it accesses ram[0x2A04 + x]
    uint8_t byte0 = ram[0x2A04 + ptr];
    uint8_t byte1 = ram[0x2A05 + ptr];

    if ((byte0 | byte1) != 0) { // ora $2a05,x / bne @97b2
        return;
    }

    // Check the LSB of the third byte to determine if action is enabled
    uint8_t byte2 = ram[0x2A06 + ptr];
    if (byte2 & 0x01) {          // and #$01 / beq @97b2
        ram[0xD1]++;              // inc $d1
    }
}

// PITFALLS: 7 (8-bit arithmetic truncation for asl/adc)
// HELPERS: select_obj_emu(snes), get_timer_ptr_emu(snes), read16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00A9=1, 0x00AD=1, 0x2A04=1, 0x2A05=1, 0x2A06=1, 0x3598=2
//   output_ram:  0x00D1=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckTimer ($97:88)