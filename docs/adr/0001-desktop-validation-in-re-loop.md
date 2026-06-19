# ADR 0001 — Desktop LakeSnes host as the RE validation gate

- **Status**: Accepted
- **Date**: 2026-06-18
- **Deciders**: Hoani Cross, claude-code (Opus 4.8)
- **Scope**: `ff4-port` (RE methodology / harness) primarily; consumes
  `ff4-gnw` (ported game library) sources

## Context

The native C port of Final Fantasy IV (SNES → retro-go-sd on the Game & Watch
STM32H7B0) boots and runs (full title screen, input, opening). Many hooks
(native C routines substituted for interpreted 65816 via the dispatch table)
have been ported and individually validated.

**Motivating failure.** Letting the game run on the G&W into a battle scene
produced a **blue screen**. The lesson is structural, not incidental: a large
number of C hooks were validated **in isolation** — each `spike_*.c` in
`ff4-port/parity/` compares one reimplementation against the interpreted ROM
for a hand-picked input — but their **integrated** correctness over real
gameplay was never factually verified. A hook can pass its spike yet break in
context: unexercised code paths, cross-routine state assumptions, ordering and
side-effect interactions the spike never drove.

Two existing facts make a cheap remedy possible:

- **LakeSnes is already desktop-capable**: SDL frontend (`LakeSnes/main.c`),
  core compiled with `clang -O2 -std=c11` by the parity harness.
- **The game logic is already isolated** behind a platform-agnostic API in
  `ff4-gnw/main.c`: `ff4_init(rom, len)`, `ff4_step()`,
  `ff4_blit_to_lcd(uint16_t*)`, `ff4_set_button(p, btn, down)`,
  `ff4_get_state()`, `ff4_shutdown()`, `extern Snes *ff4_snes`. The device
  driver `retro-go-sd/.../main_ff4.c` only wraps this glue with the G&W
  frontend (odroid/gw_lcd/gw_audio) and the serial savestate stream.

`ff4_step()` calls `snes_runFrame()` → `ppu_runLine()`, so a desktop host runs
**the exact same game logic and renderer as the device**, bit for bit. The
only thing missing on desktop is a `main()` and a screen.

Separately, the renderer is the next performance target (per-pixel LakeSnes
compositor, ~3.7–4 fps mode 1, ~6 fps mode 7 on device). Any renderer rework
needs a fast correctness loop too — the same host serves both needs.

## Decision

Introduce a **desktop LakeSnes host under `ff4-port/desktop/`** and make it a
**mandatory validation gate in the reverse-engineering loop**. It links the
`ff4-gnw` sources directly and drives the same `ff4_*` glue the device uses.

Division of responsibility:

- **`ff4-port`** owns RE and its validation. A routine is not "done" when its
  spike passes — it is done when it survives the **full-game A/B
  reconciliation oracle** below. Desktop validation is part of the RE loop.
- **`ff4-gnw`** is the ported library. The final G&W adaptation deals only
  with **platform-specific** concerns (flash layout, LCD blit, audio, watchdog,
  savestate streaming, wall-clock perf) — not with reproving game-logic
  correctness.
- The per-function spikes (`ff4-port/parity/`) are kept as a fast first
  filter; the desktop host adds the **integration** layer they lack.

### Linking `ff4-gnw` into `ff4-port/desktop/`

The host compiles the **live `ff4-gnw` working tree** via a relative path,
overridable: `FF4GNW ?= ../../ff4-gnw`. This intentionally tests the tree being
edited (edit routine → rebuild host → run oracle, no sync step). A submodule
pin is rejected: it re-introduces the multi-checkout desync that already cost a
recovery (working tree vs `retro-go-sd/external/ff4` diverged on
`main` vs `feat/run-emulated-func`), and the validation loop wants the live
tree, not a pinned revision.

### Execution plan (milestones, each verifiable)

