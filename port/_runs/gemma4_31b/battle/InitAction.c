// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Initializes the current action state for a combatant, determines if 
//          the action is a character choice, monster choice, attack, or timer effect.
//   Inputs: ram[$D2] acting entity, ram[$D3] timer/state index
//   Outputs: ram[$352E] action type, ram[$D1] = 0, ram[$38F6] updated priority
static void InitAction_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Handle acting entity and priority
    ram[0x38F6] = ram[0xD2];
    select_obj_emu(snes); // jsr SelectObj
    
    uint8_t priority = (uint8_t)(ram[0x38F6] + 1); // inc $38f6
    if (priority == 0x0D) {                        // cmp #$0d / bne @97c8
        priority = 0;                               // stz $38f6
    }
    ram[0x38F6] = priority;

    // Calculate timer index: (D3 * 2) + D3 = D3 * 3
    uint8_t d3 = ram[0xD3];
    uint8_t idx = (uint8_t)((d3 << 1) + d3);       // asl / clc / adc $d3
    ram[0xA9] = idx;

    get_timer_ptr_emu(snes);                       // jsr GetTimerPtr (returns ptr in $3598)
    uint16_t ptr = read16(ram, 0x3598);             // ldx $3598
    
    // Access table at $2A06 + ptr
    uint8_t status = ram[0x2A06 + ptr];             // lda $2a06,x
    status &= 0x7E;                                 // and #$7e

    uint8_t action_type;
    if (status != 0) {                              // bne @97ed
        if ((status & 0x08) == 0) {                 // and #$08 / beq @97f5
            action_type = 0x02;                     // 3: do attack
        } else {
            action_type = 0x03;                     // 4: do timer effect
        }
    } else {
        if (ram[0xD2] >= 0x05) {                    // cmp #$05 / bcc @97e9
            action_type = 0x01;                     // 1: choose monster action
        } else {
            action_type = 0x00;                     // 2: choose character action
        }
    }

    ram[0x352E] = action_type;                     // sta $352e
    ram[0xD1] = 0;                                  // stz $d1
}

// PITFALLS: 7 (Arithmetic truncation: (d3 << 1) + d3 cast to uint8_t to match 8-bit A)
// HELPERS: select_obj_emu(snes), get_timer_ptr_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00D2=1, 0x00D3=1, 0x2A06=1
//   output_ram: 0x352E=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::InitAction ($97:B3)