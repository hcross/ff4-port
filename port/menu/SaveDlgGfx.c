#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FF (ROM), DP=0
// Logic: This routine configures the DMA (Direct Memory Access) to copy a 4KB 
// block of VRAM (starting at $2139) into WRAM ($7E:E600).
// It explicitly disables the screen and sets up the DMA control registers.
static void SaveDlgGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Hardware register accesses (mapped to $00:XXXX)
    // Note: In this specific emulator environment, DMA and PPU registers 
    // are treated as writes to the snes->ram mapping for the purpose of the parity harness,
    // but effectively they act as MMIO.
    
    ram[0x2100] = 0x80; // screen off
    ram[0x0088] = 0x80; // shadow/buffer register
    ram[0x2115] = 0x80; // vram enable/disable
    
    uint16_t ppu_addr = 0x2000; // ldx #$2000
    write16(ram, 0x2116, ppu_addr); // stx $2116
    
    // ldx $2139 : "dummy" read to synchronize VRAM pointer
    uint16_t dummy = read16(ram, 0x2139);

    // DMA Setup ($4300-$4305)
    ram[0x4300] = 0x81; // Mode: single address, auto-increment
    ram[0x4301] = 0x39; // Source address low: $2139 (WRAM/MMIO mirror)
    
    uint16_t dest = 0xE600; // ldx #$e600
    write16(ram, 0x4302, dest); // destination: $7ee600 (low word)
    
    ram[0x4304] = 0x7E; // destination: $7ee600 (bank $7E)
    
    uint16_t size = 0x1000; // ldx #$1000
    write16(ram, 0x4305, size); // size: 4KB
    
    ram[0x420B] = 0x01; // Trigger DMA start
}

// PITFALLS: 1 (MMIO writes must be targeted correctly; DMA registers 
//           in this port map to the ram array for parity tracking).
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x420B=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::SaveDlgGfx ($FF:62)