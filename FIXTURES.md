# Reference savestates (fixtures)

Catalogue of LakeSnes savestates (`*.lss`) used as **reproducible entry
points** for the A/B oracle (`desktop/ff4-desktop-oracle`), `wram_diff`,
the spikes, and SDL inspection.

> ⚠ **Files not version-controlled (intentional).** The `.lss` files are gitignored: a
> savestate embeds WRAM + **VRAM + CGRAM (palettes) + ARAM** = fragments
> of protected Square Enix assets. They are not pushed to a public repo. This
> file documents their **provenance and role** (metadata only); the
> binaries stay **local** (absent from a fresh clone). Regeneration: see
> *Getting the fixtures* below.

## Catalogue

| Fixture | Scene / state | PC at load | Primary use | Related findings |
|---------|--------------|------------------|-----------------|---------------|
| `001-scene-after-leaving.lss` | Field, right after leaving a location | `00:3302` | Scene baseline (field rendering) | — |
| `002-combat.lss` | Combat | — | Combat baseline / ROM parity | F6, F8 |
| `003-world-map.lss` | Overworld (world map) | `00:3502` | Map field baseline | F7 |
| `004-menu.lss` | Field menu | `00:B102` | Menu baseline (IDENTICAL excluding `15cadc`/`048004`) | F4, F5 |
| `005-pre-combat.lss` | Right before combat entry | `00:9144` | Combat-entry crash repro + combat divergence | F9, F10, F12 |
| `006-in-combat.lss` | Mid-combat | `02:A353` | Combat bug repro (sprites/menu), btlgfx cluster bisection | F12, combat bug |
| `007-title-screen.lss` | Title screen | `00:9144` | Title / logo / input repro | F4, F5 |
| `008-overworld-mode7.lss` | Overworld ship → mode-7 transition | `00:9144` | **Mode-7** bug repro (corruption from ~frame 90) ; validation of the `InitMapRAM` fix (`ff4-gnw 1a86d23`) | mode-7 bug |
| `009-first-free-roam.lss` | Player's first **free-roam** instant (first control) | `00:9133` | Field control/movement baseline (captured in SDL 2026-06-30) | — |

## Conventions

- Incremental numbering `NNN-<descriptor>.lss`, capture order.
- A fixture must be **stable** (identically replayable) and target a scene
  or a specific instant useful for a given validation.
- Divergences observed by the oracle on a fixture are logged in
  `desktop/KNOWN_FINDINGS.md` and MemPalace (`wing=ff4-gnw`).

## Getting the fixtures

Since the binaries are not version-controlled, two paths:

1. **Manual capture** via SDL: `make -C desktop sdl ROM=...`, reach the
   desired scene, `Space` to pause if needed, **`5`** to save the incremental
   slot (`--save-prefix`), then rename per the convention.
2. **Boot-to-scene** *(tracked in [BACKLOG](../ff4/BACKLOG.md) — not implemented)*:
   a script that drives the emulator from boot to each target scene and
   regenerates the `.lss` locally from the user's ROM, without version-controlling
   any asset. This is the durable path to share the set without copyright exposure.

## Reproducibility note

Some fixtures expose a **delayed** bug: it only appears after N frames
of execution (e.g. `008-overworld-mode7`: FB divergence starting at ~frame 122).
Run the oracle with enough frames (`--frames 200`) to capture it.
