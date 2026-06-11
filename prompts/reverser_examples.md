# Reference examples — asm 65816 → C translations validated by parity

Two examples that PASSED parity 1000/1000 trials. Use as few-shot reference
for new translations.

---

## Example 1 — CalcHits @ $03:C987 (battle/damage.asm)

### ASM source

```asm
CalcHits:
@c987:  stz     $38fd       ; clear number of hits
        lda     $38fb
        beq     @c99e       ; return if no base hits
        tay
@c990:  jsr     Rand99
        cmp     $38fa       ; check vs. hit rate
        bcs     @c99b
        inc     $38fd       ; increment number of hits
@c99b:  dey
        bne     @c990
@c99e:  rts
```

### C translation

```c
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM (no register I/O — convention battle):
//   in : ram[$38FB] base_hits, ram[$38FA] hit_rate
//   out: ram[$38FD] = number of hits
static void CalcHits_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x38FD] = 0;
    uint8_t base = ram[0x38FB];
    if (base == 0) return;               // beq @c99e
    uint8_t rate = ram[0x38FA];
    for (uint8_t y = base; y > 0; y--) { // tay / dey / bne loop
        uint8_t r = rand99_emu(snes);    // jsr Rand99 (delegated)
        if (r < rate) {                  // cmp $38fa / bcs (inverted!)
            ram[0x38FD]++;               // inc $38fd
        }
    }
}

// PITFALLS: 3 (CMP/BCS inversion: bcs branches when A>=mem, so we
// enter the body when A<mem)
// HELPERS: rand99_emu(snes) — delegates Rand99 @ $03:858B
REVERSED_FUNCTION: battle::CalcHits ($03:C987)
```

---

## Example 2 — ApplyDmgMult @ $03:CA41 (battle/damage.asm)

### ASM source

```asm
ApplyDmgMult:
@ca41:  bne     @ca48
        stz     $a4
        stz     $a5
        rts
@ca48:  lsr
        bne     @ca50
        lsr     $a5
        ror     $a4
        rts
@ca50:  tax
        stx     $393d
        ldx     $a4
        stx     $393f
        jsr     Mult16
        ldx     $3941
        stx     $a4
        rts
```

### C translation

```c
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = elemental_multiplier (8-bit), $a4-$a5 = damage 16-bit LE
// CALLER MUST set Z and N flags to reflect the input value BEFORE jsr,
// because the routine starts with `bne` (consults Z).
//   See pitfall 2 in system prompt.
//
// Logic:
//   if mult == 0     : damage = 0
//   if mult == 1     : damage >>= 1
//   else (mult > 1)  : damage = (mult >> 1) * damage  (truncated to 16-bit)
static void ApplyDmgMult_c(Snes *snes, uint8_t mult) {
    uint8_t *ram = snes->ram;

    if (mult == 0) {                     // bne @ca48 → not taken
        ram[0xA4] = 0;
        ram[0xA5] = 0;
        return;
    }
    uint8_t shifted = mult >> 1;         // lsr A
    if (shifted == 0) {                  // bne @ca50 → not taken (mult was 1)
        uint16_t dmg = read16(ram, 0xA4);
        dmg >>= 1;                       // lsr $a5 / ror $a4
        write16(ram, 0xA4, dmg);
        return;
    }
    // mult > 1 : Mult16(shifted, damage), truncate to 16-bit
    write16(ram, 0x393D, (uint16_t)shifted);  // stx $393d (X 16-bit, X_hi=0)
    uint16_t dmg = read16(ram, 0xA4);
    write16(ram, 0x393F, dmg);                 // stx $393f
    mult16_emu(snes);                          // jsr Mult16 (delegated)
    write16(ram, 0xA4, read16(ram, 0x3941));   // damage = result lo
}

// PITFALLS: 1 (DB=$7E required), 2 (Z/N flags simulated by caller — but
// from the FUNCTION SIDE this is a documented contract, not a fix)
// HELPERS: mult16_emu(snes) — delegates Mult16 @ $03:83B9
//          read16/write16 — little-endian 16-bit accessors over ram[]
REVERSED_FUNCTION: battle::ApplyDmgMult ($03:CA41)
```

---

## Implicit shared helpers (assumed available in the C harness)

```c
static inline uint16_t read16(const uint8_t *ram, int addr) {
    return (uint16_t)(ram[addr] | (ram[addr + 1] << 8));
}
static inline void write16(uint8_t *ram, int addr, uint16_t v) {
    ram[addr] = v & 0xFF;
    ram[addr + 1] = (v >> 8) & 0xFF;
}
```

These accessors mirror the 65816 little-endian convention for 16-bit
absolute accesses in mode A 16-bit. The same pattern applies to 24-bit
(byte/word/byte triples) if needed — extend with `read24`/`write24` only
when a routine requires it.
