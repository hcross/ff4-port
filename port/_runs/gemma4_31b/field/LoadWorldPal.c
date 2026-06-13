#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$91, DP=0
// Logic:
//   The routine copies 256 bytes from the memory address pointed to by 
//   the current A (acting as a pointer via absolute indexed addressing 
//   lda a:0000,x) into the palette buffer at $0CDB.
//   Note: 'pha / plb' sets the Data Bank (DB) to the value of the 
//   stack top. In the context of this routine, it ensures the source 
//   address is read from the bank specified by the caller.
static void LoadWorldPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // pha / plb: The bank is effectively switched to the value provided on stack
    // In our C model, this means the source address is in bank cpu->db (updated by caller)
    // and the destination $0CDB is in the current DB.
    
    uint16_t source_ptr = cpu->a; // lda a:$0000,x uses A as base when X=0
    uint16_t x = 0;               // ldY #0 / inx starts loop at 0
    
    for (uint16_t y = 0; y < 0x100; y++) {
        // lda a:$0000,x -> A = ram[DB : (A + X)]
        // This is a classic 65816 "indirect" style access where A is the base.
        uint8_t val = ram[(cpu->db << 16) | (source_ptr + x)]; 
        
        // sta $0cdb,y
        ram[0x0CDB + y] = val;
        
        x++; // inx
    }

    // a = 0 / pha / plb / rts
    // The routine ends by zeroing A and performing another bank switch (plb)
    // which likely resets the bank for the caller.
    cpu->a = 0;
}

// PITFALLS: 1 (Bank switching via pha/plb affects which ram segment is read)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=16, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0CDB=256
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x91
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadWorldPal ($91:B3)