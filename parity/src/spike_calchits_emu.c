// Spike M1 — valider RunEmulatedFunc en pure-external (sans modif core LakeSnes)
// sur la routine CalcHits @ $03:C987 du ROM FF4 JP 1.1.
//
// Objectif :
//   - Charger ROM, boot suffisant pour stabiliser le CPU
//   - Préparer la WRAM manuellement (RNG table en $1900, index $97, params $38fa/$38fb)
//   - Positionner le PC sur CalcHits, exécuter l'asm jusqu'au RTS
//   - Observer la valeur produite en $38fd et la confronter à l'oracle calculé
//     en Python via la même règle (count of N first RNG entries < hit_rate)
//
// Si ça matche → l'archi RunEmulatedFunc tient debout sur LakeSnes upstream
// sans patch du core, et on peut engager le mouvement 2 (traduction C de
// CalcHits + comparaison asm vs C sur 1000 inputs).
//
// Usage:
//   ./ff4-spike-calchits <rom.sfc>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "snes.h"
#include "cpu.h"

// CalcHits adresse 24-bit, mapping LoROM bank 0x03 segment battle_code.
// Trouvée via: map segment battle_code start=0x038000 + lst offset 0x4987.
#define CALCHITS_ADDR_24 0x03C987u

// Adresses RAM clés (direct page, accessible en absolute aussi)
#define RAM_RNG_INDEX  0x0097     // ram[0x97] : index dans la rng table
#define RAM_RNG_TABLE  0x1900     // ram[0x1900..0x19FF] : RNG table copiée du ROM
#define RAM_HIT_RATE   0x38FA     // input
#define RAM_BASE_HITS  0x38FB     // input
#define RAM_NHITS_OUT  0x38FD     // output

// ---------------------------------------------------------------------------
// I/O
// ---------------------------------------------------------------------------

static uint8_t *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc(sz);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, sz, f) != (size_t)sz) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *out_len = sz;
    return buf;
}

// ---------------------------------------------------------------------------
// RunEmulatedFunc — pure external, pas de modif core LakeSnes.
//
// On pousse 2 octets bidon sur la stack pour qu'à la fin du RTS de la routine
// cible, le CPU "saute" à une adresse arbitraire. On surveille le SP : tant
// qu'il est < sp_save, on est dans la fonction (ou un sous-appel). Quand il
// remonte au niveau initial, le RTS a été exécuté → on arrête.
// ---------------------------------------------------------------------------

static inline uint32_t stack_addr(const Cpu *cpu) {
    // En mode emulation (e=1), SP est forcé en page 1 : adresse = $0100 | (sp & 0xFF).
    // En mode natif (e=0), SP est un vrai 16-bit pointant n'importe où en bank 0.
    return cpu->e ? (0x0100u | (cpu->sp & 0xFFu)) : cpu->sp;
}

static void run_emulated_func(Snes *snes, uint32_t pc24) {
    Cpu *cpu = snes->cpu;
    uint16_t sp_save = cpu->sp;

    // Push d'une adresse de retour "magique". On ne la laissera pas exécuter
    // (on stop avant). Convention 65816 : sur JSR, le CPU push PC_hi à [SP],
    // décrémente SP, push PC_lo à [SP], décrémente SP.
    snes->ram[stack_addr(cpu)] = 0x12;  // PC_hi
    cpu->sp--;
    snes->ram[stack_addr(cpu)] = 0x34;  // PC_lo
    cpu->sp--;

    // Setup PC
    cpu->k = (uint8_t)(pc24 >> 16);
    cpu->pc = (uint16_t)(pc24 & 0xFFFF);

    // Boucle : on tourne jusqu'à ce que le RTS final restaure SP au sp_save.
    // Garde-fou : on borne à 100k opcodes pour ne pas spinner si quelque chose
    // dérape (ex. la routine boucle infinie sur un état de RAM mal initialisé).
    long max_ops = 100000;
    while (cpu->sp != sp_save && max_ops-- > 0) {
        cpu_runOpcode(cpu);
        if (cpu->waiting || cpu->stopped) break;
    }
    if (max_ops <= 0) {
        fprintf(stderr, "run_emulated_func: budget opcodes épuisé — la routine "
                "n'a pas retourné. SP=%04X sp_save=%04X PC=%02X:%04X\n",
                cpu->sp, sp_save, cpu->k, cpu->pc);
    }
}

// ---------------------------------------------------------------------------
// Oracle CalcHits — formule décodée du source asm :
//   CalcHits boucle base_hits fois et appelle Rand99 à chaque tour.
//   Rand99 = RandXA(min=0, max=98). RandXA fait :
//       result = (rng_table[ram[0x97]] % (max - min + 1)) + min
//   Donc Rand99 retourne rng_table[ram[0x97]] % 99, valeur dans [0, 98].
//   Puis CalcHits compare à hit_rate ($38fa) : si Rand99 < hit_rate, count++
//
// Note : ram[0x97] N'EST PAS incrémenté par CalcHits ni Rand99 dans ce
// chemin. Donc chaque tour de boucle consulte LA MÊME valeur rng_table[idx].
// L'incrément de ram[0x97] est fait par l'appelant à un autre moment.
// (À vérifier en mouvement 2 ; pour l'instant on s'aligne sur ce comportement
// observé.)
// ---------------------------------------------------------------------------

static int oracle_calchits(const uint8_t *rng_table, uint8_t rng_index,
                           uint8_t base_hits, uint8_t hit_rate) {
    int count = 0;
    uint8_t rand_value = rng_table[rng_index] % 99;  // Rand99 toujours même valeur ici
    for (int i = 0; i < base_hits; i++) {
        if (rand_value < hit_rate) count++;
    }
    return count;
}

