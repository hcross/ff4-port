#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00 (implicit for f: labels), DP=0
// This routine retrieves a single byte from one of three dialogue tables 
// based on a bank selector in RAM.
//
// Logic:
//   - index = ram[0x0772]
//   - bank = ram[0xDD]
//   - if bank == 0: return MapDlg[index]
//   - if bank == 1: return EventDlg1[index]
//   - else: return EventDlg2[index]
static uint8_t GetByte_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // ldx $0772 (X 16-bit)
    uint16_t index = read16(ram, 0x0772);
    
    // lda $dd (A 8-bit)
    uint8_t bank = ram[0xDD];
    
    if (bank != 0) { // bne @b2cc
        if (bank == 1) { // cmp #$01 / bne @b2d7
            // lda f:EventDlg1,x
            // Assuming Map/Event tables are in ROM/mapped memory
            // The harness provides access to the full address space via snes->ram 
            // or specific ROM offsets. Since f: denotes a fixed address:
            return snes->rom[0xEventDlg1 + index]; 
        } else {
            // lda f:EventDlg2,x
            return snes->rom[0xEventDlg2 + index];
        }
    }
    
    // lda f:MapDlg,x
    return snes->rom[0xMapDlg + index];
}

// Note: Since f:MapDlg etc. are labels, in a real implementation 
// they are replaced by the absolute ROM addresses defined in the disassembly.

// PITFALLS: 6 (Mode A 8-bit vs 16-bit: verified A is 8-bit, X is 16-bit)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0772=2, 0x00DD=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Returns value in A register, not RAM)

// REVERSED_FUNCTION: field::GetByte ($B2:BE)