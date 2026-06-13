#include "snes/snes.h"

// Trampoline: sets ram[$91] = 0x39, loads A = 0xC0, then tail-jumps to _d9e5.
// The original caller's return address is still on the stack; _d9e5 will RTS
// directly back to that caller.
static void Special_06_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t *ram = snes->ram;

    // Field module conventions: DB=$7E, DP=0, A=8-bit, X/Y=16-bit
    cpu->db = 0x7E;
    cpu->dp = 0;
    cpu->mf = true;
    cpu->xf = false;

    // lda #$39 / sta $91
    ram[0x91] = 0x39;

    // lda #$c0
    cpu->a = 0xC0;
    // Flags from the load: N=1 (bit7 set), Z=0
    cpu->n = true;
    cpu->z = false;

    // jmp _d9e5 (tail call — no return address pushed)
    run_emulated_func(snes, 0x0D9E5);
}

// PITFALLS:
//   1 (DB=$7E required for direct-page store to $91)
//   2 (Z/N flags set to reflect loaded value before tail call, in case
//      _d9e5 starts with a conditional branch)
//   6 (mode A assumed 8-bit; immediate loads are single-byte)
// HELPERS:
//   none — uses run_emulated_func directly for the tail jump
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes  (trampoline; full effect includes _d9e5 chain)
// REVERSED_FUNCTION: field::Special_06 ($D9:D6)