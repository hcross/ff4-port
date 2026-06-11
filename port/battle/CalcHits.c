// battle::CalcHits — manual translation matching the M2 spike, used here
// as a worked example to validate the auto-spike generator.
//
// Inputs in WRAM:
//   $38FA hit_rate
//   $38FB base_hits
//   $1900..$19FF rng_table
//   $97 rng_index
// Output in WRAM:
//   $38FD nhits

static uint8_t rand99_emu(Snes *snes);

static void CalcHits_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x38FD] = 0;
    uint8_t base = ram[0x38FB];
    if (base == 0) return;
    uint8_t rate = ram[0x38FA];
    for (uint8_t y = base; y > 0; y--) {
        uint8_t r = rand99_emu(snes);
        if (r < rate) {
            ram[0x38FD]++;
        }
    }
}

// Local rand99 helper for the spike — delegates to the asm at $03:858B.
static uint8_t rand99_emu(Snes *snes) {
    Cpu *c = snes->cpu;
    uint16_t saved_a=c->a, saved_x=c->x, saved_y=c->y, saved_sp=c->sp;
    uint16_t saved_pc=c->pc, saved_dp=c->dp;
    uint8_t saved_k=c->k, saved_db=c->db;
    bool saved_mf=c->mf, saved_xf=c->xf;
    c->dp = 0; c->db = 0x7E;
    c->mf = true; c->xf = true;
    // run_emulated_func is provided by the parity spike harness.
    run_emulated_func(snes, 0x03858Bu);
    uint8_t r = (uint8_t)(c->a & 0xFF);
    c->a=saved_a; c->x=saved_x; c->y=saved_y;
    c->sp=saved_sp; c->pc=saved_pc; c->dp=saved_dp;
    c->k=saved_k; c->db=saved_db;
    c->mf=saved_mf; c->xf=saved_xf;
    return r;
}

// PITFALLS: 3 (CMP/BCS inversion)
// HELPERS: rand99_emu (delegates Rand99 @ $03:858B)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x38FA=1, 0x38FB=1
//   output_ram:  0x38FD=1
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::CalcHits ($03:C987)
