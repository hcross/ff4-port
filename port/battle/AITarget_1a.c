// This function sets A=1 and jumps to TargetMonsterTypeAll.
// It acts as a wrapper to target all monsters of type 1.
static void AITarget_1a_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    cpu->a = 1;                            // lda #1
    cpu->n = (cpu->a & 0x80) != 0;         // update N flag
    cpu->z = (cpu->a == 0);                // update Z flag
    targetmonstertypeall_emu(snes);        // jmp TargetMonsterTypeAll
}

// PITFALLS: 2 (flags Z/N must be set to reflect A register value)
// HELPERS: targetmonstertypeall_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AITarget_1a ($B9:64)