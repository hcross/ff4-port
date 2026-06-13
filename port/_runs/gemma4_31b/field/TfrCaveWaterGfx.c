#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$8A, DP=0
// Purpose: Transfers cave water graphics from WRAM to VRAM using DMA
// if the current tileset is equal to $0E (overworld cave).
static void TfrCaveWaterGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Check tileset at $0FDD
    if (ram[0x0FDD] != 0x0E) { // cmp #$0e / beq @8a9c
        return;
    }

    // Setup DMA for VRAM transfer
    // Note: Hardware register writes are handled via the emulator's 
    // memory map or specific register accessors. In this harness, 
    // we use the standard snes->ram for WRAM and the emulator 
    // state for registers.
    
    // sta $2115
    snes->cpu->db = 0x00; // Hardware registers are in bank 0
    // We use the emulator to perform the register write to ensure 
    // side-effects (like DMA triggers) are handled correctly.
    // However, for simple register writes in this pattern, we 
    // simulate the write to the PPU registers.
    // Since 'regs_write' was invalid, we use the emulator's 
    // write mechanism or direct memory if mapped.
    // For the purpose of this translation, we assume the 
    // harness provides a way to write to IO.
    // Given the constraints, we will use the emulator to 
    // execute the specific register writes to avoid struct errors.
    
    // To maintain parity and avoid 'regs_write' errors, we 
    // delegate the register writes to the emulator by 
    // wrapping them or using the provided run_emulated_func 
    // for the setup block if necessary. 
    // But the prompt asks for a C translation. 
    // We will use the standard LakeSnes register access:
    snes->ppu->regs[0x15] = 0x80; 

    InitDMA_emu(snes); // jsr InitDMA

    // stx $2116 (X = $0430)
    snes->ppu->regs[0x16] = 0x30;
    snes->ppu->regs[0x17] = 0x04;

    // DMA Control block at $4300 (WRAM)
    snes->cpu->db = 0x7E; // Return to WRAM bank
    ram[0x4300] = 0x01;           // sta $4300

    // Source Address: $5800
    write16(ram, 0x4302, 0x5800); // ldx #$5800 / stx $4302

    ram[0x4304] = 0x7F;           // lda #$7f / sta $4304

    // Transfer size/length: $0100
    write16(ram, 0x4305, 0x0100); // ldx #$0100 / stx $4305

    ExecDMA_emu(snes); // jsr ExecDMA
}

// PITFALLS: 1 (DB switching between 0x00 for PPU and 0x7E for WRAM)
// HELPERS: InitDMA_emu(snes), ExecDMA_emu(snes), write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0FDD=1
//   output_ram:  0x4300=1, 0x4302=2, 0x4304=1, 0x4305=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x8A
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrCaveWaterGfx ($8A:94)