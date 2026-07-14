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
| `fixtures/013-airship-dialogue-pre-combat.lss` | Red Wings deck, soldier dialogue awaiting validation; pressing A leads straight into the Zu encounter (frame 14124 of a real playthrough) | `00:8E09` | Quick access to an intro-sequence battle without replaying ~14 k frames. **First fixture captured from the live G&W device** (GDB over ST-Link on the running game — no reset — then `desktop/ff4-state-inject` rebuilt the `.lss`; see `desktop/capture_device_state.gdb`). Caveat: APU internals are fresh-init defaults (mailbox ports only) — never use for audio validation | — |
| `fixtures/014-baron-castle-exterior.lss` | Baron castle exterior (location banner, 4+ NPC guards, animated wall flags), player just outside the gate; captured 2026-07-13 in SDL | `00:9135` | **Performance bench** for the post-adaptive-skip optimization campaign: the zone with the heaviest device slowdowns without the adaptive render skip — worse than the interior corridor that benched the 2026-07 sub-frame campaign. First target of the vramGen-invalidation investigation (do the animated flags bump `vramGen` every frame → R16/R2b cache thrash?) | — |

> `desktop/seed-*.lss` are a **separate, uncatalogued** set of ad
> hoc saves from interactive SDL sessions (`--save-prefix`) — not part of
> this catalogue, not in the `fixtures/` submodule, still plain gitignored
> local scratch. Don't confuse the two; if one of them turns out to matter
> for a specific finding, promote it into this catalogue and the submodule
> rather than leaving it as an undocumented scratch file (as was done for
> `014-baron-castle-exterior`, promoted from `seed-009.lss` on 2026-07-13).

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
this repo's root populates `fixtures/` with every cataloged file. This
is the primary path — no manual capture needed.

**If you don't** (a fresh clone from GitHub with no access to that LAN,
e.g. a contributor without access to the private Gitea instance): the
`fixtures/` submodule stays empty, and there are two fallbacks:

1. **Manual capture** via SDL: `make -C desktop sdl ROM=...`, reach the
   desired scene, `Space` to pause if needed, **`5`** to save the incremental
   slot (`--save-prefix`), then rename per the convention.
2. **Boot-to-scene** *(tracked in the umbrella
   [BACKLOG](https://github.com/hcross/ff4/blob/main/BACKLOG.md) — not implemented)*:
   a script that drives the emulator from boot to each target scene and
   regenerates the `.lss` locally from the user's ROM, without needing
   access to the private submodule at all.

## Capturing new fixtures from the live device

Two device→desktop pipelines exist (both author-side — they need the
physical unit and the SWD probe):

1. **GDB live capture, no reset** (how `013` was made): halt the running
   game over ST-Link, dump the SNES state regions, rebuild the `.lss`
   with `desktop/ff4-state-inject` — see `desktop/capture_device_state.gdb`.
   Caveat on APU internals in the `013` row above.
2. **Pause-menu savestates (2026-07-14)**: states saved on the device by
   the retro-go-sd pause menu (TAMP-compressed on the internal LittleFS)
   load **byte-identical** in the desktop harness once extracted.
   Extraction recipe: `gnwmanager dump` the external-flash filesystem
   region — offset `3407872`, size `786432` since the 768 KB filesystem
   resize (retro-go-sd `a3792a60`) — then reverse each 4096-byte block
   (the filesystem is stored block-reversed from the region's end),
   mount the result with `littlefs-python`, and TAMP-decompress the slot
   file back to a plain `.lss`. Afterwards run
   `gnwmanager start 0x08100000`: `gnwmanager dump`/`ls` replace the
   running app with gnwmanager's stub (see the umbrella
   [WF-RELEASE guardrails](https://github.com/hcross/ff4/blob/main/workflows/WF-RELEASE.md)).

## Reproducibility note

Some fixtures expose a **delayed** bug: it only appears after N frames
of execution (e.g. `008-overworld-mode7`: FB divergence starting at ~frame 122).
Run the oracle with enough frames (`--frames 200`) to capture it.
