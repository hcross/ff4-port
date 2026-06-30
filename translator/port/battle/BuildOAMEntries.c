#include "snes/snes.h"
#include "snes/cart.h"

/* Standalone spike extraction of BuildOAMEntries_c (= asm DrawStatusSprites
 * @ $02:DCED), bundled in ff4-gnw/battle/btlgfx_monsters.c. Includes its
 * sub-call BackAttackYOffset_s_c (= BackAttackFlipX $02:BB0B) so the harness
 * can run the full body. inject_cycles stubbed no-op (region spike compares
 * post-state, not cycles); snes_runCycles kept (real LakeSnes fn, harmless). */

#define LOROM(bank, addr) (((uint32_t)(bank) << 15) | ((addr) & 0x7FFF))
static inline void inject_cycles(Snes *snes, int n) { (void)snes; (void)n; }

/* sub-call: BackAttackFlipX $02:BB0B (copied from btlgfx_prim.c) */
void BackAttackYOffset_s_c(Snes *snes) {
    uint8_t a = (uint8_t)(snes->cpu->a);
    if (snes->ram[0x6CC0]) {
        snes_runCycles(snes, 170);
        uint8_t na = (uint8_t)(~a);
        uint8_t res = (uint8_t)(na - 8);
        snes->cpu->a  = (snes->cpu->a & 0xFF00) | res;
        snes->cpu->c  = (na >= 8);
        snes->cpu->v  = (((na ^ 8) & (na ^ res)) & 0x80) != 0;
        snes->cpu->n  = (res & 0x80) != 0;
        snes->cpu->z  = (res == 0);
    } else {
        snes_runCycles(snes, 130);
        snes->cpu->n = (a & 0x80) != 0;
        snes->cpu->z = (a == 0);
    }
}

void BuildOAMEntries_c(Snes *snes) {
    uint8_t slot = (uint8_t)(snes->cpu->x);
    uint16_t y_row = (uint16_t)((uint16_t)slot << 2);
    uint8_t tile_count = snes->cart->rom[LOROM(0x16, 0xFD15) + slot];
    uint16_t loop_limit = (uint16_t)tile_count << 2;
    snes->ram[0x0E] = (uint8_t)(loop_limit & 0xFF);
    snes->ram[0x0F] = (uint8_t)(loop_limit >> 8);
    uint8_t variant = snes->ram[0xF078];
    snes->ram[0xF08F + slot] = variant;
    uint8_t tile_base_scaled = (uint8_t)((uint8_t)variant << 4);

    if (!snes->ram[0xF07B + y_row]) {
        inject_cycles(snes, 600);
        return;
    }

    uint8_t ctr_lo = snes->ram[0xF07C + y_row] + 1;
    snes->ram[0xF07C + y_row] = ctr_lo;
    bool hi_tick = ((ctr_lo & 0x07) == 0);
    if (hi_tick) {
        snes->ram[0xF07D + y_row]++;
    }

    uint16_t slot_w = (uint16_t)((uint16_t)slot << 1);
    uint8_t x_base = snes->ram[0x6CF3 + slot_w];
    snes->ram[0x12] = x_base;
    uint8_t y_base = snes->ram[0x6CF4 + slot_w];
    snes->ram[0x13] = y_base;

    uint8_t frame_off = (uint8_t)(((snes->ram[0xF07D + y_row] & 0x01) << 3) + tile_base_scaled);
    uint16_t rom_x = frame_off;
    uint16_t oam_y = loop_limit;
    snes->ram[0x0E] = 0;

    bool do_flip = (snes->ram[0x6CC0] != 0);
    bool variant9 = (variant == 9);

    uint8_t loop_ctr = 0;
    while (loop_ctr < 2) {
        snes->cpu->a = (snes->cpu->a & 0xFF00) |
                       (uint8_t)(snes->cart->rom[LOROM(0x13, 0xFAE1) + rom_x] + x_base);
        BackAttackYOffset_s_c(snes);
        snes->ram[0x0300 + oam_y] = (uint8_t)(snes->cpu->a);
        rom_x++; oam_y++;

        snes->ram[0x0300 + oam_y] =
            (uint8_t)(snes->cart->rom[LOROM(0x13, 0xFAE1) + rom_x] + y_base);
        rom_x++; oam_y++;

        uint8_t tile_byte;
        if (variant9) {
            tile_byte = snes->ram[0xF079 + loop_ctr];
        } else {
            tile_byte = snes->cart->rom[LOROM(0x13, 0xFAE1) + rom_x];
        }
        snes->ram[0x0300 + oam_y] = tile_byte;
        rom_x++; oam_y++;

        uint8_t attr = snes->cart->rom[LOROM(0x13, 0xFAE1) + rom_x];
        if (do_flip) {
            attr ^= 0x40;
        }
        snes->ram[0x0300 + oam_y] = attr;
        rom_x++; oam_y++;

        loop_ctr++;
        snes->ram[0x0E] = loop_ctr;
    }

    inject_cycles(snes, 0);
}

// SPIKE_COMPARE: region
// SPIKE_MASK: 0x0E-0x10
// CONTRACT:
//   inputs_reg:  x=8
//   inputs_ram:  0xF078=1, 0x6CC0=1, 0xF07B=1, 0xF07C=1, 0xF07D=1, 0x6CF3=1, 0x6CF4=1, 0xF079=1
//   output_ram:  0x0300=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: btlgfx::DrawStatusSprites ($02:DCED)
