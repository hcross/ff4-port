#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// This routine initializes the jump tables for NMI and IRQ interrupts
// by writing JML (Jump Long) opcodes and the target addresses.
// JMP Long opcode is $5C.
static void InitInterrupts_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // JmpNMI is at $00:0000 (Standard vector table)
    // Assembler labels usually resolve to absolute addresses for these vectors
    uint16_t jmp_nmi_addr = 0x0000;
    uint16_t jmp_irq_addr = 0x0004;

    // Setup NMI Jump Table: JML <Address>
    ram[jmp_nmi_addr] = 0x5C;                               // sta JmpNMI
    write16(ram, jmp_nmi_addr + 1, 0x892C);                  // ldx #FieldNMI / stx JmpNMI+1
    ram[jmp_nmi_addr + 3] = 0x00;                           // stz JmpNMI+3 (High byte of address)

    // Setup IRQ Jump Table: JML <Address>
    ram[jmp_irq_addr] = 0x5C;                               // sta JmpIRQ
    write16(ram, jmp_irq_addr + 1, 0x8910);                  // ldx #FieldIRQ / stx JmpIRQ+1
    ram[jmp_irq_addr + 3] = 0x00;                           // stz JmpIRQ+3 (High byte of address)
}

// PITFALLS: None. (Simple memory initialization).
// HELPERS: read16/write16 — little-endian 16-bit accessors.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram: 0x0000=1, 0x0001=2, 0x0003=1, 0x0004=1, 0x0005=2, 0x0007=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::InitInterrupts ($89:ED)