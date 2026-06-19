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

SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS   := $(shell sdl2-config --libs)

.PHONY: all run sdl clean

all: $(BIN) $(BIN_SDL)

$(BIN): harness_headless.c $(GNW_SRC)
	$(CC) $(CFLAGS) -o $@ harness_headless.c $(GNW_SRC) $(LDFLAGS)

$(BIN_SDL): main_sdl.c $(GNW_SRC)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $@ main_sdl.c $(GNW_SRC) $(SDL_LIBS) $(LDFLAGS)

run: $(BIN)
	./$(BIN) $(ROM) $(FRAMES)

sdl: $(BIN_SDL)
	./$(BIN_SDL) $(ROM)

clean:
	rm -f $(BIN) $(BIN_SDL)
