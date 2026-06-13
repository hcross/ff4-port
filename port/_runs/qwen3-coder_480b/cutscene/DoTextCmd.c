#include "snes/snes.h"

// Processes a text command based on the accumulator value:
//   $01: newline (delegate to NewLine)
//   $0a: period "." (delegate to DrawLetterNoDakuten)
//   $02: tab (next byte is count of spaces to draw)
// Entry: A = command byte (8-bit), [$36] = text pointer
// Mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
static void DoTextCmd_c(Snes *snes, uint8_t cmd) {
    uint8_t *ram = snes->ram;

    if (cmd == 1) {                    // cmp #$01 / beq @eb21
        new_line_emu(snes);            // jmp NewLine
        return;
    }
    if (cmd == 0x0a) {                 // cmp #$0a / bne @eb10
        draw_letter_no_dakuten_emu(snes); // jmp DrawLetterNoDakuten
        return;
    }

    // Tab command: read count, draw that many spaces
    inc_text_ptr_emu(snes);            // jsr IncTextPtr
    uint16_t ptr = read16(ram, 0x36);
    uint8_t count = ram[ptr];          // lda [$36]
    ram[0] = count;                    // sta $00

    do {
        draw_letter_no_dakuten_emu(snes); // jsr DrawLetterNoDakuten (lda #$ff)
        ram[0]--;                      // dec $00
    } while (ram[0] != 0);             // bne @eb17
}

// PITFALLS: 1 (DB must be $7E for WRAM access), 6 (A is 8-bit),
// 8 (A 8-bit and X 16-bit inherited from caller)
// HELPERS: new_line_emu, draw_letter_no_dakuten_emu, inc_text_ptr_emu
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x36=2
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::DoTextCmd ($EB:05)