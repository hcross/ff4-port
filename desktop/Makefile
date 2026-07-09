# FF4 desktop validation host — see docs/adr/0001-desktop-validation-in-re-loop.md
#
# Links the LIVE ff4-gnw working tree (the artifact under test) and drives the
# same platform-agnostic ff4_* glue the G&W device uses. M0 = headless smoke
# test: boot the ROM, run N frames, hash WRAM, report dispatch hits/misses.

# Path to the ff4-gnw working tree (override: make FF4GNW=/path/to/ff4-gnw)
FF4GNW ?= ../../ff4-gnw
ROM    ?= ../upstream/rom/ff4-jp1.sfc
FRAMES ?= 600

CC     := clang
# Compile ff4-gnw under the SAME warning contract the G&W device build uses
# (retro-go-sd Makefile.common). The ported .c files do NOT include
# ff4_helpers.h and rely on implicit (int) declarations of the helpers; we
# reproduce that faithfully so the desktop host executes the identical
# translation unit as the device (bugs included — see KNOWN_FINDINGS.md:
# the void *_emu helpers used as values).
# FF4_PORT_STATIC_SNES: the device compiles LakeSnes in this config (static
# Snes storage, device pixelBuffer dims, and crucially the ff4_dispatch_try
# hook in cpu.c). The validation host MUST match it or it runs a different
# core than ships. NOTE: it makes the Snes a single static singleton — M3's
# A/B oracle must snapshot/restore rather than hold two live instances.
CFLAGS := -O2 -Wall -std=c11 -DFF4_PORT_STATIC_SNES -I$(FF4GNW) -I$(FF4GNW)/snes \
          -Wno-implicit-function-declaration -Wno-incompatible-pointer-types \
          -Wno-int-conversion -Wno-strict-aliasing -Wno-unused-function \
          -Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS :=

# Every ff4-gnw .c: the LakeSnes fork (snes/), all ported routines
# (battle/ field/ menu/ cutscene/ sound/), plus the top-level glue
# (dispatch_all.c, ff4_helpers.c, main.c). dispatch_all.c references every
# ported routine by function pointer, so the whole set must be linked.
GNW_SRC := $(wildcard $(FF4GNW)/snes/*.c) \
           $(wildcard $(FF4GNW)/battle/*.c) \
           $(wildcard $(FF4GNW)/field/*.c) \
           $(wildcard $(FF4GNW)/menu/*.c) \
           $(wildcard $(FF4GNW)/cutscene/*.c) \
           $(wildcard $(FF4GNW)/sound/*.c) \
           $(FF4GNW)/dispatch_all.c \
           $(FF4GNW)/ff4_helpers.c \
           $(FF4GNW)/main.c

BIN     := ff4-desktop-headless
BIN_SDL := ff4-desktop-sdl
BIN_ORC := ff4-desktop-oracle

# Recursive assignment (=), not immediate (:=): with :=, `sdl2-config` would
# shell out at Makefile-parse time even for a `headless-all`-only build,
# breaking on a machine without SDL2 installed (CI, a fresh headless clone).
# With =, the $(shell ...) call is deferred until $(SDL_CFLAGS)/$(SDL_LIBS)
# are actually expanded — i.e. only when the $(BIN_SDL) recipe itself runs.
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS   = $(shell sdl2-config --libs)

.PHONY: all headless-all run sdl oracle clean

all: $(BIN) $(BIN_SDL) $(BIN_ORC) ppmdiff

# CI/headless-friendly: every validation tool EXCEPT the interactive SDL
# viewer, which is the only one that needs SDL2 installed.
headless-all: $(BIN) $(BIN_ORC) wram_diff ppmdiff proof_cyc

$(BIN): harness_headless.c $(GNW_SRC)
	$(CC) $(CFLAGS) -o $@ harness_headless.c $(GNW_SRC) $(LDFLAGS)

$(BIN_SDL): main_sdl.c $(GNW_SRC)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ main_sdl.c $(GNW_SRC) $(SDL_LIBS) $(LDFLAGS)

$(BIN_ORC): oracle_ab.c $(GNW_SRC)
	$(CC) $(CFLAGS) -o $@ oracle_ab.c $(GNW_SRC) $(LDFLAGS)

# Standalone PPM pixel-diff — no SNES/ff4-gnw dependency, closes WF-VALID §5
# (mandatory screenshot validation, previously untooled).
ppmdiff: ppmdiff.c
	$(CC) -O2 -Wall -std=c11 -o $@ ppmdiff.c

# Throwaway diagnostics (not part of `all`).
input_probe: input_probe.c $(GNW_SRC)
	$(CC) $(CFLAGS) -o $@ input_probe.c $(GNW_SRC) $(LDFLAGS)

wram_diff: wram_diff.c $(GNW_SRC)
	$(CC) $(CFLAGS) -o $@ wram_diff.c $(GNW_SRC) $(LDFLAGS)

# Device-state exfiltration: build a .lss from a GDB capture of the live G&W
# (see state_inject.c header). CAPTURE_DIR must hold the *.bin dumps and the
# device_state.inc emitted by the capture GDB script.
CAPTURE_DIR ?= /tmp/ff4cap
ff4-state-inject: state_inject.c $(GNW_SRC) $(CAPTURE_DIR)/device_state.inc
	$(CC) $(CFLAGS) -I$(CAPTURE_DIR) -o $@ state_inject.c $(GNW_SRC) $(LDFLAGS)

wram_diff_dbg: wram_diff.c $(GNW_SRC)
	$(CC) $(CFLAGS) -DDEBUG_DCED -o $@ wram_diff.c $(GNW_SRC) $(LDFLAGS)

run: $(BIN)
	./$(BIN) $(ROM) $(FRAMES)

sdl: $(BIN_SDL)
	./$(BIN_SDL) $(ROM)

# M3 A/B oracle. SEED=path to a savestate; defaults to the healthy menu seed.
SEED ?= ../fixtures/004-menu.lss
oracle: $(BIN_ORC)
	./$(BIN_ORC) $(ROM) --load $(SEED) --frames $(FRAMES)

# Clean baseline: exclude the intentionally DMA-bypassing / stubbed routines
# (F3 _15cadc OAM-DMA bypass, F4 ExecSound stub, F6 TfrSprites OAM-DMA bypass,
#  F-NEW _15ca5e palette DMA bypass — same snes_syncCycles crash as _15cadc)
# so any remaining divergence is a real faithful-port bug. Healthy seeds are
# IDENTICAL under this set; combat seeds surface the next real divergence.
EXCLUDE ?= --exclude 15cadc --exclude 048004 --exclude 03fe03 --exclude 15ca5e
oracle-baseline: $(BIN_ORC)
	./$(BIN_ORC) $(ROM) --load $(SEED) --frames $(FRAMES) $(EXCLUDE)

clean:
	rm -f $(BIN) $(BIN_SDL) $(BIN_ORC)

proof_cyc: proof_cyc.c $(GNW_SRC)
	$(CC) $(CFLAGS) -o $@ proof_cyc.c $(GNW_SRC) $(LDFLAGS)

miss_profiler: miss_profiler.c $(GNW_SRC)
	$(CC) $(CFLAGS) -o $@ miss_profiler.c $(GNW_SRC) $(LDFLAGS)
