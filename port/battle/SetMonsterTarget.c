// battle::SetMonsterTarget — delegated wrapper
// ADR-003 delegate reasons: instr_count=84 > 50
static void SetMonsterTarget_emu(Snes *snes) {
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;
    c->xf = false;
    c->a = 0; c->x = 0; c->y = 0;
    c->z = true; c->n = false;
    run_emulated_func(snes, 0x03B68Fu);
}

// DELEGATED_FUNCTION: battle::SetMonsterTarget ($03:B68F)
