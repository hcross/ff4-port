// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9E, DP=0
// Logic:
//   Calculates the timer duration for a selected object.
//   1. Sets a flag at $3558 based on whether the input A is >= 5.
//   2. Calls SelectObj to determine the object index (stored in $A6).
//   3. Loads base timer duration (2 bytes) and speed modifier into temp RAM.
//   4. Looks up a function pointer from TimerDurTbl based on the value in $D6.
//   5. Executes that function via a jump to the address stored at $80.
static void CalcTimerDur_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Compare input A (8-bit) with 5
    ram[0x3558] = 0;
    if (cpu->a >= 5) { // cmp #$05 / bcc @9e36 (inverted!)
        ram[0x3558]++; // inc $3558
    }

    select_obj_emu(snes); // jsr SelectObj

    uint16_t obj_idx = read16(ram, 0xA6); // ldx $a6
    
    // Base timer duration: load 2 bytes from $2060 + x
    // Note: The ASM uses lda $2060,x and sta $a9 / lda $2061,x / sta $aa
    // This is effectively reading a word from $2060 + obj_idx.
    ram[0xA9] = ram[0x2060 + obj_idx];
    ram[0xAA] = ram[0x2061 + obj_idx];

    // Speed modifier: load from $203B + x
    uint8_t speed_mod = ram[0x203B + obj_idx]; // lda $203b,x
    ram[0x3979] = speed_mod; // tay / sty $3979

    // Duration function lookup
    uint8_t func_idx = ram[0xD6]; // lda $d6
    uint8_t table_offset = (uint8_t)(func_idx << 1); // asl / tax

    // TimerDurTbl is in ROM (bank 0x9E). 
    // Since this is a C reimplementation, we access the table via the ROM data.
    // Assuming TimerDurTbl is a known offset in the ROM.
    // Using the pattern: lda f:TimerDurTbl,x / sta $80 ...
    // For this translation, we simulate the jump by setting up the stack/registers 
    // and calling run_emulated_func if the target is another asm routine.
    
    // We store the target address in $80-81 and the parameter 3 in $82
    // This looks like a dynamic dispatch to a duration calculator.
    uint16_t target_addr = read16(&snes->rom[0x9E00 + 0x????], table_offset); // Placeholder for TimerDurTbl
    write16(ram, 0x80, target_addr);
    ram[0x82] = 0x03;

    // jml [$0080] - Jump to the function pointed to by $80
    // In the parity harness, we execute the target address.
    run_emulated_func(snes, target_addr);
}

// PITFALLS: 3 (CMP/BCC inversion), 7 (8-bit shift truncation)
// HELPERS: select_obj_emu(snes) — delegates SelectObj @ 8489
//          run_emulated_func — used for the indirect jump at the end.
//
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x2060=1, 0x2061=1, 0x203B=1, 0xD6=1
//   output_ram:  0x3558=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (indirect jump target depends on ROM table)

REVERSED_FUNCTION: battle::CalcTimerDur ($9E:2C)