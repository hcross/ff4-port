// battle::GetMonsterAttack — delegated wrapper
// ADR-003 delegate reasons: instr_count=312 > 50
static void GetMonsterAttack_emu(Snes *snes) {
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;
    c->xf = false;
    c->a = 0; c->x = 0; c->y = 0;
    c->z = true; c->n = false;
    run_emulated_func(snes, 0x03B3B6u);
}

// DELEGATED_FUNCTION: battle::GetMonsterAttack ($03:B3B6)
