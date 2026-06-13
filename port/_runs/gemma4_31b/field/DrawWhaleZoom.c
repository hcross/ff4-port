#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$BB, DP=0
// This routine updates coordinates for the Whale zoom effect.
// It reads from a source table (_15bc1f) and writes to a target buffer ($04e0).
// The loop runs 16 times, processing 4-byte blocks per iteration.
static void DrawWhaleZoom_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $0e / sec / sbc #$08 / sta $0e
    // Note: DB is $BB, so $0e refers to $BB:000E
    uint8_t val_0e = ram[0x0E];
    val_0e = (uint8_t)(val_0e - 0x08); 
    ram[0x0E] = val_0e;

    uint8_t val_0c = ram[0x0C];

    for (uint16_t x = 0; x < 0x10; x++) { // ldx #0 / loop / inx4 / cpx #$0010
        // The source table _15bc1f is typically in ROM or a fixed RAM location.
        // Based on the provided asm, it's accessed via absolute addressing.
        // Assuming _15bc1f is a pointer/label to data.
        // Since x increases by 4 (inx4), and the asm reads _15bc1f,x etc.,
        // the source index is x * 4.
        uint16_t src_off = 0x15BC1F + (x * 4); 
        
        // We must simulate the memory access. In this port, ROM is accessed via the snes instance.
        // For brevity in this translation, we use a conceptual read_rom helper or 
        // direct indexing if _15bc1f is mapped into the emulator's address space.
        
        uint8_t byte0 = snes->rom[src_off]; // lda _15bc1f,x
        uint8_t res0 = (uint8_t)(byte0 + val_0c); // clc / adc $0c

        if (res0 < byte0) { // bcs @bc14 (Branch if Carry Set)
            // In 65816, ADC sets C if result > 255. 
            // bcs triggers if (byte0 + val_0c) > 0xFF.
            // If bcs is taken, we skip to the next iteration.
        } else {
            // The logic in the ASM: bcs @bc14 means if C=1, skip.
            // C=1 when (byte0 + val_0c) > 0xFF.
            // So we enter this block only if result <= 0xFF.
            // WAIT: The ASM says "bcs @bc14", which means "If Carry, jump to end of loop body".
            // Therefore, the body executes ONLY IF Carry is clear (result <= 0xFF).
            
            // Since we are doing a simple sum, if result > 255, Carry is set.
            // This is a boundary check for the coordinate.
            if ((uint16_t)byte0 + val_0c <= 0xFF) {
                ram[0x04E0 + x] = res0; // sta $04e0,x (Note: x is the loop counter 0..15)
                
                uint8_t byte1 = snes->rom[src_off + 1]; // lda _15bc1f+1,x
                ram[0x04E1 + x] = (uint8_t)(byte1 + val_0e); // clc / adc $0e / sta $04e1,x
                
                uint8_t byte2 = snes->rom[src_off + 2]; // lda _15bc1f+2,x
                ram[0x04E2 + x] = byte2; // sta $04e2,x
                
                uint8_t byte3 = snes->rom[src_off + 3]; // lda _15bc1f+3,x
                ram[0x04E3 + x] = byte3; // sta $04e3,x
            }
        }
    }
}

// PITFALLS: 3 (CMP/BCS logic), 6 (A 8-bit mode), 7 (8-bit truncation on ADC)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0E=1, 0x0C=1, _15bc1f=64 (table)
//   output_ram:  0x04E0=16, 0x04E1=16, 0x04E2=16, 0x04E3=16, 0x0E=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBB
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DrawWhaleZoom ($BB:EA)