| # | Deliverable | Verification |
|---|-------------|--------------|
| **M0** | `ff4-port/desktop/`: headless link of `ff4-gnw`, harness = `ff4_init` + N× `ff4_step` + dispatch hits/misses + framebuffer CRC32. | Compiles + runs N frames, no STM32-isms in the fork. |
| **M1** | `ff4_blit_to_lcd` (RGB565) → PNG; `--frames N --out f.png`. | Title PNG == device baseline `/tmp/ff4_now_active.png`. |
| **M2** | `main_sdl.c` (from `LakeSnes/main.c`): window, keyboard→input, save/load state (`statehandler.h`). | Port is playable on desktop; jump to a target scene via savestate. |
| **M3** | **Full-game A/B reconciliation oracle** — *the centerpiece*. Two `Snes` instances in lockstep: dispatch **ON** (native hooks) vs dispatch **OFF** (pure interpreter = ground truth). Per-frame diff of WRAM + framebuffer CRC; report first divergence (PC + frame). | A full boot→field→battle session diverges nowhere; when it does, it names the culprit. |
| **M4** | Profilable build (`-pg` / callgrind / `perf`). | Instruction-level attribution of `ppu_runLine`. |

A small enabling change: a global toggle (e.g. `bool ff4_dispatch_enabled`)
so the same binary can run the ON and OFF sides of the A/B without two builds.

### Resulting RE workflow

```
reverse a routine ─▶ spike passes (parity/, isolated)
                        │
                        ▼
            enable hook in dispatch table
                        │
                        ▼
   desktop A/B oracle over relevant gameplay  ◀── new mandatory gate
        (dispatch ON vs interpreter, zero divergence)
                        │
                        ▼
        device build ─ platform-specific issues only
```

**First application**: run M3 from a pre-battle savestate through the battle
that blue-screened. The first PC where native-dispatch diverges from the
interpreter is the offending hook — turning an opaque device crash into a
named regression.

## Consequences

### Positive
- Render/RE feedback loop in **seconds**, no flash/SWD cycle, no powered device.
- Desktop tooling: gdb/lldb, **ASan/UBSan** (surfaces DP/overflow bugs invisible
  on device), callgrind, scriptable PNG diff.
- M3 becomes a **permanent regression net**: it both finds the existing
  blue-screen culprit and guarantees future work (e.g. a per-scanline renderer)
  changes no pixel.

### Caveat — never forget
**Desktop instruction-count ≠ device wall-clock.** The STM32H7B0 is in-order
with slow external RAM and costly scattered access; the dev box is superscalar
out-of-order with large caches. Desktop establishes **correctness** and
**algorithmic attribution**; the device remains the **only judge of fps**.
Every desktop perf win is reconfirmed on device before it counts.

> Concrete corollary: the earlier "the brightness `/15` costs 6 divides/pixel"
> claim is likely overstated — division by the constant 15 is strength-reduced
> by the compiler (multiply-magic + shift, even on M7), not a hardware `udiv`.
> M4 settles it by measurement before any renderer change.

### Negative / costs
- One third-party header (`stb_image_write`) for M1; SDL2 as a desktop build
  dependency for M2 — no impact on the device build.
- The `ff4-gnw/snes/ppu.c` fork (with dispatch) must compile standalone on
  desktop; low risk (portable C), flushed out at M0.
- `ff4-port/desktop/` couples to `ff4-gnw`'s layout via a relative path
  (`FF4GNW`); acceptable for a personal mono-tree, overridable.

## Alternatives rejected

- **Stay device-only.** Feedback loop too slow for iterative renderer work, and
  it failed to catch the integration regression in the first place.
- **Put `desktop/` in `ff4-gnw`.** Rejected per this revision: validation of
  RE'd routines belongs to the RE methodology repo (`ff4-port`); `ff4-gnw` is
  the artifact under test, and its final adaptation should handle only
  platform-specific concerns.
- **Extend the parity harness to whole-game.** It links the pristine LakeSnes
  core, not the dispatch fork; rebuilding it would re-implement the desktop
  host without reusing the existing `ff4_*` glue — and it is per-function by
  design, the very limitation that let the regression through.
- **`ff4-gnw` as a submodule of `ff4-port`.** Re-introduces multi-checkout
  desync; the live working tree is what the loop must validate.
