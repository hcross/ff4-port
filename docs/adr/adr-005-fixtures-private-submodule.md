# ADR-005 — Reference savestates live in a private Gitea submodule, not gitignored local files

- **Status**: Accepted, implemented 2026-07-05
- **Date**: 2026-07-05
- **Deciders**: Hoani Cross, claude-code
- **Scope**: `ff4-port/fixtures/` (new git submodule), `FIXTURES.md`,
  `desktop/Makefile`, `desktop/KNOWN_FINDINGS.md`, `.gitignore`,
  `ff4/scripts/regress.sh`, `ff4/AGENTS.md`, `ff4/workflows/WF-VALID.md`

## Context

The 11 catalogued reference savestates (`FIXTURES.md`) are the reproducible
entry points the desktop A/B oracle, `wram_diff`, the spikes, and SDL
inspection all depend on. Each one embeds SNES WRAM + VRAM + CGRAM
(palettes) + ARAM — fragments of copyrighted Square Enix ROM data — so
they could never be committed to this (public, GitHub-hosted) repo. Until
now the only mitigation was `.gitignore`: the 11 files sat as plain local
files on the author's machine, never version-controlled, never backed up.

This was flagged as blind-spot #6 of the 2026-07-03 acceleration audit
(MemPalace `wing=ff4-gnw room=blind-spots`, point 6): fixtures in a single
local copy, with no scripted regeneration — no way to recover them if lost
or corrupted, and no way to share the exact set with another one of the
author's own machines short of manually copying files. The `.gitignore`
comment at the time even flagged "provenance unconfirmed" for some of them.

## Decision

Move the 11 files into their own repository, `hcross/ff4-fixtures`,
hosted on the author's **private, self-hosted Gitea instance** ("Odan",
LAN-only, `192.168.196.254`) — never GitHub, never any public host — and
mount it as a git submodule at `ff4-port/fixtures/`.

This is not a relaxation of the copyright constraint: the destination is
private infrastructure the author already uses for other personal/private
repos (via the `tea` CLI), reachable only from the author's own LAN. It
gives the fixtures real version control (history, diff-detection if a
capture is accidentally corrupted) and a durable backup, without the
files ever touching a public, third-party-hosted repository.

`desktop/seed-*.lss` (7 files, uncatalogued ad hoc SDL-session saves, not
referenced by `FIXTURES.md` or `scripts/regress.sh`) were deliberately
**left out of this migration** — they aren't part of the documented
fixture set and migrating undocumented scratch files would just relocate
the lack-of-catalogue problem rather than fix it. If one of them turns out
to matter for a specific finding, it should be promoted into the catalogue
(and the submodule) deliberately, not swept in wholesale.

## Consequences

### Positive
- Real backup and version history for the 11 fixtures, closing
  blind-spot #6 for the documented set.
- A fresh clone on any of the author's own machines with LAN access to
  Odan gets the fixtures via a single `git submodule update --init
  fixtures` — no more manual re-capture or file copying between machines.
- The copyright constraint that originally justified `.gitignore` is
  fully preserved: the data still never appears on GitHub or any public
  host, just on infrastructure the author already controls and trusts.

### Negative
- **A clone of the public `ff4-port` repo from GitHub has an empty
  `fixtures/` submodule** unless the cloning machine also has network
  access to the private Gitea instance. This is a real, accepted
  limitation for any future external contributor — `FIXTURES.md`
  documents the two fallbacks (manual SDL capture; the not-yet-implemented
  boot-to-scene script tracked in `ff4/BACKLOG.md`).
- One more submodule to keep pinned and updated (same operational
  overhead as the existing `LakeSnes` and `upstream` submodules).
- Introduces a dependency on the author's home-lab infrastructure being
  reachable for this one workflow step; unlike GitHub, Odan has no
  uptime guarantee. Accepted as a reasonable trade given the alternative
  (no version control at all) was strictly worse.

## Alternatives rejected

- **Keep the status quo (gitignored local files).** Rejected: this is the
  exact blind-spot #6 problem this ADR exists to close — no backup, no
  shared history, no recovery path if the files are lost.
- **A private GitHub repository instead of self-hosted Gitea.** Not
  chosen: the author already operates a private, self-hosted Gitea
  instance for exactly this kind of personal/sensitive-content repo (per
  the author's own standing infrastructure), so this reuses existing,
  already-trusted infrastructure rather than introducing a new one.
  Not evaluated against GitHub private-repo pricing/limits in any
  quantitative way — this was a "use the infrastructure that already
  exists for this purpose" call, not a cost/feature comparison.
- **Encrypt the fixtures and commit the encrypted blobs to this public
  repo.** Not evaluated in depth; would still require distributing a
  decryption key out-of-band, which is roughly the same trust/reachability
  problem as a private submodule, with added complexity (key management,
  encrypt/decrypt tooling) and no clear benefit over just using a private
  host directly.
- **The not-yet-implemented "boot-to-scene" regeneration script**
  (`ff4/BACKLOG.md`) as the sole fix, without a private submodule. Rejected
  as the *complete* fix for now: it would solve reproducibility for anyone
  with the vanilla ROM, but is nontrivial to build (driving the emulator
  from boot to 11 distinct, specific in-game moments) and remains a
  documented, tracked fallback rather than a replacement for having a
  real backup of the already-captured, known-good fixtures today.
