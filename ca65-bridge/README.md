# ca65-bridge

Backend bridge for **ca65 / 65816** disassembly projects (Super NES,
Apple II, NES via cc65, etc.). API compatible with the
[`re_agent.backend.protocol.REBackend`](https://github.com/Dryxio/auto-re-agent)
protocol.

Built as part of the FF4 → retro-go-sd Game & Watch port. Designed to be
**reusable** for any native C reimplementation project based on a
commented ca65 disassembly.

## Status

Phase 3 / initial scaffold. Implements:

- `get_asm(label)` — extracts an asm routine body by its label
- `xrefs_from(label)` — `jsr`/`jmp`/`bra` outgoing from a routine
- `xrefs_to(label)` — regex search across all `.asm` sources
- `search(pattern)` — labels matching a pattern
- `classify_routine(label)` — ADR-003 classification (translate vs delegate)

Not yet implemented:

- 24-bit address ↔ label resolution via `.map` / `.lst` (Phase 3.5)
- `decompile()` / `get_struct()` / `get_enum()` — not applicable (65816, no Ghidra)

## Usage

```bash
ca65-bridge get-asm CalcHits --root ../upstream
ca65-bridge xrefs-from CalcHits --root ../upstream
ca65-bridge search '^Calc' --root ../upstream
ca65-bridge classify CalcDmg --root ../upstream
ca65-bridge classify-module battle --root ../upstream
```

## Capabilities

| Capability     | Supported |
|----------------|-----------|
| has_decompile  | no — no pseudo-C |
| has_asm        | yes — verbatim 65816 asm with comments |
| has_xrefs      | yes — parsed from sources |
| has_search     | yes |
| has_structs    | no |
| has_enums      | no |
