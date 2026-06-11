# FF4 parity harness — Phase 2 scaffold
# Headless build (no SDL): we link the LakeSnes core directly as static sources.

LAKESNES := ../LakeSnes
ROM       := ../upstream/rom/ff4-jp1.sfc

CC       := clang
CFLAGS   := -O2 -Wall -Wextra -std=c11 -I$(LAKESNES)/snes -I$(LAKESNES)/zip
LDFLAGS  :=

CORE_SRC := \
	$(LAKESNES)/snes/spc.c \
	$(LAKESNES)/snes/dsp.c \
	$(LAKESNES)/snes/apu.c \
	$(LAKESNES)/snes/cpu.c \
	$(LAKESNES)/snes/dma.c \
	$(LAKESNES)/snes/ppu.c \
	$(LAKESNES)/snes/cart.c \
	$(LAKESNES)/snes/input.c \
	$(LAKESNES)/snes/statehandler.c \
	$(LAKESNES)/snes/snes.c \
	$(LAKESNES)/snes/snes_other.c

BIN_SAMPLE   := ff4-parity
BIN_COMPARE  := ff4-parity-compare
BIN_SPIKE    := ff4-spike-calchits
BIN_SPIKE_M2 := ff4-spike-calchits-m2
BIN_SPIKE_M3 := ff4-spike-apply-dmg-mult
BIN_SPIKE_M4 := ff4-spike-get-dmg-ptr
BIN_SPIKE_M5 := ff4-spike-calc-dmg
BIN_SPIKE_M6 := ff4-spike-apply-dmg

.PHONY: all clean run run-self run-patch run-spike run-spike-m2 run-spike-m3 run-spike-m4 run-spike-m5 run-spike-m6

all: $(BIN_SAMPLE) $(BIN_COMPARE) $(BIN_SPIKE) $(BIN_SPIKE_M2) $(BIN_SPIKE_M3) $(BIN_SPIKE_M4) $(BIN_SPIKE_M5) $(BIN_SPIKE_M6)

$(BIN_SPIKE_M6): src/spike_apply_dmg_m6.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_apply_dmg_m6.c $(CORE_SRC) $(LDFLAGS)

run-spike-m6: $(BIN_SPIKE_M6)
	./$(BIN_SPIKE_M6) $(ROM)

$(BIN_SPIKE_M5): src/spike_calc_dmg_m5.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calc_dmg_m5.c $(CORE_SRC) $(LDFLAGS)

run-spike-m5: $(BIN_SPIKE_M5)
	./$(BIN_SPIKE_M5) $(ROM) 100

$(BIN_SPIKE_M4): src/spike_get_dmg_ptr_m4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_get_dmg_ptr_m4.c $(CORE_SRC) $(LDFLAGS)

run-spike-m4: $(BIN_SPIKE_M4)
	./$(BIN_SPIKE_M4) $(ROM)

$(BIN_SPIKE_M3): src/spike_apply_dmg_mult_m3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_apply_dmg_mult_m3.c $(CORE_SRC) $(LDFLAGS)

run-spike-m3: $(BIN_SPIKE_M3)
	./$(BIN_SPIKE_M3) $(ROM) 1000


$(BIN_SPIKE): src/spike_calchits_emu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calchits_emu.c $(CORE_SRC) $(LDFLAGS)

$(BIN_SPIKE_M2): src/spike_calchits_m2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calchits_m2.c $(CORE_SRC) $(LDFLAGS)

run-spike: $(BIN_SPIKE)
	./$(BIN_SPIKE) $(ROM)

run-spike-m2: $(BIN_SPIKE_M2)
	./$(BIN_SPIKE_M2) $(ROM) 1000


$(BIN_SAMPLE): src/parity_main.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/parity_main.c $(CORE_SRC) $(LDFLAGS)

$(BIN_COMPARE): src/parity_compare.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/parity_compare.c $(CORE_SRC) $(LDFLAGS)

run: $(BIN_SAMPLE)
	./$(BIN_SAMPLE) $(ROM) 600

# Self-consistency: vanilla ROM vs vanilla ROM → MUST report zero divergence
run-self: $(BIN_COMPARE)
	./$(BIN_COMPARE) $(ROM) $(ROM) 600

# Sensitivity: flip one byte in the test ROM → MUST diverge quickly
run-patch: $(BIN_COMPARE)
	./$(BIN_COMPARE) $(ROM) $(ROM) 600 --patch 0x8000:00

clean:
	rm -f $(BIN_SAMPLE) $(BIN_COMPARE) $(BIN_SPIKE) $(BIN_SPIKE_M2) $(BIN_SPIKE_M3) $(BIN_SPIKE_M4) $(BIN_SPIKE_M5) $(BIN_SPIKE_M6)
