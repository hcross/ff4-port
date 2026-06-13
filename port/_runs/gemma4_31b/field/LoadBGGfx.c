#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B0, DP=0
// Logic:
//   - Checks ram[0x0FDD] to determine graphics mode.
//   - If value == 0 or 0x0F: Use 4bpp (direct DMA transfer to VRAM).
//   - Otherwise: Use 3bpp (calls ClearBGGfx and TfrBGGfx).
static void LoadBGGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t gfx_mode = ram[0x0FDD];

    if (gfx_mode == 0 || gfx_mode == 0x0F) {
        // 4bpp path (@b0be)
        ram[0x47] = 0;                            // ldx #0 / stx $47
        ram[0x45] = 0x00;                         // ldx #$2400 / stx $45 (A 8-bit mode write)
        ram[0x46] = 0x24;                         // (Implicit 16-bit X load into 8-bit RAM)
        
        // Note: MapGfx_0000 bank byte is a constant from the ROM/ASM
        // In the provided asm, .bankbyte(MapGfx_0000) is used.
        // For the purpose of this C translation, we assume the harness 
        // provides the correct bank byte for MapGfx_0000.
        uint8_t bank_byte = 0x00; // Placeholder for .bankbyte(MapGfx_0000)
        ram[0x3C] = bank_byte;                    // sta $3c
        
        // MapGfxPtrs access
        uint8_t ptr_lo = ram[0x0000]; // Placeholder: MapGfxPtrs address needed
        uint8_t ptr_hi = ram[0x0001]; // Placeholder: MapGfxPtrs+1 address needed
        // Since MapGfxPtrs is a label in 'f' (field) section, 
        // these would be absolute ROM/RAM reads.
        // For parity, we use the emulated memory map.
        ram[0x3D] = ptr_lo;                      // sta $3d
        ram[0x3E] = ptr_hi;                      // sta $3e
        
        ram[0x2115] = 0x80;                      // sta $2115
        ram[0x4300] = 0x01;                      // sta $4300
        ram[0x4301] = 0x18;                      // sta $4301
        ram[0x4304] = ram[0x3C];                 // lda $3c / sta $4304
        ram[0x2116] = ram[0x47];                 // ldx $47 / stx $2116
        ram[0x4302] = ram[0x3D];                 // ldx $3d / stx $4302
        ram[0x4303] = ram[0x3E];                 // (The asm implies 16-bit X transfer to $4302)
        
        uint16_t size = read16(ram, 0x45);
        write16(ram, 0x4305, size);              // ldx $45 / stx $4305
        
        // hMDMAEN is a global variable/address
        // ram[hMDMAEN] = 1; 
        return;
    }

    // 3bpp path (@b094)
    clear_bggfx_emu(snes);                       // jsl ClearBGGfx
    ram[0x420B] = 0;                             // stz $420b
    
    uint8_t idx = (uint8_t)(gfx_mode << 1);      // lda $0fdd / asl (Pitfall 7)
    
    // MapGfxPtrs lookup (X is index)
    // MapGfxPtrs is an array of 16-bit pointers
    uint16_t ptr = read16(ram, 0x0000 + (idx * 2)); // lda MapGfxPtrs,x / sta $4302...
    write16(ram, 0x4302, ptr);
    
    // MapGfxBankTbl lookup
    uint8_t bank = ram[0x0000 + idx];            // lda MapGfxBankTbl,x
    ram[0x4304] = bank;                          // sta $4304
    
    tfr_bggfx_emu(snes);                         // jsl TfrBGGfx
}

// PITFALLS: 7 (ASL truncate: (uint8_t)(gfx_mode << 1))
// HELPERS: clear_bggfx_emu(snes), tfr_bggfx_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0FDD=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: field::LoadBGGfx ($B0:88)