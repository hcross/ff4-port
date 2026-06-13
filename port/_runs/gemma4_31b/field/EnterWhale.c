#include "snes/snes.h"

// Logic:
//   1. Set sound parameters in RAM at $1E00-$1E01
//   2. Execute sound effect via ExecSound_ext
//   3. Load script ID $54 and execute via ExecTriggerScript
static void EnterWhale_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x1E01] = 0x0E;         // lda #$0e / sta $1e01
    ram[0x1E00] = 0x01;         // lda #$01 / sta $1e00
    
    ExecSound_ext_emu(snes);    // jsl ExecSound_ext
    
    snes->cpu->a = 0x54;         // lda #$54
    ExecTriggerScript_emu(snes); // jsr ExecTriggerScript
}

// PITFALLS: 1 (DB=$A2 context; writes to $1E00 treated as ram access)
// HELPERS: ExecSound_ext_emu(snes), ExecTriggerScript_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x1E00=1, 0x1E01=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xA2
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EnterWhale ($A2:EB)