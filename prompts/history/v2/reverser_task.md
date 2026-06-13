Translate the following ca65/65816 routine into idiomatic C, matching the
parity-validated pattern in `reverser_examples.md`.

**Target:** `${module}::${function_name}` at `$${bank}:${offset}`

**ASM source (verbatim from `ca65-bridge get-asm`):**
```asm
${asm_body}
```

**Calls from this routine (xrefs out — sub-routines you may need to delegate
via `*_emu(snes)` helpers if they have not been translated yet):**
${xrefs_out}

**Calls into this routine (xrefs in — context on call sites, for inferring
the expected register/flag state at entry):**
${xrefs_in}

**Macros to be aware of (from `include/macros.inc`):**
- `clr_a`    : `tdc`              (A = D register, 16-bit transfer)
- `clr_ax`   : `tdc / tax`        (A = X = D)
- `clr_ay`   : `tdc / tay`        (A = Y = D)
- `longa`    : `.a16 / rep #$20`  (A → 16-bit mode)
- `shorta`   : `.a8 / sep #$20`   (A → 8-bit mode)
- `shorta0`  : `clr_a / shorta`   (A = D then 8-bit)
- `longi`    : `rep #$10`         (X/Y → 16-bit)
- `shorti`   : `sep #$10`         (X/Y → 8-bit)
- `iny2`     : `iny / iny`        (Y += 2, useful for word-iteration)

**Constraints:**
1. Match every branch and call EXACTLY. Parity = byte-equality.
2. For each sub-routine not yet translated, use a `<name>_emu(snes)` helper.
3. State your assumptions about mode A/X (8 vs 16-bit) at entry in comments.
4. If a routine starts with a conditional branch (`bne`/`beq`/`bpl`/`bmi`/
   `bcc`/`bcs`/`bvc`/`bvs`), the flag must be set by the caller to reflect
   the input — document this in the C function's contract.

Output as specified in `reverser_system.md`.