// ---------------------------------------------------------------------------
// Test driver
// ---------------------------------------------------------------------------

static int run_trial(Snes *snes, const uint8_t *rng_table,
                     uint8_t rng_index, uint8_t base_hits, uint8_t hit_rate) {
    // Snapshot des registres CPU avant l'appel (pour pouvoir relancer)
    Cpu *cpu = snes->cpu;
    uint16_t saved_a = cpu->a, saved_x = cpu->x, saved_y = cpu->y;
    uint16_t saved_sp = cpu->sp, saved_pc = cpu->pc, saved_dp = cpu->dp;
    uint8_t saved_k = cpu->k, saved_db = cpu->db;
    bool saved_mf = cpu->mf, saved_xf = cpu->xf;

    // Setup mémoire :
    // - RNG table en $1900
    memcpy(snes->ram + RAM_RNG_TABLE, rng_table, 256);
    // - index courant en $97 (direct page)
    snes->ram[RAM_RNG_INDEX] = rng_index;
    // - hit rate / base hits / clear output
    snes->ram[RAM_HIT_RATE] = hit_rate;
    snes->ram[RAM_BASE_HITS] = base_hits;
    snes->ram[RAM_NHITS_OUT] = 0xCC;  // sentinelle (CalcHits doit l'écraser)

    // CPU state pour battle :
    // - DP = 0 (direct page non utilisée par CalcHits, mais Rand99/RandXA
    //   font de la DP arithmetique sur $96/$97/$3947/...). DP=0 est cohérent
    //   avec la convention battle d'après la lecture de damage.asm et le fait
    //   que toutes ces adresses sont accédées comme `<adr>` absolute < $100.
    // - DB = $7E (data bank = WRAM). C'est CRUCIAL : sans ça, `STZ $38fd`
    //   écrit dans le hardware register $00:38FD, pas dans la WRAM.
    cpu->dp = 0;
    cpu->db = 0x7E;
    cpu->mf = true;   // 8-bit A
    cpu->xf = true;   // 8-bit X/Y

    run_emulated_func(snes, CALCHITS_ADDR_24);

    int observed = snes->ram[RAM_NHITS_OUT];
    int expected = oracle_calchits(rng_table, rng_index, base_hits, hit_rate);

    bool ok = (observed == expected);
    printf("  rng_idx=%3u base=%3u rate=%3u  observed=%3d  expected=%3d  %s\n",
           rng_index, base_hits, hit_rate, observed, expected, ok ? "OK" : "FAIL");

    // Restaure le CPU pour un trial propre suivant (le ROM n'a pas bougé,
    // mais on a poussé/poppé sur la stack — qu'on rétablisse explicitement).
    cpu->a = saved_a; cpu->x = saved_x; cpu->y = saved_y;
    cpu->sp = saved_sp; cpu->pc = saved_pc; cpu->dp = saved_dp;
    cpu->k = saved_k; cpu->db = saved_db;
    cpu->mf = saved_mf; cpu->xf = saved_xf;

    return ok ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <rom.sfc>\n", argv[0]);
        return 1;
    }

    size_t rom_len = 0;
    uint8_t *rom = read_file(argv[1], &rom_len);
    if (!rom) return 2;

    Snes *snes = snes_init();
    if (!snes_loadRom(snes, rom, rom_len)) {
        fprintf(stderr, "snes_loadRom failed\n");
        return 3;
    }
    snes_reset(snes, true);

    // Laisser tourner quelques frames pour que le boot init stabilise le CPU.
    // (Sinon les registres SP/PC sont dans un état très tôt — utilisable
    // mais bizarre.)
    for (int i = 0; i < 60; i++) snes_runFrame(snes);

    // Désactiver les IRQ/NMI pendant les trials pour éviter qu'une interruption
    // survienne au milieu d'un run_emulated_func.
    snes->cpu->i = true;  // I flag SET = IRQ masqué
    snes->cpu->nmiWanted = false;
    snes->cpu->irqWanted = false;

    // Table RNG synthétique pour les trials.
    uint8_t rng_table[256];
    for (int i = 0; i < 256; i++) rng_table[i] = (uint8_t)i;  // 0..255

    // Trials : couvre rate=0/50/99, base=0 (return early), wrap idx>255,
    // table avec valeurs nulles, table avec valeurs proches du max Rand99.
    int total = 0, fails = 0;

    #define TRIAL(idx, base, rate) do { fails += run_trial(snes, rng_table, idx, base, rate); total++; } while(0)
    TRIAL(0,  10, 99);
    TRIAL(0,  10, 50);
    TRIAL(0,  10, 0);
    TRIAL(50, 20, 99);
    TRIAL(0,  0,  99);    // base=0 → CalcHits doit return sans rien faire (mais sentinelle écrasée par STZ)
    TRIAL(200,100,99);    // wrap idx>=256 (idx & 0xFF = 200)
    TRIAL(0,  1,  98);    // 1 tirage, valeur table[0]=0 < 98 → 1 hit
    TRIAL(98, 1,  98);    // table[98]=98 % 99 = 98 ; 98 < 98 FALSE → 0 hit
    TRIAL(99, 1,  98);    // table[99]=99 % 99 = 0 ; 0 < 98 → 1 hit
    #undef TRIAL

    printf("\n=== summary ===\nfails: %d/%d\n", fails, total);
    free(rom);
    return fails == 0 ? 0 : 1;
}
