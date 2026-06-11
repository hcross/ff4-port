# ca65-bridge

Backend bridge pour projets de disassembly **ca65 / 65816** (Super NES, Apple II,
NES via cc65, etc.). API compatible avec le protocol
[`re_agent.backend.protocol.REBackend`](https://github.com/Dryxio/auto-re-agent).

Construit dans le cadre du port FF4 → retro-go-sd Game & Watch. Conçu pour être
**réutilisable** par tout projet de réimplémentation C natif basé sur une
disassembly ca65 commentée.

## Statut

Phase 3 / scaffold initial. Implémente :

- `get_asm(label)` — extrait le body d'une routine asm par son label
- `xrefs_from(label)` — `jsr`/`jmp`/`bra` sortants d'une routine
- `xrefs_to(label)` — recherche regex sur l'ensemble des `.asm`
- `search(pattern)` — labels matchant un motif

Pas encore implémenté :

- Résolution adresse 24-bit ↔ label via `.map` / `.lst` (Phase 3.5)
- `decompile()` / `get_struct()` / `get_enum()` — non applicables (65816, pas de Ghidra)

## Usage

```bash
ca65-bridge get-asm CalcHits --root ../upstream
ca65-bridge xrefs-from CalcHits --root ../upstream
ca65-bridge search '^Calc' --root ../upstream
```

## Capabilities

| Capability     | Supporté |
|----------------|---------|
| has_decompile  | non — pas de pseudo-C |
| has_asm        | oui — asm 65816 commentée verbatim |
| has_xrefs      | oui — depuis sources |
| has_search     | oui |
| has_structs    | non |
| has_enums      | non |
