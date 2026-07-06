# Reference savestates (fixtures)

Catalogue of LakeSnes savestates (`*.lss`) used as **reproducible entry
points** for the A/B oracle (`desktop/ff4-desktop-oracle`), `wram_diff`,
the spikes, and SDL inspection.

> ⚠ **Private submodule, not this (public) repo.** The binaries live in
> [`fixtures/`](fixtures/), a git submodule pointing at a **private,
> self-hosted** repo (`hcross/ff4-fixtures` on the author's own LAN Gitea
> instance) — never GitHub, never any public host. A savestate embeds WRAM
> + **VRAM + CGRAM (palettes) + ARAM** = fragments of protected Square Enix
> assets, which is exactly why they can't live in this public repo. Before
> 2026-07-05 they were local-only and gitignored (single copy, no backup,
> no way to regenerate if lost — tracked as MemPalace `wing=ff4-gnw
> room=blind-spots` point 6); moving them to a private submodule gives
> them real version control and a backup without changing the
> copyright constraint. **A clone of this (public) repo will have an empty
> `fixtures/` directory** unless you also have network access to that LAN
> Gitea instance — see *Getting the fixtures* below. This file documents
> **provenance and role** (metadata only); the binaries themselves are not
> duplicated here (one-writer rule).

## Catalogue

| Fixture | Scene / state | PC at load | Primary use | Related findings |
|---------|--------------|------------------|-----------------|---------------|
| `fixtures/001-scene-after-leaving.lss` | Field, right after leaving a location | `00:3302` | Scene baseline (field rendering) | — |
| `fixtures/002-combat.lss` | Combat | — | Combat baseline / ROM parity | F6, F8 |
| `fixtures/003-world-map.lss` | Overworld (world map) | `00:3502` | Map field baseline | F7 |
| `fixtures/004-menu.lss` | Field menu | `00:B102` | Menu baseline (IDENTICAL excluding `15cadc`/`048004`) | F4, F5 |
| `fixtures/005-pre-combat.lss` | Right before combat entry | `00:9144` | Combat-entry crash repro + combat divergence | F9, F10, F12 |
| `fixtures/006-in-combat.lss` | Mid-combat | `02:A353` | Combat bug repro (sprites/menu), btlgfx cluster bisection | F12, combat bug |
| `fixtures/007-title-screen.lss` | Title screen | `00:9144` | Title / logo / input repro | F4, F5 |
| `fixtures/008-overworld-mode7.lss` | Overworld ship → mode-7 transition | `00:9144` | **Mode-7** bug repro (corruption from ~frame 90) ; validation of the `InitMapRAM` fix (`ff4-gnw 1a86d23`) | mode-7 bug |
| `fixtures/009-first-free-roam.lss` | Player's first **free-roam** instant (first control) | `00:9133` | Field control/movement baseline (captured in SDL 2026-06-30) | — |
| `fixtures/010-castle-exit-anim.lss` | Castle exit animation (per filename; capture context not recorded) | `00:9135` | Undocumented — added to this catalog 2026-07-03, no recorded primary use or findings link | — |
| `fixtures/011-worldmap-entry.lss` | Worldmap entry (per filename; capture context not recorded) | `00:9133` | Undocumented — added to this catalog 2026-07-03, no recorded primary use or findings link | — |
| `fixtures/012-first-worldmap-combat.lss` | First random encounter triggered near Baron castle on the world map, captured right before the SPC-handshake black-screen freeze | `00:9144` | Combat black-screen repro (fixed, see `ExecSound_ext_stub` fix, ff4-gnw `450cfe8`) ; now also the reference fixture for the physical-attack-always-misses and monster-damage-oversized bugs under investigation (native-dispatch-only) | combat black-screen (fixed) |

> `desktop/seed-*.lss` (8 files) are a **separate, uncatalogued** set of ad
> hoc saves from interactive SDL sessions (`--save-prefix`) — not part of
> this catalogue, not in the `fixtures/` submodule, still plain gitignored
> local scratch. Don't confuse the two; if one of them turns out to matter
> for a specific finding, promote it into this catalogue and the submodule
> rather than leaving it as an undocumented scratch file.

> ℹ **Several fixtures share the same "PC at load" (`00:9144` / `00:9133` /
> `00:9135`).** Verified 2026-07-03 by loading each `.lss` and reading back
> `cpu->pc` — this is not a documentation error: LakeSnes's savestate
> captures whatever PC the main loop happens to be parked at (a common
> resynchronization point most scenes pass through), which is unrelated to
> which scene is actually rendered (that lives in WRAM/VRAM/CGRAM). Do not
> use this column to distinguish fixtures — use the scene description.

## Conventions

- Incremental numbering `NNN-<descriptor>.lss`, capture order.
- A fixture must be **stable** (identically replayable) and target a scene
  or a specific instant useful for a given validation.
- Divergences observed by the oracle on a fixture are logged in
  `desktop/KNOWN_FINDINGS.md` and MemPalace (`wing=ff4-gnw`).

## Getting the fixtures

**If you have network access to the author's LAN** (the normal case for
the author's own machines): `git submodule update --init fixtures` from
this repo's root populates `fixtures/` with the 11 cataloged files. This
is the primary path — no manual capture needed.

**If you don't** (a fresh clone from GitHub with no access to that LAN,
e.g. a contributor without access to the private Gitea instance): the
`fixtures/` submodule stays empty, and there are two fallbacks:

1. **Manual capture** via SDL: `make -C desktop sdl ROM=...`, reach the
   desired scene, `Space` to pause if needed, **`5`** to save the incremental
   slot (`--save-prefix`), then rename per the convention.
2. **Boot-to-scene** *(tracked in [BACKLOG](../ff4/BACKLOG.md) — not implemented)*:
   a script that drives the emulator from boot to each target scene and
   regenerates the `.lss` locally from the user's ROM, without needing
   access to the private submodule at all.

## Reproducibility note

Some fixtures expose a **delayed** bug: it only appears after N frames
of execution (e.g. `008-overworld-mode7`: FB divergence starting at ~frame 122).
Run the oracle with enough frames (`--frames 200`) to capture it.
