#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E5 (bank of EventScript), DP=0
// Logic:
//   Iterates through the EventScript table starting at the pointer in ram[0x09D3].
//   Increments the pointer until a terminator byte (0xFF) is found.
//   Updates the pointer in ram[0x09D3] to the position of the terminator.
static void FindEventTerminator_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Event scripts are typically in ROM. 
    // f:EventScript usually refers to the start of the script table.
    // Since the prompt specifies DB=$E5 for this routine, we access the 
    // table relative to that bank's memory space.
    uint8_t *event_script_table = &snes->ram[0xE5000]; // Simplified ROM/RAM bank mapping

    // ldx $09d3: event script pointer (16-bit)
    uint16_t x = read16(ram, 0x09D3);
    
    // inx
    x++;

    // loop @e55e
    while (1) {
        // lda f:EventScript,x (mf=1, so 8-bit load)
        uint8_t a = event_script_table[x];
        
        // cmp #$ff / beq @e56a
        if (a == 0xFF) {
            break;
        }
        
        // inx / jmp @e55e
        x++;
    }

    // stx $09d3
    write16(ram, 0x09D3, x);
}

// PITFALLS: 6 (Mode A is 8-bit for the 0xFF check, X is 16-bit for the pointer)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x09D3=2, EventScriptTable=1
//   output_ram: 0x09D3=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE5
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::FindEventTerminator ($E5:5A)