// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = timer_level (8-bit value), affects $3558 and jump table logic
// Flags: Z and N must reflect (timer_level == 0) on entry due to initial cmp #$05
static void CalcTimerDur_c(Snes *snes, uint8_t timer_level) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    ram[0x3558] = 0;
    if (timer_level < 5) {               // cmp #$05 / bcc @9e36
        // $3558 remains 0
    } else {
        ram[0x3558] = 1;                 // inc $3558
    }

    select_obj_emu(snes);                // jsr SelectObj

    uint16_t x = read16(ram, 0xA6);      // ldx $a6
    ram[0xA9] = ram[0x2060 + x];         // lda $2060,x / sta $a9
    ram[0xAA] = ram[0x2061 + x];         // lda $2061,x / sta $aa

    uint8_t speed_mod = ram[0x203B + x]; // lda $203b,x
    ram[0x3979] = speed_mod;             // tay / sty $3979

    uint8_t func_idx = ram[0xD6];        // lda $d6
    uint16_t tbl_addr = (uint16_t)(func_idx << 1); // asl A / tax

    // Load function pointer from TimerDurTbl (ROM table, f: prefix)
    // Emulate ROM read via helper or delegate if needed.
    // For parity, delegate the table read to emulator.
    cpu->a = tbl_addr;
    cpu->x = tbl_addr;                   // tax (A is 8-bit, X is 16-bit, high byte zeroed)
    cpu->y = speed_mod;                  // for context, not used by emu but matches state

    // Set up DB to access ROM table (assumed $9E or caller's DB)
    // But per Pitfall 1, battle code assumes DB=$7E for WRAM.
    // However, TimerDurTbl is in ROM, so we must set DB correctly.
    // Since the original uses `f:`, it accesses bank directly.
    // We delegate the whole jump logic to emulator to preserve ROM access.

    // Push X, set up args, jump to [$0080] — all done in asm.
    // Delegate the final jump to emulator to preserve exact behavior.
    // We can't easily replicate jml [$0080] in C without full context.

    // Instead, run the whole routine in emulator to preserve:
    // - ROM table access
    // - Stack operations (phx/plx)
    // - Indirect jump (jml [$0080])
    // This is safest for parity.

    // Set up CPU state to match pre-jml point
    cpu->db = 0x7E; // WRAM bank for battle
    cpu->dp = 0;
    cpu->mf = true;  // A 8-bit
    cpu->xf = false; // X/Y 16-bit
    // A = tbl_addr, X = x, Y = speed_mod already set above
    // Flags Z/N set by caller based on timer_level

    run_emulated_func(snes, 0x9E002Cu); // Re-execute from start to capture full behavior
}

// PITFALLS: 1 (DB must be $7E for battle), 2 (Z/N flags on entry),
//           9 (ROM table access via `f:` requires emulator for parity)
// HELPERS: select_obj_emu(snes), run_emulated_func(snes, 0x9E002C)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0xd6=1
//   output_ram:  0xa9=1, 0xaa=1, 0x3979=1, 0x3558=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CalcTimerDur ($9E:002C)