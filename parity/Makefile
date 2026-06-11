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

# AUTO: spike for CalcHits
ff4-spike-auto-calchits: src/spike_calchits_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calchits_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_17
ff4-spike-auto-aitarget_17: src/spike_aitarget_17_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_17_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AICond_02
ff4-spike-auto-aicond_02: src/spike_aicond_02_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_16
ff4-spike-auto-aitarget_16: src/spike_aitarget_16_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_16_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_19
ff4-spike-auto-aitarget_19: src/spike_aitarget_19_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_19_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_1a
ff4-spike-auto-aitarget_1a: src/spike_aitarget_1a_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1a_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_1b
ff4-spike-auto-aitarget_1b: src/spike_aitarget_1b_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1b_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for GetFrontRowChars
ff4-spike-auto-getfrontrowchars: src/spike_getfrontrowchars_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getfrontrowchars_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for GetBackRowChars
ff4-spike-auto-getbackrowchars: src/spike_getbackrowchars_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbackrowchars_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_1e
ff4-spike-auto-aitarget_1e: src/spike_aitarget_1e_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1e_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_1f
ff4-spike-auto-aitarget_1f: src/spike_aitarget_1f_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1f_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_20
ff4-spike-auto-aitarget_20: src/spike_aitarget_20_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_20_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_21
ff4-spike-auto-aitarget_21: src/spike_aitarget_21_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_21_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_22
ff4-spike-auto-aitarget_22: src/spike_aitarget_22_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_22_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_23
ff4-spike-auto-aitarget_23: src/spike_aitarget_23_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_23_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_24
ff4-spike-auto-aitarget_24: src/spike_aitarget_24_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_24_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_25
ff4-spike-auto-aitarget_25: src/spike_aitarget_25_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_25_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITarget_29
ff4-spike-auto-aitarget_29: src/spike_aitarget_29_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_29_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AITargetTbl
ff4-spike-auto-aitargettbl: src/spike_aitargettbl_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitargettbl_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AICond_00
ff4-spike-auto-aicond_00: src/spike_aicond_00_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_00_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO: spike for AICond_03
ff4-spike-auto-aicond_03: src/spike_aicond_03_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_03_auto.c $(CORE_SRC) $(LDFLAGS)
