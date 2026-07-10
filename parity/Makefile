# FF4 parity harness — Phase 2 scaffold
# Headless build (no SDL): we link the LakeSnes core directly as static sources.

LAKESNES := ../LakeSnes
ROM       := ../upstream/rom/ff4-jp1.sfc

CC       := clang
# Same warning contract as ff4-port/desktop/Makefile and the retro-go-sd
# device build: ff4-gnw .c files linked in as SPIKE_EXTRA_SRC call sibling
# helpers (read16/write16, *_emu delegates) via implicit (int) declarations
# by design (they don't include ff4_helpers.h) -- reproduce that here too,
# or those files fail to compile as hard errors instead of the warnings
# the device build tolerates.
CFLAGS   := -O2 -Wall -Wextra -std=c11 -I$(LAKESNES) -I$(LAKESNES)/snes -I$(LAKESNES)/zip \
            -Wno-implicit-function-declaration -Wno-incompatible-pointer-types \
            -Wno-int-conversion -Wno-strict-aliasing
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

# AUTO (qwen3): spike for AICondTargetTbl
ff4-spike-qwen3-aicondtargettbl: src/spike_aicondtargettbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtargettbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICondTarget_25
ff4-spike-gemma4-aicondtarget_25: src/spike_aicondtarget_25_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICondTarget_23
ff4-spike-qwen3-aicondtarget_23: src/spike_aicondtarget_23_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_23_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICondTarget_26
ff4-spike-gemma4-aicondtarget_26: src/spike_aicondtarget_26_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_26_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICondTarget_25
ff4-spike-qwen3-aicondtarget_25: src/spike_aicondtarget_25_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICondTarget_27
ff4-spike-gemma4-aicondtarget_27: src/spike_aicondtarget_27_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_27_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICondTarget_26
ff4-spike-qwen3-aicondtarget_26: src/spike_aicondtarget_26_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_26_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICond_00
ff4-spike-gemma4-aicond_00: src/spike_aicond_00_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_00_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICondTarget_27
ff4-spike-qwen3-aicondtarget_27: src/spike_aicondtarget_27_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_27_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICond_02
ff4-spike-gemma4-aicond_02: src/spike_aicond_02_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICondTbl
ff4-spike-qwen3-aicondtbl: src/spike_aicondtbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICond_03
ff4-spike-gemma4-aicond_03: src/spike_aicond_03_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_03_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICond_00
ff4-spike-qwen3-aicond_00: src/spike_aicond_00_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_00_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICond_02
ff4-spike-qwen3-aicond_02: src/spike_aicond_02_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICond_04
ff4-spike-gemma4-aicond_04: src/spike_aicond_04_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_04_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICond_05
ff4-spike-gemma4-aicond_05: src/spike_aicond_05_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICond_03
ff4-spike-qwen3-aicond_03: src/spike_aicond_03_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_03_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICond_06
ff4-spike-gemma4-aicond_06: src/spike_aicond_06_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_06_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICond_04
ff4-spike-qwen3-aicond_04: src/spike_aicond_04_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_04_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICond_09
ff4-spike-gemma4-aicond_09: src/spike_aicond_09_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_09_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICond_05
ff4-spike-qwen3-aicond_05: src/spike_aicond_05_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICond_0a
ff4-spike-gemma4-aicond_0a: src/spike_aicond_0a_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_0a_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICond_06
ff4-spike-qwen3-aicond_06: src/spike_aicond_06_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_06_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AICond_0b
ff4-spike-gemma4-aicond_0b: src/spike_aicond_0b_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_0b_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICond_09
ff4-spike-qwen3-aicond_09: src/spike_aicond_09_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_09_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICond_0a
ff4-spike-qwen3-aicond_0a: src/spike_aicond_0a_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_0a_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AITarget_1e
ff4-spike-gemma4-aitarget_1e: src/spike_aitarget_1e_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1e_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AICond_0b
ff4-spike-qwen3-aicond_0b: src/spike_aicond_0b_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_0b_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITargetTbl
ff4-spike-qwen3-aitargettbl: src/spike_aitargettbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitargettbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_16
ff4-spike-qwen3-aitarget_16: src/spike_aitarget_16_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_16_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_19
ff4-spike-qwen3-aitarget_19: src/spike_aitarget_19_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_19_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AITarget_1f
ff4-spike-gemma4-aitarget_1f: src/spike_aitarget_1f_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1f_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_1a
ff4-spike-qwen3-aitarget_1a: src/spike_aitarget_1a_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1a_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_1b
ff4-spike-qwen3-aitarget_1b: src/spike_aitarget_1b_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1b_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_1e
ff4-spike-qwen3-aitarget_1e: src/spike_aitarget_1e_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1e_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AITarget_22
ff4-spike-gemma4-aitarget_22: src/spike_aitarget_22_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_22_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AITarget_29
ff4-spike-gemma4-aitarget_29: src/spike_aitarget_29_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_29_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_1f
ff4-spike-qwen3-aitarget_1f: src/spike_aitarget_1f_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1f_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Add16
ff4-spike-gemma4-add16: src/spike_add16_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_add16_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_20
ff4-spike-qwen3-aitarget_20: src/spike_aitarget_20_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_20_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Asl_6
ff4-spike-gemma4-asl_6: src/spike_asl_6_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_asl_6_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CheckAICond
ff4-spike-gemma4-checkaicond: src/spike_checkaicond_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkaicond_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Cmd_21
ff4-spike-gemma4-cmd_21: src/spike_cmd_21_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_21_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_21
ff4-spike-qwen3-aitarget_21: src/spike_aitarget_21_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_21_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CountBits
ff4-spike-gemma4-countbits: src/spike_countbits_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_countbits_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for DrawMP
ff4-spike-gemma4-drawmp: src/spike_drawmp_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawmp_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for EndMultiAttack
ff4-spike-gemma4-endmultiattack: src/spike_endmultiattack_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endmultiattack_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_22
ff4-spike-qwen3-aitarget_22: src/spike_aitarget_22_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_22_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_23
ff4-spike-qwen3-aitarget_23: src/spike_aitarget_23_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_23_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_24
ff4-spike-qwen3-aitarget_24: src/spike_aitarget_24_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_24_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_25
ff4-spike-qwen3-aitarget_25: src/spike_aitarget_25_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_25_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AITarget_29
ff4-spike-qwen3-aitarget_29: src/spike_aitarget_29_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_29_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AnyTarget
ff4-spike-qwen3-anytarget: src/spike_anytarget_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_anytarget_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckAICond
ff4-spike-qwen3-checkaicond: src/spike_checkaicond_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkaicond_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CmdTbl
ff4-spike-qwen3-cmdtbl: src/spike_cmdtbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmdtbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Cmd_21
ff4-spike-qwen3-cmd_21: src/spike_cmd_21_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_21_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Div16
ff4-spike-qwen3-div16: src/spike_div16_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_div16_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for DrawMP
ff4-spike-qwen3-drawmp: src/spike_drawmp_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawmp_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for EndMultiAttack
ff4-spike-qwen3-endmultiattack: src/spike_endmultiattack_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endmultiattack_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AddMsg1
ff4-spike-gemma4-addmsg1: src/spike_addmsg1_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg1_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AddMsg2
ff4-spike-gemma4-addmsg2: src/spike_addmsg2_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg2_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AddMsg3
ff4-spike-gemma4-addmsg3: src/spike_addmsg3_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg3_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for ApplyDmgMult
ff4-spike-gemma4-applydmgmult: src/spike_applydmgmult_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_applydmgmult_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for ApplySpeedMod
ff4-spike-gemma4-applyspeedmod: src/spike_applyspeedmod_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_applyspeedmod_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AutoBattlePtrs
ff4-spike-gemma4-autobattleptrs: src/spike_autobattleptrs_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_autobattleptrs_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AutoBattle_0001
ff4-spike-gemma4-autobattle_0001: src/spike_autobattle_0001_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_autobattle_0001_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Add16
ff4-spike-qwen3-add16: src/spike_add16_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_add16_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AddMsg1
ff4-spike-qwen3-addmsg1: src/spike_addmsg1_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg1_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AddMsg2
ff4-spike-qwen3-addmsg2: src/spike_addmsg2_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg2_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AddMsg3
ff4-spike-qwen3-addmsg3: src/spike_addmsg3_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg3_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for AutoBattle_0007
ff4-spike-gemma4-autobattle_0007: src/spike_autobattle_0007_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_autobattle_0007_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CalcHits
ff4-spike-gemma4-calchits: src/spike_calchits_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calchits_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for ApplyDmgMult
ff4-spike-qwen3-applydmgmult: src/spike_applydmgmult_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_applydmgmult_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for ApplySpeedMod
ff4-spike-qwen3-applyspeedmod: src/spike_applyspeedmod_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_applyspeedmod_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CheckBattleList
ff4-spike-gemma4-checkbattlelist: src/spike_checkbattlelist_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkbattlelist_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Asl_6
ff4-spike-qwen3-asl_6: src/spike_asl_6_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_asl_6_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AutoBattlePtrs
ff4-spike-qwen3-autobattleptrs: src/spike_autobattleptrs_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_autobattleptrs_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AutoBattle_0003
ff4-spike-qwen3-autobattle_0003: src/spike_autobattle_0003_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_autobattle_0003_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for AutoBattle_0004
ff4-spike-qwen3-autobattle_0004: src/spike_autobattle_0004_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_autobattle_0004_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CalcHits
ff4-spike-qwen3-calchits: src/spike_calchits_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calchits_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CalcTimerDur
ff4-spike-qwen3-calctimerdur: src/spike_calctimerdur_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calctimerdur_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckAttackName
ff4-spike-qwen3-checkattackname: src/spike_checkattackname_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkattackname_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckBattleList
ff4-spike-qwen3-checkbattlelist: src/spike_checkbattlelist_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkbattlelist_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckBit
ff4-spike-qwen3-checkbit: src/spike_checkbit_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkbit_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckCanUseItem
ff4-spike-qwen3-checkcanuseitem: src/spike_checkcanuseitem_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkcanuseitem_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckFanfare
ff4-spike-qwen3-checkfanfare: src/spike_checkfanfare_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkfanfare_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckMonsterFlash
ff4-spike-qwen3-checkmonsterflash: src/spike_checkmonsterflash_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkmonsterflash_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckRegen
ff4-spike-qwen3-checkregen: src/spike_checkregen_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkregen_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckStrongElem
ff4-spike-qwen3-checkstrongelem: src/spike_checkstrongelem_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkstrongelem_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckTimer
ff4-spike-qwen3-checktimer: src/spike_checktimer_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktimer_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckWeakElem
ff4-spike-qwen3-checkweakelem: src/spike_checkweakelem_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkweakelem_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CheckWinAnim
ff4-spike-qwen3-checkwinanim: src/spike_checkwinanim_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkwinanim_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for ClearBit
ff4-spike-qwen3-clearbit: src/spike_clearbit_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearbit_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for ClearInvalidTargets
ff4-spike-qwen3-clearinvalidtargets: src/spike_clearinvalidtargets_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearinvalidtargets_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CmdDelayTbl
ff4-spike-qwen3-cmddelaytbl: src/spike_cmddelaytbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmddelaytbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CmdTargetTbl
ff4-spike-qwen3-cmdtargettbl: src/spike_cmdtargettbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmdtargettbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Cmd_01
ff4-spike-qwen3-cmd_01: src/spike_cmd_01_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_01_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Cmd_08
ff4-spike-qwen3-cmd_08: src/spike_cmd_08_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_08_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Cmd_0c
ff4-spike-qwen3-cmd_0c: src/spike_cmd_0c_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0c_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Cmd_0e
ff4-spike-qwen3-cmd_0e: src/spike_cmd_0e_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0e_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Cmd_0f
ff4-spike-qwen3-cmd_0f: src/spike_cmd_0f_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0f_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Cmd_22
ff4-spike-qwen3-cmd_22: src/spike_cmd_22_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_22_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CountBits
ff4-spike-qwen3-countbits: src/spike_countbits_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_countbits_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for CountCharTargets
ff4-spike-qwen3-countchartargets: src/spike_countchartargets_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_countchartargets_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for DisableCmdStatusTbl
ff4-spike-qwen3-disablecmdstatustbl: src/spike_disablecmdstatustbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_disablecmdstatustbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for DoAction
ff4-spike-qwen3-doaction: src/spike_doaction_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_doaction_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for DrawStolenItem
ff4-spike-qwen3-drawstolenitem: src/spike_drawstolenitem_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawstolenitem_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for EquipStatTbl
ff4-spike-qwen3-equipstattbl: src/spike_equipstattbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_equipstattbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for ExecBattle
ff4-spike-qwen3-execbattle: src/spike_execbattle_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execbattle_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for ExecBtlGfx
ff4-spike-qwen3-execbtlgfx: src/spike_execbtlgfx_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execbtlgfx_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for ExecCmd
ff4-spike-qwen3-execcmd: src/spike_execcmd_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execcmd_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for FindValidChar
ff4-spike-qwen3-findvalidchar: src/spike_findvalidchar_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_findvalidchar_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for GetAntiElem
ff4-spike-qwen3-getantielem: src/spike_getantielem_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getantielem_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for GetBackRowChars
ff4-spike-qwen3-getbackrowchars: src/spike_getbackrowchars_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbackrowchars_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for GetDmgPtr
ff4-spike-qwen3-getdmgptr: src/spike_getdmgptr_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdmgptr_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for GetFrontRowChars
ff4-spike-qwen3-getfrontrowchars: src/spike_getfrontrowchars_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getfrontrowchars_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for GetObjPtr
ff4-spike-qwen3-getobjptr: src/spike_getobjptr_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getobjptr_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for GetPendingAction
ff4-spike-qwen3-getpendingaction: src/spike_getpendingaction_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getpendingaction_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for GetScriptPtr
ff4-spike-qwen3-getscriptptr: src/spike_getscriptptr_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getscriptptr_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for GetShieldProp
ff4-spike-qwen3-getshieldprop: src/spike_getshieldprop_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getshieldprop_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for GetSummonAttack
ff4-spike-qwen3-getsummonattack: src/spike_getsummonattack_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getsummonattack_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for GetTimerPtr
ff4-spike-qwen3-gettimerptr: src/spike_gettimerptr_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettimerptr_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for InitAction
ff4-spike-qwen3-initaction: src/spike_initaction_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initaction_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for InitCharRows
ff4-spike-qwen3-initcharrows: src/spike_initcharrows_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initcharrows_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Lsr_6
ff4-spike-qwen3-lsr_6: src/spike_lsr_6_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_lsr_6_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicDmgEffect
ff4-spike-qwen3-magicdmgeffect: src/spike_magicdmgeffect_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magicdmgeffect_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_03
ff4-spike-qwen3-magiceffect_03: src/spike_magiceffect_03_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_03_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_04
ff4-spike-qwen3-magiceffect_04: src/spike_magiceffect_04_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_04_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_08
ff4-spike-qwen3-magiceffect_08: src/spike_magiceffect_08_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_08_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_0c
ff4-spike-qwen3-magiceffect_0c: src/spike_magiceffect_0c_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0c_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_0d
ff4-spike-qwen3-magiceffect_0d: src/spike_magiceffect_0d_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0d_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_0e
ff4-spike-qwen3-magiceffect_0e: src/spike_magiceffect_0e_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0e_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_0f
ff4-spike-qwen3-magiceffect_0f: src/spike_magiceffect_0f_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0f_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_13
ff4-spike-qwen3-magiceffect_13: src/spike_magiceffect_13_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_13_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_15
ff4-spike-qwen3-magiceffect_15: src/spike_magiceffect_15_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_15_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_16
ff4-spike-qwen3-magiceffect_16: src/spike_magiceffect_16_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_16_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_20
ff4-spike-qwen3-magiceffect_20: src/spike_magiceffect_20_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_20_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_21
ff4-spike-qwen3-magiceffect_21: src/spike_magiceffect_21_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_21_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_22
ff4-spike-qwen3-magiceffect_22: src/spike_magiceffect_22_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_22_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_23
ff4-spike-qwen3-magiceffect_23: src/spike_magiceffect_23_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_23_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_24
ff4-spike-qwen3-magiceffect_24: src/spike_magiceffect_24_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_24_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_25
ff4-spike-qwen3-magiceffect_25: src/spike_magiceffect_25_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_25_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_27
ff4-spike-qwen3-magiceffect_27: src/spike_magiceffect_27_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_27_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_28
ff4-spike-qwen3-magiceffect_28: src/spike_magiceffect_28_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_28_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_29
ff4-spike-qwen3-magiceffect_29: src/spike_magiceffect_29_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_29_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_2a
ff4-spike-qwen3-magiceffect_2a: src/spike_magiceffect_2a_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2a_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_2b
ff4-spike-qwen3-magiceffect_2b: src/spike_magiceffect_2b_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2b_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_2c
ff4-spike-qwen3-magiceffect_2c: src/spike_magiceffect_2c_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2c_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_2f
ff4-spike-qwen3-magiceffect_2f: src/spike_magiceffect_2f_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2f_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for MagicEffect_32
ff4-spike-qwen3-magiceffect_32: src/spike_magiceffect_32_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_32_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Max255
ff4-spike-qwen3-max255: src/spike_max255_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_max255_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Max99
ff4-spike-qwen3-max99: src/spike_max99_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_max99_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Mult16
ff4-spike-qwen3-mult16: src/spike_mult16_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult16_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Mult8
ff4-spike-qwen3-mult8: src/spike_mult8_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult8_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for NextObj
ff4-spike-qwen3-nextobj: src/spike_nextobj_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nextobj_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for NoFanfareTbl
ff4-spike-qwen3-nofanfaretbl: src/spike_nofanfaretbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nofanfaretbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for NoMonsterFlashTbl
ff4-spike-qwen3-nomonsterflashtbl: src/spike_nomonsterflashtbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nomonsterflashtbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for NoNameAttackTbl
ff4-spike-qwen3-nonameattacktbl: src/spike_nonameattacktbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nonameattacktbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for PlayBattleSong
ff4-spike-qwen3-playbattlesong: src/spike_playbattlesong_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playbattlesong_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Rand
ff4-spike-qwen3-rand: src/spike_rand_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_rand_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Rand99
ff4-spike-qwen3-rand99: src/spike_rand99_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_rand99_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for RandChar
ff4-spike-qwen3-randchar: src/spike_randchar_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_randchar_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for RandMonster
ff4-spike-qwen3-randmonster: src/spike_randmonster_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_randmonster_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for RemoveTarget
ff4-spike-qwen3-removetarget: src/spike_removetarget_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_removetarget_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for RestoreHP
ff4-spike-qwen3-restorehp: src/spike_restorehp_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_restorehp_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for SetBit
ff4-spike-qwen3-setbit: src/spike_setbit_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_setbit_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for SetMagicStatus
ff4-spike-qwen3-setmagicstatus: src/spike_setmagicstatus_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_setmagicstatus_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for SetTargets
ff4-spike-qwen3-settargets: src/spike_settargets_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_settargets_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for SetTimerDur
ff4-spike-qwen3-settimerdur: src/spike_settimerdur_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_settimerdur_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for SpellListTbl
ff4-spike-qwen3-spelllisttbl: src/spike_spelllisttbl_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_spelllisttbl_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for Sub16
ff4-spike-qwen3-sub16: src/spike_sub16_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_sub16_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TargetMonster
ff4-spike-qwen3-targetmonster: src/spike_targetmonster_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_targetmonster_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TimerDur_00
ff4-spike-qwen3-timerdur_00: src/spike_timerdur_00_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_00_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TimerDur_02
ff4-spike-qwen3-timerdur_02: src/spike_timerdur_02_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_02_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TimerDur_03
ff4-spike-qwen3-timerdur_03: src/spike_timerdur_03_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_03_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TimerDur_05
ff4-spike-qwen3-timerdur_05: src/spike_timerdur_05_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_05_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TimerDur_07
ff4-spike-qwen3-timerdur_07: src/spike_timerdur_07_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_07_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TimerDur_08
ff4-spike-qwen3-timerdur_08: src/spike_timerdur_08_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_08_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TimerDur_09
ff4-spike-qwen3-timerdur_09: src/spike_timerdur_09_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_09_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TimerDur_0a
ff4-spike-qwen3-timerdur_0a: src/spike_timerdur_0a_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_0a_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TimerDur_0b
ff4-spike-qwen3-timerdur_0b: src/spike_timerdur_0b_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_0b_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for TwinFailed
ff4-spike-qwen3-twinfailed: src/spike_twinfailed_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_twinfailed_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for UpdateEquip_far
ff4-spike-qwen3-updateequip_far: src/spike_updateequip_far_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updateequip_far_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for ValidateAITarget
ff4-spike-qwen3-validateaitarget: src/spike_validateaitarget_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_validateaitarget_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3): spike for ValidateChar
ff4-spike-qwen3-validatechar: src/spike_validatechar_qwen3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_validatechar_qwen3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CheckCanUseItem
ff4-spike-gemma4-checkcanuseitem: src/spike_checkcanuseitem_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkcanuseitem_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CheckFanfare
ff4-spike-gemma4-checkfanfare: src/spike_checkfanfare_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkfanfare_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CheckMonsterFlash
ff4-spike-gemma4-checkmonsterflash: src/spike_checkmonsterflash_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkmonsterflash_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CheckRegen
ff4-spike-gemma4-checkregen: src/spike_checkregen_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkregen_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CheckStrongElem
ff4-spike-gemma4-checkstrongelem: src/spike_checkstrongelem_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkstrongelem_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CheckTimer
ff4-spike-gemma4-checktimer: src/spike_checktimer_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktimer_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CheckWeakElem
ff4-spike-gemma4-checkweakelem: src/spike_checkweakelem_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkweakelem_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CheckWinAnim
ff4-spike-gemma4-checkwinanim: src/spike_checkwinanim_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkwinanim_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Cmd_01
ff4-spike-gemma4-cmd_01: src/spike_cmd_01_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_01_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Cmd_08
ff4-spike-gemma4-cmd_08: src/spike_cmd_08_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_08_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Cmd_0c
ff4-spike-gemma4-cmd_0c: src/spike_cmd_0c_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0c_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Cmd_0e
ff4-spike-gemma4-cmd_0e: src/spike_cmd_0e_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0e_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Cmd_0f
ff4-spike-gemma4-cmd_0f: src/spike_cmd_0f_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0f_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Cmd_22
ff4-spike-gemma4-cmd_22: src/spike_cmd_22_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_22_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for CountCharTargets
ff4-spike-gemma4-countchartargets: src/spike_countchartargets_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_countchartargets_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for DoMagicEffect
ff4-spike-gemma4-domagiceffect: src/spike_domagiceffect_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_domagiceffect_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for DrawStolenItem
ff4-spike-gemma4-drawstolenitem: src/spike_drawstolenitem_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawstolenitem_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for ExecBtlGfx
ff4-spike-gemma4-execbtlgfx: src/spike_execbtlgfx_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execbtlgfx_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for ExecCmd
ff4-spike-gemma4-execcmd: src/spike_execcmd_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execcmd_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for FindValidChar
ff4-spike-gemma4-findvalidchar: src/spike_findvalidchar_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_findvalidchar_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for GetAntiElem
ff4-spike-gemma4-getantielem: src/spike_getantielem_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getantielem_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for GetBackRowChars
ff4-spike-gemma4-getbackrowchars: src/spike_getbackrowchars_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbackrowchars_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for GetDmgPtr
ff4-spike-gemma4-getdmgptr: src/spike_getdmgptr_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdmgptr_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for GetFrontRowChars
ff4-spike-gemma4-getfrontrowchars: src/spike_getfrontrowchars_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getfrontrowchars_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for GetObjPtr
ff4-spike-gemma4-getobjptr: src/spike_getobjptr_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getobjptr_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for GetSummonAttack
ff4-spike-gemma4-getsummonattack: src/spike_getsummonattack_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getsummonattack_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for GetTimerPtr
ff4-spike-gemma4-gettimerptr: src/spike_gettimerptr_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettimerptr_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for InitAction
ff4-spike-gemma4-initaction: src/spike_initaction_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initaction_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for InitCharRows
ff4-spike-gemma4-initcharrows: src/spike_initcharrows_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initcharrows_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicDmgEffect
ff4-spike-gemma4-magicdmgeffect: src/spike_magicdmgeffect_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magicdmgeffect_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_03
ff4-spike-gemma4-magiceffect_03: src/spike_magiceffect_03_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_03_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_04
ff4-spike-gemma4-magiceffect_04: src/spike_magiceffect_04_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_04_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_08
ff4-spike-gemma4-magiceffect_08: src/spike_magiceffect_08_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_08_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_0c
ff4-spike-gemma4-magiceffect_0c: src/spike_magiceffect_0c_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0c_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_0d
ff4-spike-gemma4-magiceffect_0d: src/spike_magiceffect_0d_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0d_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_0e
ff4-spike-gemma4-magiceffect_0e: src/spike_magiceffect_0e_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0e_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_0f
ff4-spike-gemma4-magiceffect_0f: src/spike_magiceffect_0f_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0f_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_15
ff4-spike-gemma4-magiceffect_15: src/spike_magiceffect_15_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_15_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_16
ff4-spike-gemma4-magiceffect_16: src/spike_magiceffect_16_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_16_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_20
ff4-spike-gemma4-magiceffect_20: src/spike_magiceffect_20_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_20_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_22
ff4-spike-gemma4-magiceffect_22: src/spike_magiceffect_22_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_22_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_23
ff4-spike-gemma4-magiceffect_23: src/spike_magiceffect_23_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_23_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_24
ff4-spike-gemma4-magiceffect_24: src/spike_magiceffect_24_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_24_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_25
ff4-spike-gemma4-magiceffect_25: src/spike_magiceffect_25_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_25_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_27
ff4-spike-gemma4-magiceffect_27: src/spike_magiceffect_27_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_27_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_29
ff4-spike-gemma4-magiceffect_29: src/spike_magiceffect_29_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_29_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_2a
ff4-spike-gemma4-magiceffect_2a: src/spike_magiceffect_2a_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2a_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_2c
ff4-spike-gemma4-magiceffect_2c: src/spike_magiceffect_2c_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2c_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_2f
ff4-spike-gemma4-magiceffect_2f: src/spike_magiceffect_2f_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2f_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for MagicEffect_32
ff4-spike-gemma4-magiceffect_32: src/spike_magiceffect_32_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_32_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Mult8
ff4-spike-gemma4-mult8: src/spike_mult8_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult8_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for NextObj
ff4-spike-gemma4-nextobj: src/spike_nextobj_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nextobj_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for NoFanfareTbl
ff4-spike-gemma4-nofanfaretbl: src/spike_nofanfaretbl_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nofanfaretbl_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for NoUpdateBattleTbl
ff4-spike-gemma4-noupdatebattletbl: src/spike_noupdatebattletbl_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_noupdatebattletbl_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for PlayBattleSong
ff4-spike-gemma4-playbattlesong: src/spike_playbattlesong_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playbattlesong_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for RandChar
ff4-spike-gemma4-randchar: src/spike_randchar_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_randchar_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for RandMonster
ff4-spike-gemma4-randmonster: src/spike_randmonster_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_randmonster_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for RemoveTarget
ff4-spike-gemma4-removetarget: src/spike_removetarget_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_removetarget_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for SetTargets
ff4-spike-gemma4-settargets: src/spike_settargets_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_settargets_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for SetTimerDur
ff4-spike-gemma4-settimerdur: src/spike_settimerdur_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_settimerdur_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for Sub16
ff4-spike-gemma4-sub16: src/spike_sub16_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_sub16_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for TargetMonster
ff4-spike-gemma4-targetmonster: src/spike_targetmonster_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_targetmonster_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for TimerDur_02
ff4-spike-gemma4-timerdur_02: src/spike_timerdur_02_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_02_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for TimerDur_08
ff4-spike-gemma4-timerdur_08: src/spike_timerdur_08_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_08_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for TimerDur_0a
ff4-spike-gemma4-timerdur_0a: src/spike_timerdur_0a_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_0a_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for TwinFailed
ff4-spike-gemma4-twinfailed: src/spike_twinfailed_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_twinfailed_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for UpdateEquip_far
ff4-spike-gemma4-updateequip_far: src/spike_updateequip_far_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updateequip_far_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4): spike for ValidateChar
ff4-spike-gemma4-validatechar: src/spike_validatechar_gemma4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_validatechar_gemma4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_retry): spike for AICond_09
ff4-spike-qwen3_retry-aicond_09: src/spike_aicond_09_qwen3_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_09_qwen3_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_retry): spike for AICondTbl
ff4-spike-qwen3_retry-aicondtbl: src/spike_aicondtbl_qwen3_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtbl_qwen3_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_retry): spike for AICond_00
ff4-spike-gemma4_retry-aicond_00: src/spike_aicond_00_gemma4_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_00_gemma4_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_retry2): spike for AICond_09
ff4-spike-qwen3_retry2-aicond_09: src/spike_aicond_09_qwen3_retry2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_09_qwen3_retry2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_retry2): spike for AICond_00
ff4-spike-gemma4_retry2-aicond_00: src/spike_aicond_00_gemma4_retry2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_00_gemma4_retry2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICondTarget_23
ff4-spike-qwen3_after_retry-aicondtarget_23: src/spike_aicondtarget_23_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_23_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICondTarget_25
ff4-spike-qwen3_after_retry-aicondtarget_25: src/spike_aicondtarget_25_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICondTarget_26
ff4-spike-qwen3_after_retry-aicondtarget_26: src/spike_aicondtarget_26_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_26_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICondTarget_27
ff4-spike-qwen3_after_retry-aicondtarget_27: src/spike_aicondtarget_27_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_27_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICondTbl
ff4-spike-qwen3_after_retry-aicondtbl: src/spike_aicondtbl_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtbl_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICondTarget_25
ff4-spike-gemma4_after_retry-aicondtarget_25: src/spike_aicondtarget_25_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICond_00
ff4-spike-qwen3_after_retry-aicond_00: src/spike_aicond_00_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_00_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICondTarget_26
ff4-spike-gemma4_after_retry-aicondtarget_26: src/spike_aicondtarget_26_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_26_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICondTarget_27
ff4-spike-gemma4_after_retry-aicondtarget_27: src/spike_aicondtarget_27_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_27_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICond_00
ff4-spike-gemma4_after_retry-aicond_00: src/spike_aicond_00_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_00_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICond_02
ff4-spike-qwen3_after_retry-aicond_02: src/spike_aicond_02_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICond_03
ff4-spike-qwen3_after_retry-aicond_03: src/spike_aicond_03_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_03_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICond_02
ff4-spike-gemma4_after_retry-aicond_02: src/spike_aicond_02_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICond_03
ff4-spike-gemma4_after_retry-aicond_03: src/spike_aicond_03_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_03_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICond_04
ff4-spike-qwen3_after_retry-aicond_04: src/spike_aicond_04_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_04_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICond_05
ff4-spike-qwen3_after_retry-aicond_05: src/spike_aicond_05_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICond_06
ff4-spike-qwen3_after_retry-aicond_06: src/spike_aicond_06_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_06_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICond_09
ff4-spike-qwen3_after_retry-aicond_09: src/spike_aicond_09_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_09_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICond_04
ff4-spike-gemma4_after_retry-aicond_04: src/spike_aicond_04_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_04_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICond_0a
ff4-spike-qwen3_after_retry-aicond_0a: src/spike_aicond_0a_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_0a_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICond_05
ff4-spike-gemma4_after_retry-aicond_05: src/spike_aicond_05_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AICond_0b
ff4-spike-qwen3_after_retry-aicond_0b: src/spike_aicond_0b_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_0b_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICond_06
ff4-spike-gemma4_after_retry-aicond_06: src/spike_aicond_06_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_06_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITargetTbl
ff4-spike-qwen3_after_retry-aitargettbl: src/spike_aitargettbl_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitargettbl_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICond_09
ff4-spike-gemma4_after_retry-aicond_09: src/spike_aicond_09_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_09_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_16
ff4-spike-qwen3_after_retry-aitarget_16: src/spike_aitarget_16_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_16_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICond_0a
ff4-spike-gemma4_after_retry-aicond_0a: src/spike_aicond_0a_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_0a_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_19
ff4-spike-qwen3_after_retry-aitarget_19: src/spike_aitarget_19_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_19_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_1a
ff4-spike-qwen3_after_retry-aitarget_1a: src/spike_aitarget_1a_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1a_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AICond_0b
ff4-spike-gemma4_after_retry-aicond_0b: src/spike_aicond_0b_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_0b_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_1b
ff4-spike-qwen3_after_retry-aitarget_1b: src/spike_aitarget_1b_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1b_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_1e
ff4-spike-qwen3_after_retry-aitarget_1e: src/spike_aitarget_1e_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1e_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AITarget_1e
ff4-spike-gemma4_after_retry-aitarget_1e: src/spike_aitarget_1e_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1e_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_1f
ff4-spike-qwen3_after_retry-aitarget_1f: src/spike_aitarget_1f_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1f_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AITarget_1f
ff4-spike-gemma4_after_retry-aitarget_1f: src/spike_aitarget_1f_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_1f_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_20
ff4-spike-qwen3_after_retry-aitarget_20: src/spike_aitarget_20_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_20_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AITarget_22
ff4-spike-gemma4_after_retry-aitarget_22: src/spike_aitarget_22_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_22_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AITarget_29
ff4-spike-gemma4_after_retry-aitarget_29: src/spike_aitarget_29_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_29_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_21
ff4-spike-qwen3_after_retry-aitarget_21: src/spike_aitarget_21_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_21_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Add16
ff4-spike-gemma4_after_retry-add16: src/spike_add16_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_add16_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AddMsg1
ff4-spike-gemma4_after_retry-addmsg1: src/spike_addmsg1_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg1_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_22
ff4-spike-qwen3_after_retry-aitarget_22: src/spike_aitarget_22_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_22_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_23
ff4-spike-qwen3_after_retry-aitarget_23: src/spike_aitarget_23_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_23_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AddMsg2
ff4-spike-gemma4_after_retry-addmsg2: src/spike_addmsg2_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg2_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_24
ff4-spike-qwen3_after_retry-aitarget_24: src/spike_aitarget_24_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_24_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AddMsg3
ff4-spike-gemma4_after_retry-addmsg3: src/spike_addmsg3_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg3_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_25
ff4-spike-qwen3_after_retry-aitarget_25: src/spike_aitarget_25_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_25_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for ApplyDmgMult
ff4-spike-gemma4_after_retry-applydmgmult: src/spike_applydmgmult_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_applydmgmult_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AITarget_29
ff4-spike-qwen3_after_retry-aitarget_29: src/spike_aitarget_29_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_29_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for ApplySpeedMod
ff4-spike-gemma4_after_retry-applyspeedmod: src/spike_applyspeedmod_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_applyspeedmod_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Asl_6
ff4-spike-gemma4_after_retry-asl_6: src/spike_asl_6_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_asl_6_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AutoBattlePtrs
ff4-spike-gemma4_after_retry-autobattleptrs: src/spike_autobattleptrs_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_autobattleptrs_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for AutoBattle_0001
ff4-spike-gemma4_after_retry-autobattle_0001: src/spike_autobattle_0001_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_autobattle_0001_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Add16
ff4-spike-qwen3_after_retry-add16: src/spike_add16_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_add16_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AddMsg1
ff4-spike-qwen3_after_retry-addmsg1: src/spike_addmsg1_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg1_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AddMsg2
ff4-spike-qwen3_after_retry-addmsg2: src/spike_addmsg2_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg2_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AddMsg3
ff4-spike-qwen3_after_retry-addmsg3: src/spike_addmsg3_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_addmsg3_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AnyTarget
ff4-spike-qwen3_after_retry-anytarget: src/spike_anytarget_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_anytarget_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for ApplyDmgMult
ff4-spike-qwen3_after_retry-applydmgmult: src/spike_applydmgmult_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_applydmgmult_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for ApplySpeedMod
ff4-spike-qwen3_after_retry-applyspeedmod: src/spike_applyspeedmod_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_applyspeedmod_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Asl_6
ff4-spike-qwen3_after_retry-asl_6: src/spike_asl_6_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_asl_6_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AutoBattlePtrs
ff4-spike-qwen3_after_retry-autobattleptrs: src/spike_autobattleptrs_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_autobattleptrs_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for AutoBattle_0003
ff4-spike-qwen3_after_retry-autobattle_0003: src/spike_autobattle_0003_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_autobattle_0003_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CalcHits
ff4-spike-gemma4_after_retry-calchits: src/spike_calchits_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calchits_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CheckAICond
ff4-spike-gemma4_after_retry-checkaicond: src/spike_checkaicond_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkaicond_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CheckBattleList
ff4-spike-gemma4_after_retry-checkbattlelist: src/spike_checkbattlelist_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkbattlelist_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CalcHits
ff4-spike-qwen3_after_retry-calchits: src/spike_calchits_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calchits_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CalcTimerDur
ff4-spike-qwen3_after_retry-calctimerdur: src/spike_calctimerdur_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calctimerdur_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CheckCanUseItem
ff4-spike-gemma4_after_retry-checkcanuseitem: src/spike_checkcanuseitem_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkcanuseitem_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CheckFanfare
ff4-spike-gemma4_after_retry-checkfanfare: src/spike_checkfanfare_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkfanfare_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CheckMonsterFlash
ff4-spike-gemma4_after_retry-checkmonsterflash: src/spike_checkmonsterflash_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkmonsterflash_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CheckRegen
ff4-spike-gemma4_after_retry-checkregen: src/spike_checkregen_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkregen_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckAICond
ff4-spike-qwen3_after_retry-checkaicond: src/spike_checkaicond_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkaicond_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CheckStrongElem
ff4-spike-gemma4_after_retry-checkstrongelem: src/spike_checkstrongelem_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkstrongelem_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CheckTimer
ff4-spike-gemma4_after_retry-checktimer: src/spike_checktimer_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktimer_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CheckWeakElem
ff4-spike-gemma4_after_retry-checkweakelem: src/spike_checkweakelem_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkweakelem_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CheckWinAnim
ff4-spike-gemma4_after_retry-checkwinanim: src/spike_checkwinanim_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkwinanim_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Cmd_01
ff4-spike-gemma4_after_retry-cmd_01: src/spike_cmd_01_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_01_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckAttackName
ff4-spike-qwen3_after_retry-checkattackname: src/spike_checkattackname_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkattackname_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Cmd_08
ff4-spike-gemma4_after_retry-cmd_08: src/spike_cmd_08_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_08_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckBattleList
ff4-spike-qwen3_after_retry-checkbattlelist: src/spike_checkbattlelist_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkbattlelist_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Cmd_0c
ff4-spike-gemma4_after_retry-cmd_0c: src/spike_cmd_0c_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0c_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckBit
ff4-spike-qwen3_after_retry-checkbit: src/spike_checkbit_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkbit_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Cmd_0e
ff4-spike-gemma4_after_retry-cmd_0e: src/spike_cmd_0e_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0e_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckCanUseItem
ff4-spike-qwen3_after_retry-checkcanuseitem: src/spike_checkcanuseitem_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkcanuseitem_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckFanfare
ff4-spike-qwen3_after_retry-checkfanfare: src/spike_checkfanfare_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkfanfare_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Cmd_0f
ff4-spike-gemma4_after_retry-cmd_0f: src/spike_cmd_0f_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0f_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Cmd_21
ff4-spike-gemma4_after_retry-cmd_21: src/spike_cmd_21_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_21_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Cmd_22
ff4-spike-gemma4_after_retry-cmd_22: src/spike_cmd_22_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_22_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CountBits
ff4-spike-gemma4_after_retry-countbits: src/spike_countbits_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_countbits_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for CountCharTargets
ff4-spike-gemma4_after_retry-countchartargets: src/spike_countchartargets_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_countchartargets_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for DrawMP
ff4-spike-gemma4_after_retry-drawmp: src/spike_drawmp_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawmp_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for DrawStolenItem
ff4-spike-gemma4_after_retry-drawstolenitem: src/spike_drawstolenitem_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawstolenitem_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for EndMultiAttack
ff4-spike-gemma4_after_retry-endmultiattack: src/spike_endmultiattack_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endmultiattack_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckMonsterFlash
ff4-spike-qwen3_after_retry-checkmonsterflash: src/spike_checkmonsterflash_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkmonsterflash_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckRegen
ff4-spike-qwen3_after_retry-checkregen: src/spike_checkregen_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkregen_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckStrongElem
ff4-spike-qwen3_after_retry-checkstrongelem: src/spike_checkstrongelem_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkstrongelem_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckTimer
ff4-spike-qwen3_after_retry-checktimer: src/spike_checktimer_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktimer_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckWeakElem
ff4-spike-qwen3_after_retry-checkweakelem: src/spike_checkweakelem_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkweakelem_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CheckWinAnim
ff4-spike-qwen3_after_retry-checkwinanim: src/spike_checkwinanim_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkwinanim_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for ClearBit
ff4-spike-qwen3_after_retry-clearbit: src/spike_clearbit_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearbit_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for ClearInvalidTargets
ff4-spike-qwen3_after_retry-clearinvalidtargets: src/spike_clearinvalidtargets_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearinvalidtargets_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CmdDelayTbl
ff4-spike-qwen3_after_retry-cmddelaytbl: src/spike_cmddelaytbl_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmddelaytbl_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CmdTargetTbl
ff4-spike-qwen3_after_retry-cmdtargettbl: src/spike_cmdtargettbl_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmdtargettbl_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for ExecBtlGfx
ff4-spike-gemma4_after_retry-execbtlgfx: src/spike_execbtlgfx_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execbtlgfx_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for ExecCmd
ff4-spike-gemma4_after_retry-execcmd: src/spike_execcmd_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execcmd_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CmdTbl
ff4-spike-qwen3_after_retry-cmdtbl: src/spike_cmdtbl_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmdtbl_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Cmd_01
ff4-spike-qwen3_after_retry-cmd_01: src/spike_cmd_01_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_01_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Cmd_08
ff4-spike-qwen3_after_retry-cmd_08: src/spike_cmd_08_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_08_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Cmd_0c
ff4-spike-qwen3_after_retry-cmd_0c: src/spike_cmd_0c_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0c_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Cmd_0e
ff4-spike-qwen3_after_retry-cmd_0e: src/spike_cmd_0e_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0e_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Cmd_0f
ff4-spike-qwen3_after_retry-cmd_0f: src/spike_cmd_0f_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_0f_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for FindValidChar
ff4-spike-gemma4_after_retry-findvalidchar: src/spike_findvalidchar_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_findvalidchar_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Cmd_21
ff4-spike-qwen3_after_retry-cmd_21: src/spike_cmd_21_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_21_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for GetAntiElem
ff4-spike-gemma4_after_retry-getantielem: src/spike_getantielem_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getantielem_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Cmd_22
ff4-spike-qwen3_after_retry-cmd_22: src/spike_cmd_22_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmd_22_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for GetBackRowChars
ff4-spike-gemma4_after_retry-getbackrowchars: src/spike_getbackrowchars_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbackrowchars_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CountBits
ff4-spike-qwen3_after_retry-countbits: src/spike_countbits_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_countbits_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for GetDmgPtr
ff4-spike-gemma4_after_retry-getdmgptr: src/spike_getdmgptr_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdmgptr_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for CountCharTargets
ff4-spike-qwen3_after_retry-countchartargets: src/spike_countchartargets_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_countchartargets_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for GetFrontRowChars
ff4-spike-gemma4_after_retry-getfrontrowchars: src/spike_getfrontrowchars_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getfrontrowchars_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for DisableCmdStatusTbl
ff4-spike-qwen3_after_retry-disablecmdstatustbl: src/spike_disablecmdstatustbl_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_disablecmdstatustbl_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for GetObjPtr
ff4-spike-gemma4_after_retry-getobjptr: src/spike_getobjptr_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getobjptr_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Div16
ff4-spike-qwen3_after_retry-div16: src/spike_div16_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_div16_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for GetSummonAttack
ff4-spike-gemma4_after_retry-getsummonattack: src/spike_getsummonattack_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getsummonattack_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for DoAction
ff4-spike-qwen3_after_retry-doaction: src/spike_doaction_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_doaction_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for GetTimerPtr
ff4-spike-gemma4_after_retry-gettimerptr: src/spike_gettimerptr_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettimerptr_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for DrawMP
ff4-spike-qwen3_after_retry-drawmp: src/spike_drawmp_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawmp_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for InitAction
ff4-spike-gemma4_after_retry-initaction: src/spike_initaction_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initaction_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for DrawStolenItem
ff4-spike-qwen3_after_retry-drawstolenitem: src/spike_drawstolenitem_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawstolenitem_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for InitCharRows
ff4-spike-gemma4_after_retry-initcharrows: src/spike_initcharrows_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initcharrows_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for EndMultiAttack
ff4-spike-qwen3_after_retry-endmultiattack: src/spike_endmultiattack_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endmultiattack_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicDmgEffect
ff4-spike-gemma4_after_retry-magicdmgeffect: src/spike_magicdmgeffect_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magicdmgeffect_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_03
ff4-spike-gemma4_after_retry-magiceffect_03: src/spike_magiceffect_03_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_03_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_04
ff4-spike-gemma4_after_retry-magiceffect_04: src/spike_magiceffect_04_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_04_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_08
ff4-spike-gemma4_after_retry-magiceffect_08: src/spike_magiceffect_08_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_08_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_0c
ff4-spike-gemma4_after_retry-magiceffect_0c: src/spike_magiceffect_0c_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0c_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_0d
ff4-spike-gemma4_after_retry-magiceffect_0d: src/spike_magiceffect_0d_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0d_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_0e
ff4-spike-gemma4_after_retry-magiceffect_0e: src/spike_magiceffect_0e_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0e_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_0f
ff4-spike-gemma4_after_retry-magiceffect_0f: src/spike_magiceffect_0f_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0f_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_15
ff4-spike-gemma4_after_retry-magiceffect_15: src/spike_magiceffect_15_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_15_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_16
ff4-spike-gemma4_after_retry-magiceffect_16: src/spike_magiceffect_16_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_16_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_20
ff4-spike-gemma4_after_retry-magiceffect_20: src/spike_magiceffect_20_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_20_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for EquipStatTbl
ff4-spike-qwen3_after_retry-equipstattbl: src/spike_equipstattbl_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_equipstattbl_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_22
ff4-spike-gemma4_after_retry-magiceffect_22: src/spike_magiceffect_22_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_22_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for ExecBattle
ff4-spike-qwen3_after_retry-execbattle: src/spike_execbattle_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execbattle_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_23
ff4-spike-gemma4_after_retry-magiceffect_23: src/spike_magiceffect_23_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_23_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_24
ff4-spike-gemma4_after_retry-magiceffect_24: src/spike_magiceffect_24_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_24_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_25
ff4-spike-gemma4_after_retry-magiceffect_25: src/spike_magiceffect_25_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_25_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for ExecBtlGfx
ff4-spike-qwen3_after_retry-execbtlgfx: src/spike_execbtlgfx_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execbtlgfx_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_27
ff4-spike-gemma4_after_retry-magiceffect_27: src/spike_magiceffect_27_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_27_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_29
ff4-spike-gemma4_after_retry-magiceffect_29: src/spike_magiceffect_29_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_29_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_2a
ff4-spike-gemma4_after_retry-magiceffect_2a: src/spike_magiceffect_2a_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2a_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_2c
ff4-spike-gemma4_after_retry-magiceffect_2c: src/spike_magiceffect_2c_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2c_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_2f
ff4-spike-gemma4_after_retry-magiceffect_2f: src/spike_magiceffect_2f_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2f_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for MagicEffect_32
ff4-spike-gemma4_after_retry-magiceffect_32: src/spike_magiceffect_32_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_32_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Mult8
ff4-spike-gemma4_after_retry-mult8: src/spike_mult8_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult8_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for NextObj
ff4-spike-gemma4_after_retry-nextobj: src/spike_nextobj_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nextobj_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for ExecCmd
ff4-spike-qwen3_after_retry-execcmd: src/spike_execcmd_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execcmd_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for NoFanfareTbl
ff4-spike-gemma4_after_retry-nofanfaretbl: src/spike_nofanfaretbl_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nofanfaretbl_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for FindValidChar
ff4-spike-qwen3_after_retry-findvalidchar: src/spike_findvalidchar_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_findvalidchar_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for NoUpdateBattleTbl
ff4-spike-gemma4_after_retry-noupdatebattletbl: src/spike_noupdatebattletbl_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_noupdatebattletbl_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for GetAntiElem
ff4-spike-qwen3_after_retry-getantielem: src/spike_getantielem_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getantielem_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for PlayBattleSong
ff4-spike-gemma4_after_retry-playbattlesong: src/spike_playbattlesong_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playbattlesong_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for GetBackRowChars
ff4-spike-qwen3_after_retry-getbackrowchars: src/spike_getbackrowchars_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbackrowchars_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for GetDmgPtr
ff4-spike-qwen3_after_retry-getdmgptr: src/spike_getdmgptr_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdmgptr_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for RandChar
ff4-spike-gemma4_after_retry-randchar: src/spike_randchar_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_randchar_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for GetFrontRowChars
ff4-spike-qwen3_after_retry-getfrontrowchars: src/spike_getfrontrowchars_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getfrontrowchars_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for RandMonster
ff4-spike-gemma4_after_retry-randmonster: src/spike_randmonster_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_randmonster_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for GetObjPtr
ff4-spike-qwen3_after_retry-getobjptr: src/spike_getobjptr_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getobjptr_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for RemoveTarget
ff4-spike-gemma4_after_retry-removetarget: src/spike_removetarget_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_removetarget_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for GetPendingAction
ff4-spike-qwen3_after_retry-getpendingaction: src/spike_getpendingaction_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getpendingaction_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for SetTargets
ff4-spike-gemma4_after_retry-settargets: src/spike_settargets_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_settargets_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for GetScriptPtr
ff4-spike-qwen3_after_retry-getscriptptr: src/spike_getscriptptr_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getscriptptr_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for SetTimerDur
ff4-spike-gemma4_after_retry-settimerdur: src/spike_settimerdur_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_settimerdur_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for Sub16
ff4-spike-gemma4_after_retry-sub16: src/spike_sub16_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_sub16_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for TargetMonster
ff4-spike-gemma4_after_retry-targetmonster: src/spike_targetmonster_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_targetmonster_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for TimerDur_02
ff4-spike-gemma4_after_retry-timerdur_02: src/spike_timerdur_02_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_02_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for TimerDur_08
ff4-spike-gemma4_after_retry-timerdur_08: src/spike_timerdur_08_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_08_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for TimerDur_0a
ff4-spike-gemma4_after_retry-timerdur_0a: src/spike_timerdur_0a_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_0a_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for TwinFailed
ff4-spike-gemma4_after_retry-twinfailed: src/spike_twinfailed_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_twinfailed_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for UpdateEquip_far
ff4-spike-gemma4_after_retry-updateequip_far: src/spike_updateequip_far_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updateequip_far_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for GetShieldProp
ff4-spike-qwen3_after_retry-getshieldprop: src/spike_getshieldprop_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getshieldprop_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_after_retry): spike for ValidateChar
ff4-spike-gemma4_after_retry-validatechar: src/spike_validatechar_gemma4_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_validatechar_gemma4_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for GetSummonAttack
ff4-spike-qwen3_after_retry-getsummonattack: src/spike_getsummonattack_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getsummonattack_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for GetTimerPtr
ff4-spike-qwen3_after_retry-gettimerptr: src/spike_gettimerptr_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettimerptr_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for InitAction
ff4-spike-qwen3_after_retry-initaction: src/spike_initaction_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initaction_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for InitCharRows
ff4-spike-qwen3_after_retry-initcharrows: src/spike_initcharrows_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initcharrows_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Lsr_6
ff4-spike-qwen3_after_retry-lsr_6: src/spike_lsr_6_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_lsr_6_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicDmgEffect
ff4-spike-qwen3_after_retry-magicdmgeffect: src/spike_magicdmgeffect_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magicdmgeffect_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_03
ff4-spike-qwen3_after_retry-magiceffect_03: src/spike_magiceffect_03_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_03_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_04
ff4-spike-qwen3_after_retry-magiceffect_04: src/spike_magiceffect_04_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_04_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_08
ff4-spike-qwen3_after_retry-magiceffect_08: src/spike_magiceffect_08_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_08_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_0c
ff4-spike-qwen3_after_retry-magiceffect_0c: src/spike_magiceffect_0c_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0c_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_0d
ff4-spike-qwen3_after_retry-magiceffect_0d: src/spike_magiceffect_0d_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0d_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_0e
ff4-spike-qwen3_after_retry-magiceffect_0e: src/spike_magiceffect_0e_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0e_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_0f
ff4-spike-qwen3_after_retry-magiceffect_0f: src/spike_magiceffect_0f_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_0f_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_13
ff4-spike-qwen3_after_retry-magiceffect_13: src/spike_magiceffect_13_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_13_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_15
ff4-spike-qwen3_after_retry-magiceffect_15: src/spike_magiceffect_15_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_15_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_16
ff4-spike-qwen3_after_retry-magiceffect_16: src/spike_magiceffect_16_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_16_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_20
ff4-spike-qwen3_after_retry-magiceffect_20: src/spike_magiceffect_20_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_20_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_21
ff4-spike-qwen3_after_retry-magiceffect_21: src/spike_magiceffect_21_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_21_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_22
ff4-spike-qwen3_after_retry-magiceffect_22: src/spike_magiceffect_22_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_22_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_23
ff4-spike-qwen3_after_retry-magiceffect_23: src/spike_magiceffect_23_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_23_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_24
ff4-spike-qwen3_after_retry-magiceffect_24: src/spike_magiceffect_24_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_24_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_25
ff4-spike-qwen3_after_retry-magiceffect_25: src/spike_magiceffect_25_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_25_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_27
ff4-spike-qwen3_after_retry-magiceffect_27: src/spike_magiceffect_27_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_27_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_28
ff4-spike-qwen3_after_retry-magiceffect_28: src/spike_magiceffect_28_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_28_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_29
ff4-spike-qwen3_after_retry-magiceffect_29: src/spike_magiceffect_29_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_29_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_2a
ff4-spike-qwen3_after_retry-magiceffect_2a: src/spike_magiceffect_2a_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2a_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_2b
ff4-spike-qwen3_after_retry-magiceffect_2b: src/spike_magiceffect_2b_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2b_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_2c
ff4-spike-qwen3_after_retry-magiceffect_2c: src/spike_magiceffect_2c_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2c_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_2f
ff4-spike-qwen3_after_retry-magiceffect_2f: src/spike_magiceffect_2f_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_2f_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for MagicEffect_32
ff4-spike-qwen3_after_retry-magiceffect_32: src/spike_magiceffect_32_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_32_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Max255
ff4-spike-qwen3_after_retry-max255: src/spike_max255_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_max255_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Max99
ff4-spike-qwen3_after_retry-max99: src/spike_max99_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_max99_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Mult16
ff4-spike-qwen3_after_retry-mult16: src/spike_mult16_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult16_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Mult8
ff4-spike-qwen3_after_retry-mult8: src/spike_mult8_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult8_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for NextObj
ff4-spike-qwen3_after_retry-nextobj: src/spike_nextobj_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nextobj_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for NoFanfareTbl
ff4-spike-qwen3_after_retry-nofanfaretbl: src/spike_nofanfaretbl_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nofanfaretbl_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for NoMonsterFlashTbl
ff4-spike-qwen3_after_retry-nomonsterflashtbl: src/spike_nomonsterflashtbl_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nomonsterflashtbl_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for PlayBattleSong
ff4-spike-qwen3_after_retry-playbattlesong: src/spike_playbattlesong_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playbattlesong_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Rand
ff4-spike-qwen3_after_retry-rand: src/spike_rand_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_rand_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Rand99
ff4-spike-qwen3_after_retry-rand99: src/spike_rand99_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_rand99_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for RandChar
ff4-spike-qwen3_after_retry-randchar: src/spike_randchar_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_randchar_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for RandMonster
ff4-spike-qwen3_after_retry-randmonster: src/spike_randmonster_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_randmonster_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for RemoveTarget
ff4-spike-qwen3_after_retry-removetarget: src/spike_removetarget_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_removetarget_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for RestoreHP
ff4-spike-qwen3_after_retry-restorehp: src/spike_restorehp_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_restorehp_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for SetBit
ff4-spike-qwen3_after_retry-setbit: src/spike_setbit_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_setbit_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for SetMagicStatus
ff4-spike-qwen3_after_retry-setmagicstatus: src/spike_setmagicstatus_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_setmagicstatus_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for SetTargets
ff4-spike-qwen3_after_retry-settargets: src/spike_settargets_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_settargets_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for SetTimerDur
ff4-spike-qwen3_after_retry-settimerdur: src/spike_settimerdur_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_settimerdur_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for SpellListTbl
ff4-spike-qwen3_after_retry-spelllisttbl: src/spike_spelllisttbl_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_spelllisttbl_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for Sub16
ff4-spike-qwen3_after_retry-sub16: src/spike_sub16_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_sub16_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TargetMonster
ff4-spike-qwen3_after_retry-targetmonster: src/spike_targetmonster_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_targetmonster_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TimerDur_00
ff4-spike-qwen3_after_retry-timerdur_00: src/spike_timerdur_00_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_00_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TimerDur_02
ff4-spike-qwen3_after_retry-timerdur_02: src/spike_timerdur_02_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_02_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TimerDur_03
ff4-spike-qwen3_after_retry-timerdur_03: src/spike_timerdur_03_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_03_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TimerDur_05
ff4-spike-qwen3_after_retry-timerdur_05: src/spike_timerdur_05_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_05_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TimerDur_07
ff4-spike-qwen3_after_retry-timerdur_07: src/spike_timerdur_07_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_07_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TimerDur_08
ff4-spike-qwen3_after_retry-timerdur_08: src/spike_timerdur_08_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_08_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TimerDur_09
ff4-spike-qwen3_after_retry-timerdur_09: src/spike_timerdur_09_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_09_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TimerDur_0a
ff4-spike-qwen3_after_retry-timerdur_0a: src/spike_timerdur_0a_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_0a_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TimerDur_0b
ff4-spike-qwen3_after_retry-timerdur_0b: src/spike_timerdur_0b_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_0b_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for TwinFailed
ff4-spike-qwen3_after_retry-twinfailed: src/spike_twinfailed_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_twinfailed_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for UpdateEquip_far
ff4-spike-qwen3_after_retry-updateequip_far: src/spike_updateequip_far_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updateequip_far_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for ValidateAITarget
ff4-spike-qwen3_after_retry-validateaitarget: src/spike_validateaitarget_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_validateaitarget_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_after_retry): spike for ValidateChar
ff4-spike-qwen3_after_retry-validatechar: src/spike_validatechar_qwen3_after_retry.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_validatechar_qwen3_after_retry.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for CreditsStarsTileTbl
ff4-spike-qwen3_cutscene-creditsstarstiletbl: src/spike_creditsstarstiletbl_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_creditsstarstiletbl_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for DoTextCmd
ff4-spike-qwen3_cutscene-dotextcmd: src/spike_dotextcmd_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_dotextcmd_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for DrawOtherPlanet
ff4-spike-qwen3_cutscene-drawotherplanet: src/spike_drawotherplanet_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawotherplanet_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for EndCredits
ff4-spike-qwen3_cutscene-endcredits: src/spike_endcredits_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endcredits_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for FadeIn
ff4-spike-qwen3_cutscene-fadein: src/spike_fadein_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadein_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for FadeOut
ff4-spike-qwen3_cutscene-fadeout: src/spike_fadeout_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeout_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for GetEarthSpritePos
ff4-spike-qwen3_cutscene-getearthspritepos: src/spike_getearthspritepos_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for GetMoonTile
ff4-spike-qwen3_cutscene-getmoontile: src/spike_getmoontile_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getmoontile_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for GetOtherPlanetTile
ff4-spike-qwen3_cutscene-getotherplanettile: src/spike_getotherplanettile_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for HDMATbl
ff4-spike-qwen3_cutscene-hdmatbl: src/spike_hdmatbl_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_hdmatbl_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for InitStars
ff4-spike-qwen3_cutscene-initstars: src/spike_initstars_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for LoadCreditsStarsGfx
ff4-spike-qwen3_cutscene-loadcreditsstarsgfx: src/spike_loadcreditsstarsgfx_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadcreditsstarsgfx_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for LoadTheEndGfx
ff4-spike-qwen3_cutscene-loadtheendgfx: src/spike_loadtheendgfx_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadtheendgfx_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for MoonTileTbl
ff4-spike-qwen3_cutscene-moontiletbl: src/spike_moontiletbl_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_moontiletbl_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for Mult16_1
ff4-spike-qwen3_cutscene-mult16_1: src/spike_mult16_1_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult16_1_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for NewLine
ff4-spike-qwen3_cutscene-newline: src/spike_newline_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for SolarSystem2
ff4-spike-qwen3_cutscene-solarsystem2: src/spike_solarsystem2_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_solarsystem2_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for UpdateMoon1
ff4-spike-qwen3_cutscene-updatemoon1: src/spike_updatemoon1_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatemoon1_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for UpdateMoon2
ff4-spike-qwen3_cutscene-updatemoon2: src/spike_updatemoon2_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatemoon2_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for UpdateSun
ff4-spike-qwen3_cutscene-updatesun: src/spike_updatesun_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatesun_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for WaitVBlank
ff4-spike-qwen3_cutscene-waitvblank: src/spike_waitvblank_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblank_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13d87f
ff4-spike-qwen3_cutscene-_13d87f: src/spike__13d87f_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d87f_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13d89a
ff4-spike-qwen3_cutscene-_13d89a: src/spike__13d89a_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d89a_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13d92d
ff4-spike-qwen3_cutscene-_13d92d: src/spike__13d92d_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d92d_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13d977
ff4-spike-qwen3_cutscene-_13d977: src/spike__13d977_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d977_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13db10
ff4-spike-qwen3_cutscene-_13db10: src/spike__13db10_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13db10_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13db23
ff4-spike-qwen3_cutscene-_13db23: src/spike__13db23_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13db23_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13ddd6
ff4-spike-qwen3_cutscene-_13ddd6: src/spike__13ddd6_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ddd6_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13df4d
ff4-spike-qwen3_cutscene-_13df4d: src/spike__13df4d_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13df4d_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13e058
ff4-spike-qwen3_cutscene-_13e058: src/spike__13e058_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e058_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13e07d
ff4-spike-qwen3_cutscene-_13e07d: src/spike__13e07d_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e07d_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13e4f2
ff4-spike-qwen3_cutscene-_13e4f2: src/spike__13e4f2_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e4f2_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13e565
ff4-spike-qwen3_cutscene-_13e565: src/spike__13e565_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e565_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13e578
ff4-spike-qwen3_cutscene-_13e578: src/spike__13e578_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e578_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13e58b
ff4-spike-qwen3_cutscene-_13e58b: src/spike__13e58b_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e58b_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13eb60
ff4-spike-qwen3_cutscene-_13eb60: src/spike__13eb60_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13eb60_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13ebb8
ff4-spike-qwen3_cutscene-_13ebb8: src/spike__13ebb8_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ebb8_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13ec24
ff4-spike-qwen3_cutscene-_13ec24: src/spike__13ec24_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ec24_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene): spike for _13ef4c
ff4-spike-qwen3_cutscene-_13ef4c: src/spike__13ef4c_qwen3_cutscene.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ef4c_qwen3_cutscene.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for CreditsStarsTileTbl
ff4-spike-qwen3_cutscene_v2-creditsstarstiletbl: src/spike_creditsstarstiletbl_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_creditsstarstiletbl_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for DoTextCmd
ff4-spike-qwen3_cutscene_v2-dotextcmd: src/spike_dotextcmd_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_dotextcmd_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for DrawOtherPlanet
ff4-spike-qwen3_cutscene_v2-drawotherplanet: src/spike_drawotherplanet_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawotherplanet_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for EndCredits
ff4-spike-qwen3_cutscene_v2-endcredits: src/spike_endcredits_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endcredits_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for FadeIn
ff4-spike-qwen3_cutscene_v2-fadein: src/spike_fadein_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadein_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for FadeOut
ff4-spike-qwen3_cutscene_v2-fadeout: src/spike_fadeout_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeout_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for GetEarthSpritePos
ff4-spike-qwen3_cutscene_v2-getearthspritepos: src/spike_getearthspritepos_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for GetMoonTile
ff4-spike-qwen3_cutscene_v2-getmoontile: src/spike_getmoontile_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getmoontile_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for GetOtherPlanetTile
ff4-spike-qwen3_cutscene_v2-getotherplanettile: src/spike_getotherplanettile_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for HDMATbl
ff4-spike-qwen3_cutscene_v2-hdmatbl: src/spike_hdmatbl_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_hdmatbl_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for InitStars
ff4-spike-qwen3_cutscene_v2-initstars: src/spike_initstars_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for LoadCreditsStarsGfx
ff4-spike-qwen3_cutscene_v2-loadcreditsstarsgfx: src/spike_loadcreditsstarsgfx_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadcreditsstarsgfx_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for LoadTheEndGfx
ff4-spike-qwen3_cutscene_v2-loadtheendgfx: src/spike_loadtheendgfx_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadtheendgfx_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for MoonTileTbl
ff4-spike-qwen3_cutscene_v2-moontiletbl: src/spike_moontiletbl_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_moontiletbl_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for Mult16_1
ff4-spike-qwen3_cutscene_v2-mult16_1: src/spike_mult16_1_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult16_1_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for NewLine
ff4-spike-qwen3_cutscene_v2-newline: src/spike_newline_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for SolarSystem2
ff4-spike-qwen3_cutscene_v2-solarsystem2: src/spike_solarsystem2_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_solarsystem2_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for UpdateMoon1
ff4-spike-qwen3_cutscene_v2-updatemoon1: src/spike_updatemoon1_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatemoon1_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for UpdateMoon2
ff4-spike-qwen3_cutscene_v2-updatemoon2: src/spike_updatemoon2_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatemoon2_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for UpdateSun
ff4-spike-qwen3_cutscene_v2-updatesun: src/spike_updatesun_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatesun_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for WaitVBlank
ff4-spike-qwen3_cutscene_v2-waitvblank: src/spike_waitvblank_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblank_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13d87f
ff4-spike-qwen3_cutscene_v2-_13d87f: src/spike__13d87f_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d87f_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13d89a
ff4-spike-qwen3_cutscene_v2-_13d89a: src/spike__13d89a_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d89a_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13d92d
ff4-spike-qwen3_cutscene_v2-_13d92d: src/spike__13d92d_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d92d_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13d977
ff4-spike-qwen3_cutscene_v2-_13d977: src/spike__13d977_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d977_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13db10
ff4-spike-qwen3_cutscene_v2-_13db10: src/spike__13db10_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13db10_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13db23
ff4-spike-qwen3_cutscene_v2-_13db23: src/spike__13db23_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13db23_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13ddd6
ff4-spike-qwen3_cutscene_v2-_13ddd6: src/spike__13ddd6_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ddd6_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13df4d
ff4-spike-qwen3_cutscene_v2-_13df4d: src/spike__13df4d_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13df4d_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13e058
ff4-spike-qwen3_cutscene_v2-_13e058: src/spike__13e058_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e058_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13e07d
ff4-spike-qwen3_cutscene_v2-_13e07d: src/spike__13e07d_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e07d_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13e4f2
ff4-spike-qwen3_cutscene_v2-_13e4f2: src/spike__13e4f2_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e4f2_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13e565
ff4-spike-qwen3_cutscene_v2-_13e565: src/spike__13e565_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e565_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13e578
ff4-spike-qwen3_cutscene_v2-_13e578: src/spike__13e578_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e578_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13e58b
ff4-spike-qwen3_cutscene_v2-_13e58b: src/spike__13e58b_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e58b_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13eb60
ff4-spike-qwen3_cutscene_v2-_13eb60: src/spike__13eb60_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13eb60_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13ebb8
ff4-spike-qwen3_cutscene_v2-_13ebb8: src/spike__13ebb8_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ebb8_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13ec24
ff4-spike-qwen3_cutscene_v2-_13ec24: src/spike__13ec24_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ec24_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v2): spike for _13ef4c
ff4-spike-qwen3_cutscene_v2-_13ef4c: src/spike__13ef4c_qwen3_cutscene_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ef4c_qwen3_cutscene_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for CreditsStarsTileTbl
ff4-spike-qwen3_cutscene_v3-creditsstarstiletbl: src/spike_creditsstarstiletbl_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_creditsstarstiletbl_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for DoTextCmd
ff4-spike-qwen3_cutscene_v3-dotextcmd: src/spike_dotextcmd_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_dotextcmd_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for DrawOtherPlanet
ff4-spike-qwen3_cutscene_v3-drawotherplanet: src/spike_drawotherplanet_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawotherplanet_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for EndCredits
ff4-spike-qwen3_cutscene_v3-endcredits: src/spike_endcredits_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endcredits_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for FadeIn
ff4-spike-qwen3_cutscene_v3-fadein: src/spike_fadein_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadein_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for FadeOut
ff4-spike-qwen3_cutscene_v3-fadeout: src/spike_fadeout_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeout_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for GetEarthSpritePos
ff4-spike-qwen3_cutscene_v3-getearthspritepos: src/spike_getearthspritepos_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for GetMoonTile
ff4-spike-qwen3_cutscene_v3-getmoontile: src/spike_getmoontile_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getmoontile_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for GetOtherPlanetTile
ff4-spike-qwen3_cutscene_v3-getotherplanettile: src/spike_getotherplanettile_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for HDMATbl
ff4-spike-qwen3_cutscene_v3-hdmatbl: src/spike_hdmatbl_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_hdmatbl_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for InitStars
ff4-spike-qwen3_cutscene_v3-initstars: src/spike_initstars_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for LoadCreditsStarsGfx
ff4-spike-qwen3_cutscene_v3-loadcreditsstarsgfx: src/spike_loadcreditsstarsgfx_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadcreditsstarsgfx_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for LoadTheEndGfx
ff4-spike-qwen3_cutscene_v3-loadtheendgfx: src/spike_loadtheendgfx_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadtheendgfx_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for MoonTileTbl
ff4-spike-qwen3_cutscene_v3-moontiletbl: src/spike_moontiletbl_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_moontiletbl_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for Mult16_1
ff4-spike-qwen3_cutscene_v3-mult16_1: src/spike_mult16_1_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult16_1_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for NewLine
ff4-spike-qwen3_cutscene_v3-newline: src/spike_newline_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for SolarSystem2
ff4-spike-qwen3_cutscene_v3-solarsystem2: src/spike_solarsystem2_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_solarsystem2_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for UpdateMoon1
ff4-spike-qwen3_cutscene_v3-updatemoon1: src/spike_updatemoon1_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatemoon1_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for UpdateMoon2
ff4-spike-qwen3_cutscene_v3-updatemoon2: src/spike_updatemoon2_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatemoon2_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for UpdateSun
ff4-spike-qwen3_cutscene_v3-updatesun: src/spike_updatesun_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatesun_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for WaitVBlank
ff4-spike-qwen3_cutscene_v3-waitvblank: src/spike_waitvblank_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblank_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13d87f
ff4-spike-qwen3_cutscene_v3-_13d87f: src/spike__13d87f_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d87f_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13d89a
ff4-spike-qwen3_cutscene_v3-_13d89a: src/spike__13d89a_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d89a_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13d92d
ff4-spike-qwen3_cutscene_v3-_13d92d: src/spike__13d92d_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d92d_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13d977
ff4-spike-qwen3_cutscene_v3-_13d977: src/spike__13d977_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13d977_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13db10
ff4-spike-qwen3_cutscene_v3-_13db10: src/spike__13db10_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13db10_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13db23
ff4-spike-qwen3_cutscene_v3-_13db23: src/spike__13db23_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13db23_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13ddd6
ff4-spike-qwen3_cutscene_v3-_13ddd6: src/spike__13ddd6_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ddd6_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13df4d
ff4-spike-qwen3_cutscene_v3-_13df4d: src/spike__13df4d_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13df4d_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13e058
ff4-spike-qwen3_cutscene_v3-_13e058: src/spike__13e058_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e058_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13e07d
ff4-spike-qwen3_cutscene_v3-_13e07d: src/spike__13e07d_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e07d_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13e4f2
ff4-spike-qwen3_cutscene_v3-_13e4f2: src/spike__13e4f2_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e4f2_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13e565
ff4-spike-qwen3_cutscene_v3-_13e565: src/spike__13e565_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e565_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13e578
ff4-spike-qwen3_cutscene_v3-_13e578: src/spike__13e578_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e578_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13e58b
ff4-spike-qwen3_cutscene_v3-_13e58b: src/spike__13e58b_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13e58b_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13eb60
ff4-spike-qwen3_cutscene_v3-_13eb60: src/spike__13eb60_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13eb60_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13ebb8
ff4-spike-qwen3_cutscene_v3-_13ebb8: src/spike__13ebb8_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ebb8_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13ec24
ff4-spike-qwen3_cutscene_v3-_13ec24: src/spike__13ec24_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ec24_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_cutscene_v3): spike for _13ef4c
ff4-spike-qwen3_cutscene_v3-_13ef4c: src/spike__13ef4c_qwen3_cutscene_v3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ef4c_qwen3_cutscene_v3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_sound): spike for ExecInterrupt
ff4-spike-qwen3_sound-execinterrupt: src/spike_execinterrupt_qwen3_sound.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execinterrupt_qwen3_sound.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_sound): spike for ExecSound_ext
ff4-spike-qwen3_sound-execsound_ext: src/spike_execsound_ext_qwen3_sound.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execsound_ext_qwen3_sound.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_sound): spike for InitSound_ext
ff4-spike-qwen3_sound-initsound_ext: src/spike_initsound_ext_qwen3_sound.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initsound_ext_qwen3_sound.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_sound): spike for PlayGameSfx
ff4-spike-qwen3_sound-playgamesfx: src/spike_playgamesfx_qwen3_sound.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playgamesfx_qwen3_sound.c $(CORE_SRC) $(LDFLAGS)

# AUTO (qwen3_sound): spike for PlaySystemSfx
ff4-spike-qwen3_sound-playsystemsfx: src/spike_playsystemsfx_qwen3_sound.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsystemsfx_qwen3_sound.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for AfterBattle
ff4-spike-gemma4_field-afterbattle: src/spike_afterbattle_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_afterbattle_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for AfterCutscene
ff4-spike-gemma4_field-aftercutscene: src/spike_aftercutscene_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aftercutscene_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for BoardBkChoco
ff4-spike-gemma4_field-boardbkchoco: src/spike_boardbkchoco_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardbkchoco_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for BoardChoco
ff4-spike-gemma4_field-boardchoco: src/spike_boardchoco_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardchoco_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for BoardShip
ff4-spike-gemma4_field-boardship: src/spike_boardship_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardship_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for BoardWhale
ff4-spike-gemma4_field-boardwhale: src/spike_boardwhale_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardwhale_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for CheckNPCBlock
ff4-spike-gemma4_field-checknpcblock: src/spike_checknpcblock_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checknpcblock_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for CheckNPCMap2
ff4-spike-gemma4_field-checknpcmap2: src/spike_checknpcmap2_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checknpcmap2_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for CheckNPCSwitch
ff4-spike-gemma4_field-checknpcswitch: src/spike_checknpcswitch_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checknpcswitch_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for CheckTilePass
ff4-spike-gemma4_field-checktilepass: src/spike_checktilepass_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktilepass_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for ChocoSpriteTbl
ff4-spike-gemma4_field-chocospritetbl: src/spike_chocospritetbl_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_chocospritetbl_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for ChooseRandBattle
ff4-spike-gemma4_field-chooserandbattle: src/spike_chooserandbattle_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_chooserandbattle_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for ClearPlayerNPCMap
ff4-spike-gemma4_field-clearplayernpcmap: src/spike_clearplayernpcmap_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearplayernpcmap_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for ClearPrologueVRAM
ff4-spike-gemma4_field-clearprologuevram: src/spike_clearprologuevram_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearprologuevram_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for CloseDlgWindow
ff4-spike-gemma4_field-closedlgwindow: src/spike_closedlgwindow_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closedlgwindow_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for CloseYesNoWindow
ff4-spike-gemma4_field-closeyesnowindow: src/spike_closeyesnowindow_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closeyesnowindow_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for DecBrightness
ff4-spike-gemma4_field-decbrightness: src/spike_decbrightness_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_decbrightness_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for DlgTilesBtm
ff4-spike-gemma4_field-dlgtilesbtm: src/spike_dlgtilesbtm_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_dlgtilesbtm_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for DlgTilesMid
ff4-spike-gemma4_field-dlgtilesmid: src/spike_dlgtilesmid_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_dlgtilesmid_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for DlgTilesTop
ff4-spike-gemma4_field-dlgtilestop: src/spike_dlgtilestop_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_dlgtilestop_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for DrawItemCursor
ff4-spike-gemma4_field-drawitemcursor: src/spike_drawitemcursor_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawitemcursor_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EnterWhale
ff4-spike-gemma4_field-enterwhale: src/spike_enterwhale_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_enterwhale_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EventCmd_d0
ff4-spike-gemma4_field-eventcmd_d0: src/spike_eventcmd_d0_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d0_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EventCmd_d1
ff4-spike-gemma4_field-eventcmd_d1: src/spike_eventcmd_d1_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d1_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EventCmd_d2
ff4-spike-gemma4_field-eventcmd_d2: src/spike_eventcmd_d2_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d2_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EventCmd_d7
ff4-spike-gemma4_field-eventcmd_d7: src/spike_eventcmd_d7_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d7_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EventCmd_d8
ff4-spike-gemma4_field-eventcmd_d8: src/spike_eventcmd_d8_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d8_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EventCmd_dd
ff4-spike-gemma4_field-eventcmd_dd: src/spike_eventcmd_dd_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_dd_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EventCmd_e0
ff4-spike-gemma4_field-eventcmd_e0: src/spike_eventcmd_e0_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_e0_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EventCmd_e6
ff4-spike-gemma4_field-eventcmd_e6: src/spike_eventcmd_e6_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_e6_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EventCmd_ea
ff4-spike-gemma4_field-eventcmd_ea: src/spike_eventcmd_ea_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_ea_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for EventCmd_f9
ff4-spike-gemma4_field-eventcmd_f9: src/spike_eventcmd_f9_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_f9_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for ExecDMA
ff4-spike-gemma4_field-execdma: src/spike_execdma_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execdma_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for ExplosionPosTbl
ff4-spike-gemma4_field-explosionpostbl: src/spike_explosionpostbl_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_explosionpostbl_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for FadeInSong
ff4-spike-gemma4_field-fadeinsong: src/spike_fadeinsong_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeinsong_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for FadeOutSongFast
ff4-spike-gemma4_field-fadeoutsongfast: src/spike_fadeoutsongfast_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeoutsongfast_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for FadeOutSongSlow
ff4-spike-gemma4_field-fadeoutsongslow: src/spike_fadeoutsongslow_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeoutsongslow_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for FieldMain
ff4-spike-gemma4_field-fieldmain: src/spike_fieldmain_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fieldmain_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for FindEventTerminator
ff4-spike-gemma4_field-findeventterminator: src/spike_findeventterminator_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_findeventterminator_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetBattleBG
ff4-spike-gemma4_field-getbattlebg: src/spike_getbattlebg_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbattlebg_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetCurrGil
ff4-spike-gemma4_field-getcurrgil: src/spike_getcurrgil_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getcurrgil_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetDlgID
ff4-spike-gemma4_field-getdlgid: src/spike_getdlgid_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgid_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetDlgPtr0
ff4-spike-gemma4_field-getdlgptr0: src/spike_getdlgptr0_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr0_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetDlgPtr1H
ff4-spike-gemma4_field-getdlgptr1h: src/spike_getdlgptr1h_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetDlgPtr1L
ff4-spike-gemma4_field-getdlgptr1l: src/spike_getdlgptr1l_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1l_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetDlgPtr2
ff4-spike-gemma4_field-getdlgptr2: src/spike_getdlgptr2_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr2_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetDoorTiles
ff4-spike-gemma4_field-getdoortiles: src/spike_getdoortiles_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdoortiles_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetEventSwitchOffset
ff4-spike-gemma4_field-geteventswitchoffset: src/spike_geteventswitchoffset_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_geteventswitchoffset_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetNPCMapPtr
ff4-spike-gemma4_field-getnpcmapptr: src/spike_getnpcmapptr_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcmapptr_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetNPCPtr
ff4-spike-gemma4_field-getnpcptr: src/spike_getnpcptr_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcptr_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetNPCScriptByte
ff4-spike-gemma4_field-getnpcscriptbyte: src/spike_getnpcscriptbyte_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcscriptbyte_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetNextByte
ff4-spike-gemma4_field-getnextbyte: src/spike_getnextbyte_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnextbyte_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetNextEventByte
ff4-spike-gemma4_field-getnexteventbyte: src/spike_getnexteventbyte_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnexteventbyte_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetTileProp
ff4-spike-gemma4_field-gettileprop: src/spike_gettileprop_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettileprop_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetTopCharPtr
ff4-spike-gemma4_field-gettopcharptr: src/spike_gettopcharptr_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettopcharptr_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetTreasurePtr
ff4-spike-gemma4_field-gettreasureptr: src/spike_gettreasureptr_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettreasureptr_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GetTreasureTiles
ff4-spike-gemma4_field-gettreasuretiles: src/spike_gettreasuretiles_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettreasuretiles_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GilWindowTiles1
ff4-spike-gemma4_field-gilwindowtiles1: src/spike_gilwindowtiles1_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gilwindowtiles1_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GilWindowTiles3
ff4-spike-gemma4_field-gilwindowtiles3: src/spike_gilwindowtiles3_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gilwindowtiles3_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GilWindowTiles4
ff4-spike-gemma4_field-gilwindowtiles4: src/spike_gilwindowtiles4_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gilwindowtiles4_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for GiveGil
ff4-spike-gemma4_field-givegil: src/spike_givegil_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_givegil_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for HideMapTitle
ff4-spike-gemma4_field-hidemaptitle: src/spike_hidemaptitle_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_hidemaptitle_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for IncBrightness
ff4-spike-gemma4_field-incbrightness: src/spike_incbrightness_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_incbrightness_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for IncZoomMult
ff4-spike-gemma4_field-inczoommult: src/spike_inczoommult_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_inczoommult_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for InitDMA
ff4-spike-gemma4_field-initdma: src/spike_initdma_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initdma_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for InitDlgIRQ
ff4-spike-gemma4_field-initdlgirq: src/spike_initdlgirq_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initdlgirq_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for InitInterrupts
ff4-spike-gemma4_field-initinterrupts: src/spike_initinterrupts_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initinterrupts_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for InitItemWindowIRQ
ff4-spike-gemma4_field-inititemwindowirq: src/spike_inititemwindowirq_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_inititemwindowirq_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for InitMapRAM
ff4-spike-gemma4_field-initmapram: src/spike_initmapram_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initmapram_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LandBkChoco
ff4-spike-gemma4_field-landbkchoco: src/spike_landbkchoco_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_landbkchoco_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LandHover
ff4-spike-gemma4_field-landhover: src/spike_landhover_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_landhover_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LavaAnimPal
ff4-spike-gemma4_field-lavaanimpal: src/spike_lavaanimpal_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_lavaanimpal_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LiftoffBkChoco
ff4-spike-gemma4_field-liftoffbkchoco: src/spike_liftoffbkchoco_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_liftoffbkchoco_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LiftoffEnterprise
ff4-spike-gemma4_field-liftoffenterprise: src/spike_liftoffenterprise_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_liftoffenterprise_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LiftoffFalcon
ff4-spike-gemma4_field-liftofffalcon: src/spike_liftofffalcon_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_liftofffalcon_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LiftoffWhale
ff4-spike-gemma4_field-liftoffwhale: src/spike_liftoffwhale_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_liftoffwhale_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LoadNPCGfx
ff4-spike-gemma4_field-loadnpcgfx: src/spike_loadnpcgfx_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadnpcgfx_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LoadOverworldIntro
ff4-spike-gemma4_field-loadoverworldintro: src/spike_loadoverworldintro_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadoverworldintro_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LoadPlayerGfx
ff4-spike-gemma4_field-loadplayergfx: src/spike_loadplayergfx_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadplayergfx_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LoadSpriteGfx
ff4-spike-gemma4_field-loadspritegfx: src/spike_loadspritegfx_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadspritegfx_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for LoadTitleGfx
ff4-spike-gemma4_field-loadtitlegfx: src/spike_loadtitlegfx_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadtitlegfx_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for MapGfxBankTbl
ff4-spike-gemma4_field-mapgfxbanktbl: src/spike_mapgfxbanktbl_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mapgfxbanktbl_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for MapTitleTilesBtm
ff4-spike-gemma4_field-maptitletilesbtm: src/spike_maptitletilesbtm_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_maptitletilesbtm_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for MapTitleTilesTop
ff4-spike-gemma4_field-maptitletilestop: src/spike_maptitletilestop_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_maptitletilestop_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for MoveCursor
ff4-spike-gemma4_field-movecursor: src/spike_movecursor_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_movecursor_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for NextChar
ff4-spike-gemma4_field-nextchar: src/spike_nextchar_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nextchar_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for PlayCursorSfx
ff4-spike-gemma4_field-playcursorsfx: src/spike_playcursorsfx_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playcursorsfx_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for PlayMapSong
ff4-spike-gemma4_field-playmapsong: src/spike_playmapsong_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playmapsong_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for PlaySongEvent
ff4-spike-gemma4_field-playsongevent: src/spike_playsongevent_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsongevent_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for PlayerSpritePosTbl
ff4-spike-gemma4_field-playerspritepostbl: src/spike_playerspritepostbl_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playerspritepostbl_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for PrologueSpriteTbl
ff4-spike-gemma4_field-prologuespritetbl: src/spike_prologuespritetbl_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_prologuespritetbl_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for ReloadNPCs
ff4-spike-gemma4_field-reloadnpcs: src/spike_reloadnpcs_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_reloadnpcs_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for RotateWord
ff4-spike-gemma4_field-rotateword: src/spike_rotateword_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_rotateword_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for ScrollItemListDown
ff4-spike-gemma4_field-scrollitemlistdown: src/spike_scrollitemlistdown_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_scrollitemlistdown_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for ScrollItemListUp
ff4-spike-gemma4_field-scrollitemlistup: src/spike_scrollitemlistup_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_scrollitemlistup_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for SetCurrGil
ff4-spike-gemma4_field-setcurrgil: src/spike_setcurrgil_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_setcurrgil_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_02
ff4-spike-gemma4_field-special_02: src/spike_special_02_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_02_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_06
ff4-spike-gemma4_field-special_06: src/spike_special_06_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_0a
ff4-spike-gemma4_field-special_0a: src/spike_special_0a_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_0a_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_0b
ff4-spike-gemma4_field-special_0b: src/spike_special_0b_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_0b_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_0d
ff4-spike-gemma4_field-special_0d: src/spike_special_0d_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_0d_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_12
ff4-spike-gemma4_field-special_12: src/spike_special_12_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_12_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_13
ff4-spike-gemma4_field-special_13: src/spike_special_13_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_13_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_15
ff4-spike-gemma4_field-special_15: src/spike_special_15_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_15_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_19
ff4-spike-gemma4_field-special_19: src/spike_special_19_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_19_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_1a
ff4-spike-gemma4_field-special_1a: src/spike_special_1a_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_1a_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_1b
ff4-spike-gemma4_field-special_1b: src/spike_special_1b_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_1b_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_1c
ff4-spike-gemma4_field-special_1c: src/spike_special_1c_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_1c_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_1d
ff4-spike-gemma4_field-special_1d: src/spike_special_1d_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_1d_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_1e
ff4-spike-gemma4_field-special_1e: src/spike_special_1e_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_1e_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_28
ff4-spike-gemma4_field-special_28: src/spike_special_28_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_28_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_29
ff4-spike-gemma4_field-special_29: src/spike_special_29_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_29_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_2d
ff4-spike-gemma4_field-special_2d: src/spike_special_2d_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_2d_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_2e
ff4-spike-gemma4_field-special_2e: src/spike_special_2e_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_2e_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_2f
ff4-spike-gemma4_field-special_2f: src/spike_special_2f_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_2f_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_30
ff4-spike-gemma4_field-special_30: src/spike_special_30_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_30_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_31
ff4-spike-gemma4_field-special_31: src/spike_special_31_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_31_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Special_3a
ff4-spike-gemma4_field-special_3a: src/spike_special_3a_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_3a_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TfrBG2Tilemap
ff4-spike-gemma4_field-tfrbg2tilemap: src/spike_tfrbg2tilemap_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrbg2tilemap_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TfrBGAnimGfx
ff4-spike-gemma4_field-tfrbganimgfx: src/spike_tfrbganimgfx_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrbganimgfx_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TfrBGGfx
ff4-spike-gemma4_field-tfrbggfx: src/spike_tfrbggfx_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrbggfx_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TfrCaveWaterGfx
ff4-spike-gemma4_field-tfrcavewatergfx: src/spike_tfrcavewatergfx_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrcavewatergfx_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TfrInvertPal
ff4-spike-gemma4_field-tfrinvertpal: src/spike_tfrinvertpal_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrinvertpal_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TfrLavaGfx
ff4-spike-gemma4_field-tfrlavagfx: src/spike_tfrlavagfx_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrlavagfx_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TfrTitleCrystalTiles
ff4-spike-gemma4_field-tfrtitlecrystaltiles: src/spike_tfrtitlecrystaltiles_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrtitlecrystaltiles_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TfrVRAM
ff4-spike-gemma4_field-tfrvram: src/spike_tfrvram_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrvram_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TfrWaterLavaGfx
ff4-spike-gemma4_field-tfrwaterlavagfx: src/spike_tfrwaterlavagfx_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrwaterlavagfx_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TfrWaterTiles
ff4-spike-gemma4_field-tfrwatertiles: src/spike_tfrwatertiles_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrwatertiles_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TitleCrystalVRAMTbl
ff4-spike-gemma4_field-titlecrystalvramtbl: src/spike_titlecrystalvramtbl_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_titlecrystalvramtbl_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for TowerAnimBluePal
ff4-spike-gemma4_field-toweranimbluepal: src/spike_toweranimbluepal_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_toweranimbluepal_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for UpdateBattleParams
ff4-spike-gemma4_field-updatebattleparams: src/spike_updatebattleparams_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatebattleparams_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for UpdateMode7Regs
ff4-spike-gemma4_field-updatemode7regs: src/spike_updatemode7regs_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatemode7regs_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for UpdatePlayerSpeed
ff4-spike-gemma4_field-updateplayerspeed: src/spike_updateplayerspeed_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updateplayerspeed_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for UpdateTopChar
ff4-spike-gemma4_field-updatetopchar: src/spike_updatetopchar_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatetopchar_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for UpdateWhalePal
ff4-spike-gemma4_field-updatewhalepal: src/spike_updatewhalepal_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewhalepal_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for UpdateWipeIRQ
ff4-spike-gemma4_field-updatewipeirq: src/spike_updatewipeirq_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewipeirq_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for UpdateWipeNMI
ff4-spike-gemma4_field-updatewipenmi: src/spike_updatewipenmi_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewipenmi_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for ValidateTopChar
ff4-spike-gemma4_field-validatetopchar: src/spike_validatetopchar_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_validatetopchar_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for Vectors
ff4-spike-gemma4_field-vectors: src/spike_vectors_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_vectors_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for VehicleSpriteTbl
ff4-spike-gemma4_field-vehiclespritetbl: src/spike_vehiclespritetbl_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_vehiclespritetbl_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for WaitKeyDown
ff4-spike-gemma4_field-waitkeydown: src/spike_waitkeydown_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitkeydown_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for WaitKeyUp
ff4-spike-gemma4_field-waitkeyup: src/spike_waitkeyup_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitkeyup_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for WaitSpecial
ff4-spike-gemma4_field-waitspecial: src/spike_waitspecial_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for WaitVblankEvent
ff4-spike-gemma4_field-waitvblankevent: src/spike_waitvblankevent_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblankevent_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for WaitVblankLong
ff4-spike-gemma4_field-waitvblanklong: src/spike_waitvblanklong_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblanklong_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for WaitVblankShort
ff4-spike-gemma4_field-waitvblankshort: src/spike_waitvblankshort_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblankshort_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for WipeIn
ff4-spike-gemma4_field-wipein: src/spike_wipein_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_wipein_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for WipeOut
ff4-spike-gemma4_field-wipeout: src/spike_wipeout_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_wipeout_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for WipeScanlineTbl
ff4-spike-gemma4_field-wipescanlinetbl: src/spike_wipescanlinetbl_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_wipescanlinetbl_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00a51a
ff4-spike-gemma4_field-_00a51a: src/spike__00a51a_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00a51a_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00a527
ff4-spike-gemma4_field-_00a527: src/spike__00a527_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00a527_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00bdb0
ff4-spike-gemma4_field-_00bdb0: src/spike__00bdb0_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00bdb0_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00bfe3
ff4-spike-gemma4_field-_00bfe3: src/spike__00bfe3_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00bfe3_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00cb05
ff4-spike-gemma4_field-_00cb05: src/spike__00cb05_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00cb05_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00cb72
ff4-spike-gemma4_field-_00cb72: src/spike__00cb72_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00cb72_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00cc32
ff4-spike-gemma4_field-_00cc32: src/spike__00cc32_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00cc32_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00cec5
ff4-spike-gemma4_field-_00cec5: src/spike__00cec5_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00cec5_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00cf98
ff4-spike-gemma4_field-_00cf98: src/spike__00cf98_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00cf98_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00d95b
ff4-spike-gemma4_field-_00d95b: src/spike__00d95b_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00d95b_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00da79
ff4-spike-gemma4_field-_00da79: src/spike__00da79_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00da79_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00dd3a
ff4-spike-gemma4_field-_00dd3a: src/spike__00dd3a_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00dd3a_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00de04
ff4-spike-gemma4_field-_00de04: src/spike__00de04_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00de04_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00de1b
ff4-spike-gemma4_field-_00de1b: src/spike__00de1b_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00de1b_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00de2a
ff4-spike-gemma4_field-_00de2a: src/spike__00de2a_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00de2a_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00de43
ff4-spike-gemma4_field-_00de43: src/spike__00de43_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00de43_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00df33
ff4-spike-gemma4_field-_00df33: src/spike__00df33_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00df33_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00df43
ff4-spike-gemma4_field-_00df43: src/spike__00df43_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00df43_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00df53
ff4-spike-gemma4_field-_00df53: src/spike__00df53_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00df53_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00e9cf
ff4-spike-gemma4_field-_00e9cf: src/spike__00e9cf_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00e9cf_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00ea14
ff4-spike-gemma4_field-_00ea14: src/spike__00ea14_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00ea14_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00edf6
ff4-spike-gemma4_field-_00edf6: src/spike__00edf6_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00edf6_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _00f922
ff4-spike-gemma4_field-_00f922: src/spike__00f922_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00f922_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _14f380
ff4-spike-gemma4_field-_14f380: src/spike__14f380_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__14f380_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _14f3a1
ff4-spike-gemma4_field-_14f3a1: src/spike__14f3a1_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__14f3a1_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _14f58e
ff4-spike-gemma4_field-_14f58e: src/spike__14f58e_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__14f58e_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _15b866
ff4-spike-gemma4_field-_15b866: src/spike__15b866_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__15b866_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _15b8c9
ff4-spike-gemma4_field-_15b8c9: src/spike__15b8c9_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__15b8c9_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _15bb6a
ff4-spike-gemma4_field-_15bb6a: src/spike__15bb6a_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__15bb6a_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _15c144
ff4-spike-gemma4_field-_15c144: src/spike__15c144_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__15c144_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for _15c23d
ff4-spike-gemma4_field-_15c23d: src/spike__15c23d_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__15c23d_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for fade_song_half
ff4-spike-gemma4_field-fade_song_half: src/spike_fade_song_half_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fade_song_half_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field): spike for update_dlg_irq
ff4-spike-gemma4_field-update_dlg_irq: src/spike_update_dlg_irq_gemma4_field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_update_dlg_irq_gemma4_field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for AfterBattle
ff4-spike-gemma4_field_v2-afterbattle: src/spike_afterbattle_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_afterbattle_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for AfterCutscene
ff4-spike-gemma4_field_v2-aftercutscene: src/spike_aftercutscene_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aftercutscene_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for BoardBkChoco
ff4-spike-gemma4_field_v2-boardbkchoco: src/spike_boardbkchoco_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardbkchoco_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for BoardChoco
ff4-spike-gemma4_field_v2-boardchoco: src/spike_boardchoco_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardchoco_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for BoardShip
ff4-spike-gemma4_field_v2-boardship: src/spike_boardship_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardship_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for BoardWhale
ff4-spike-gemma4_field_v2-boardwhale: src/spike_boardwhale_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardwhale_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for CheckNPCBlock
ff4-spike-gemma4_field_v2-checknpcblock: src/spike_checknpcblock_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checknpcblock_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for CheckNPCMap2
ff4-spike-gemma4_field_v2-checknpcmap2: src/spike_checknpcmap2_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checknpcmap2_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for CheckNPCSwitch
ff4-spike-gemma4_field_v2-checknpcswitch: src/spike_checknpcswitch_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checknpcswitch_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for CheckTilePass
ff4-spike-gemma4_field_v2-checktilepass: src/spike_checktilepass_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktilepass_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for ChocoSpriteTbl
ff4-spike-gemma4_field_v2-chocospritetbl: src/spike_chocospritetbl_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_chocospritetbl_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for ChooseRandBattle
ff4-spike-gemma4_field_v2-chooserandbattle: src/spike_chooserandbattle_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_chooserandbattle_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for ClearPlayerNPCMap
ff4-spike-gemma4_field_v2-clearplayernpcmap: src/spike_clearplayernpcmap_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearplayernpcmap_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for ClearPrologueVRAM
ff4-spike-gemma4_field_v2-clearprologuevram: src/spike_clearprologuevram_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearprologuevram_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for CloseDlgWindow
ff4-spike-gemma4_field_v2-closedlgwindow: src/spike_closedlgwindow_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closedlgwindow_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for CloseYesNoWindow
ff4-spike-gemma4_field_v2-closeyesnowindow: src/spike_closeyesnowindow_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closeyesnowindow_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for DecBrightness
ff4-spike-gemma4_field_v2-decbrightness: src/spike_decbrightness_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_decbrightness_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for DlgTilesMid
ff4-spike-gemma4_field_v2-dlgtilesmid: src/spike_dlgtilesmid_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_dlgtilesmid_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for DlgTilesTop
ff4-spike-gemma4_field_v2-dlgtilestop: src/spike_dlgtilestop_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_dlgtilestop_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for DrawItemCursor
ff4-spike-gemma4_field_v2-drawitemcursor: src/spike_drawitemcursor_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawitemcursor_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EnterWhale
ff4-spike-gemma4_field_v2-enterwhale: src/spike_enterwhale_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_enterwhale_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EventCmd_d0
ff4-spike-gemma4_field_v2-eventcmd_d0: src/spike_eventcmd_d0_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d0_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EventCmd_d1
ff4-spike-gemma4_field_v2-eventcmd_d1: src/spike_eventcmd_d1_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d1_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EventCmd_d2
ff4-spike-gemma4_field_v2-eventcmd_d2: src/spike_eventcmd_d2_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d2_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EventCmd_d7
ff4-spike-gemma4_field_v2-eventcmd_d7: src/spike_eventcmd_d7_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d7_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EventCmd_d8
ff4-spike-gemma4_field_v2-eventcmd_d8: src/spike_eventcmd_d8_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d8_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EventCmd_dd
ff4-spike-gemma4_field_v2-eventcmd_dd: src/spike_eventcmd_dd_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_dd_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EventCmd_e0
ff4-spike-gemma4_field_v2-eventcmd_e0: src/spike_eventcmd_e0_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_e0_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EventCmd_e6
ff4-spike-gemma4_field_v2-eventcmd_e6: src/spike_eventcmd_e6_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_e6_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EventCmd_ea
ff4-spike-gemma4_field_v2-eventcmd_ea: src/spike_eventcmd_ea_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_ea_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for EventCmd_f9
ff4-spike-gemma4_field_v2-eventcmd_f9: src/spike_eventcmd_f9_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_f9_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for ExecDMA
ff4-spike-gemma4_field_v2-execdma: src/spike_execdma_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execdma_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for ExplosionPosTbl
ff4-spike-gemma4_field_v2-explosionpostbl: src/spike_explosionpostbl_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_explosionpostbl_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for FadeInSong
ff4-spike-gemma4_field_v2-fadeinsong: src/spike_fadeinsong_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeinsong_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for FadeOutSongFast
ff4-spike-gemma4_field_v2-fadeoutsongfast: src/spike_fadeoutsongfast_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeoutsongfast_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for FadeOutSongSlow
ff4-spike-gemma4_field_v2-fadeoutsongslow: src/spike_fadeoutsongslow_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeoutsongslow_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for FieldMain
ff4-spike-gemma4_field_v2-fieldmain: src/spike_fieldmain_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fieldmain_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for FindEventTerminator
ff4-spike-gemma4_field_v2-findeventterminator: src/spike_findeventterminator_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_findeventterminator_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetBattleBG
ff4-spike-gemma4_field_v2-getbattlebg: src/spike_getbattlebg_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbattlebg_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetCurrGil
ff4-spike-gemma4_field_v2-getcurrgil: src/spike_getcurrgil_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getcurrgil_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetDlgID
ff4-spike-gemma4_field_v2-getdlgid: src/spike_getdlgid_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgid_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetDlgPtr0
ff4-spike-gemma4_field_v2-getdlgptr0: src/spike_getdlgptr0_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr0_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetDlgPtr1H
ff4-spike-gemma4_field_v2-getdlgptr1h: src/spike_getdlgptr1h_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetDlgPtr1L
ff4-spike-gemma4_field_v2-getdlgptr1l: src/spike_getdlgptr1l_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1l_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetDlgPtr2
ff4-spike-gemma4_field_v2-getdlgptr2: src/spike_getdlgptr2_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr2_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetDoorTiles
ff4-spike-gemma4_field_v2-getdoortiles: src/spike_getdoortiles_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdoortiles_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetEventSwitchOffset
ff4-spike-gemma4_field_v2-geteventswitchoffset: src/spike_geteventswitchoffset_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_geteventswitchoffset_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetNPCMapPtr
ff4-spike-gemma4_field_v2-getnpcmapptr: src/spike_getnpcmapptr_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcmapptr_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetNPCPtr
ff4-spike-gemma4_field_v2-getnpcptr: src/spike_getnpcptr_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcptr_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetNPCScriptByte
ff4-spike-gemma4_field_v2-getnpcscriptbyte: src/spike_getnpcscriptbyte_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcscriptbyte_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetNextByte
ff4-spike-gemma4_field_v2-getnextbyte: src/spike_getnextbyte_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnextbyte_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetNextEventByte
ff4-spike-gemma4_field_v2-getnexteventbyte: src/spike_getnexteventbyte_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnexteventbyte_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetTileProp
ff4-spike-gemma4_field_v2-gettileprop: src/spike_gettileprop_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettileprop_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetTopCharPtr
ff4-spike-gemma4_field_v2-gettopcharptr: src/spike_gettopcharptr_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettopcharptr_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetTreasurePtr
ff4-spike-gemma4_field_v2-gettreasureptr: src/spike_gettreasureptr_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettreasureptr_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GetTreasureTiles
ff4-spike-gemma4_field_v2-gettreasuretiles: src/spike_gettreasuretiles_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettreasuretiles_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GilWindowTiles1
ff4-spike-gemma4_field_v2-gilwindowtiles1: src/spike_gilwindowtiles1_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gilwindowtiles1_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GilWindowTiles3
ff4-spike-gemma4_field_v2-gilwindowtiles3: src/spike_gilwindowtiles3_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gilwindowtiles3_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GilWindowTiles4
ff4-spike-gemma4_field_v2-gilwindowtiles4: src/spike_gilwindowtiles4_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gilwindowtiles4_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for GiveGil
ff4-spike-gemma4_field_v2-givegil: src/spike_givegil_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_givegil_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for HideMapTitle
ff4-spike-gemma4_field_v2-hidemaptitle: src/spike_hidemaptitle_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_hidemaptitle_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for IncBrightness
ff4-spike-gemma4_field_v2-incbrightness: src/spike_incbrightness_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_incbrightness_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for IncZoomMult
ff4-spike-gemma4_field_v2-inczoommult: src/spike_inczoommult_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_inczoommult_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for InitDMA
ff4-spike-gemma4_field_v2-initdma: src/spike_initdma_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initdma_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for InitDlgIRQ
ff4-spike-gemma4_field_v2-initdlgirq: src/spike_initdlgirq_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initdlgirq_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for InitInterrupts
ff4-spike-gemma4_field_v2-initinterrupts: src/spike_initinterrupts_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initinterrupts_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for InitItemWindowIRQ
ff4-spike-gemma4_field_v2-inititemwindowirq: src/spike_inititemwindowirq_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_inititemwindowirq_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for InitMapRAM
ff4-spike-gemma4_field_v2-initmapram: src/spike_initmapram_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initmapram_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LandBkChoco
ff4-spike-gemma4_field_v2-landbkchoco: src/spike_landbkchoco_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_landbkchoco_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LandHover
ff4-spike-gemma4_field_v2-landhover: src/spike_landhover_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_landhover_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LavaAnimPal
ff4-spike-gemma4_field_v2-lavaanimpal: src/spike_lavaanimpal_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_lavaanimpal_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LiftoffBkChoco
ff4-spike-gemma4_field_v2-liftoffbkchoco: src/spike_liftoffbkchoco_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_liftoffbkchoco_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LiftoffEnterprise
ff4-spike-gemma4_field_v2-liftoffenterprise: src/spike_liftoffenterprise_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_liftoffenterprise_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LiftoffFalcon
ff4-spike-gemma4_field_v2-liftofffalcon: src/spike_liftofffalcon_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_liftofffalcon_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LiftoffWhale
ff4-spike-gemma4_field_v2-liftoffwhale: src/spike_liftoffwhale_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_liftoffwhale_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LoadNPCGfx
ff4-spike-gemma4_field_v2-loadnpcgfx: src/spike_loadnpcgfx_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadnpcgfx_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LoadOverworldIntro
ff4-spike-gemma4_field_v2-loadoverworldintro: src/spike_loadoverworldintro_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadoverworldintro_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LoadPlayerGfx
ff4-spike-gemma4_field_v2-loadplayergfx: src/spike_loadplayergfx_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadplayergfx_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LoadSpriteGfx
ff4-spike-gemma4_field_v2-loadspritegfx: src/spike_loadspritegfx_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadspritegfx_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for LoadTitleGfx
ff4-spike-gemma4_field_v2-loadtitlegfx: src/spike_loadtitlegfx_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadtitlegfx_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for MapGfxBankTbl
ff4-spike-gemma4_field_v2-mapgfxbanktbl: src/spike_mapgfxbanktbl_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mapgfxbanktbl_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for MapTitleTilesBtm
ff4-spike-gemma4_field_v2-maptitletilesbtm: src/spike_maptitletilesbtm_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_maptitletilesbtm_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for MapTitleTilesTop
ff4-spike-gemma4_field_v2-maptitletilestop: src/spike_maptitletilestop_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_maptitletilestop_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for MoveCursor
ff4-spike-gemma4_field_v2-movecursor: src/spike_movecursor_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_movecursor_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for NextChar
ff4-spike-gemma4_field_v2-nextchar: src/spike_nextchar_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_nextchar_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for PlayCursorSfx
ff4-spike-gemma4_field_v2-playcursorsfx: src/spike_playcursorsfx_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playcursorsfx_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for PlayMapSong
ff4-spike-gemma4_field_v2-playmapsong: src/spike_playmapsong_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playmapsong_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for PlaySongEvent
ff4-spike-gemma4_field_v2-playsongevent: src/spike_playsongevent_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsongevent_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for PlayerSpritePosTbl
ff4-spike-gemma4_field_v2-playerspritepostbl: src/spike_playerspritepostbl_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playerspritepostbl_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for PrologueSpriteTbl
ff4-spike-gemma4_field_v2-prologuespritetbl: src/spike_prologuespritetbl_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_prologuespritetbl_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for ReloadNPCs
ff4-spike-gemma4_field_v2-reloadnpcs: src/spike_reloadnpcs_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_reloadnpcs_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for RotateWord
ff4-spike-gemma4_field_v2-rotateword: src/spike_rotateword_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_rotateword_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for ScrollItemListDown
ff4-spike-gemma4_field_v2-scrollitemlistdown: src/spike_scrollitemlistdown_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_scrollitemlistdown_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for ScrollItemListUp
ff4-spike-gemma4_field_v2-scrollitemlistup: src/spike_scrollitemlistup_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_scrollitemlistup_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for SetCurrGil
ff4-spike-gemma4_field_v2-setcurrgil: src/spike_setcurrgil_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_setcurrgil_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_02
ff4-spike-gemma4_field_v2-special_02: src/spike_special_02_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_02_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_06
ff4-spike-gemma4_field_v2-special_06: src/spike_special_06_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_0a
ff4-spike-gemma4_field_v2-special_0a: src/spike_special_0a_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_0a_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_0b
ff4-spike-gemma4_field_v2-special_0b: src/spike_special_0b_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_0b_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_0d
ff4-spike-gemma4_field_v2-special_0d: src/spike_special_0d_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_0d_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_12
ff4-spike-gemma4_field_v2-special_12: src/spike_special_12_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_12_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_13
ff4-spike-gemma4_field_v2-special_13: src/spike_special_13_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_13_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_15
ff4-spike-gemma4_field_v2-special_15: src/spike_special_15_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_15_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_19
ff4-spike-gemma4_field_v2-special_19: src/spike_special_19_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_19_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_1a
ff4-spike-gemma4_field_v2-special_1a: src/spike_special_1a_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_1a_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_1b
ff4-spike-gemma4_field_v2-special_1b: src/spike_special_1b_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_1b_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_1c
ff4-spike-gemma4_field_v2-special_1c: src/spike_special_1c_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_1c_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_1d
ff4-spike-gemma4_field_v2-special_1d: src/spike_special_1d_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_1d_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_1e
ff4-spike-gemma4_field_v2-special_1e: src/spike_special_1e_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_1e_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_28
ff4-spike-gemma4_field_v2-special_28: src/spike_special_28_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_28_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_29
ff4-spike-gemma4_field_v2-special_29: src/spike_special_29_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_29_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_2d
ff4-spike-gemma4_field_v2-special_2d: src/spike_special_2d_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_2d_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_2e
ff4-spike-gemma4_field_v2-special_2e: src/spike_special_2e_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_2e_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_2f
ff4-spike-gemma4_field_v2-special_2f: src/spike_special_2f_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_2f_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_30
ff4-spike-gemma4_field_v2-special_30: src/spike_special_30_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_30_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_31
ff4-spike-gemma4_field_v2-special_31: src/spike_special_31_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_31_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Special_3a
ff4-spike-gemma4_field_v2-special_3a: src/spike_special_3a_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_3a_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TfrBG2Tilemap
ff4-spike-gemma4_field_v2-tfrbg2tilemap: src/spike_tfrbg2tilemap_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrbg2tilemap_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TfrBGAnimGfx
ff4-spike-gemma4_field_v2-tfrbganimgfx: src/spike_tfrbganimgfx_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrbganimgfx_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TfrBGGfx
ff4-spike-gemma4_field_v2-tfrbggfx: src/spike_tfrbggfx_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrbggfx_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TfrCaveWaterGfx
ff4-spike-gemma4_field_v2-tfrcavewatergfx: src/spike_tfrcavewatergfx_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrcavewatergfx_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TfrInvertPal
ff4-spike-gemma4_field_v2-tfrinvertpal: src/spike_tfrinvertpal_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrinvertpal_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TfrLavaGfx
ff4-spike-gemma4_field_v2-tfrlavagfx: src/spike_tfrlavagfx_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrlavagfx_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TfrTitleCrystalTiles
ff4-spike-gemma4_field_v2-tfrtitlecrystaltiles: src/spike_tfrtitlecrystaltiles_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrtitlecrystaltiles_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TfrVRAM
ff4-spike-gemma4_field_v2-tfrvram: src/spike_tfrvram_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrvram_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TfrWaterLavaGfx
ff4-spike-gemma4_field_v2-tfrwaterlavagfx: src/spike_tfrwaterlavagfx_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrwaterlavagfx_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TfrWaterTiles
ff4-spike-gemma4_field_v2-tfrwatertiles: src/spike_tfrwatertiles_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrwatertiles_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TitleCrystalVRAMTbl
ff4-spike-gemma4_field_v2-titlecrystalvramtbl: src/spike_titlecrystalvramtbl_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_titlecrystalvramtbl_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for TowerAnimBluePal
ff4-spike-gemma4_field_v2-toweranimbluepal: src/spike_toweranimbluepal_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_toweranimbluepal_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for UpdateBattleParams
ff4-spike-gemma4_field_v2-updatebattleparams: src/spike_updatebattleparams_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatebattleparams_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for UpdateMode7Regs
ff4-spike-gemma4_field_v2-updatemode7regs: src/spike_updatemode7regs_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatemode7regs_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for UpdatePlayerSpeed
ff4-spike-gemma4_field_v2-updateplayerspeed: src/spike_updateplayerspeed_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updateplayerspeed_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for UpdateTopChar
ff4-spike-gemma4_field_v2-updatetopchar: src/spike_updatetopchar_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatetopchar_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for UpdateWhalePal
ff4-spike-gemma4_field_v2-updatewhalepal: src/spike_updatewhalepal_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewhalepal_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for UpdateWipeIRQ
ff4-spike-gemma4_field_v2-updatewipeirq: src/spike_updatewipeirq_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewipeirq_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for ValidateTopChar
ff4-spike-gemma4_field_v2-validatetopchar: src/spike_validatetopchar_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_validatetopchar_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for Vectors
ff4-spike-gemma4_field_v2-vectors: src/spike_vectors_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_vectors_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for VehicleSpriteTbl
ff4-spike-gemma4_field_v2-vehiclespritetbl: src/spike_vehiclespritetbl_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_vehiclespritetbl_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for WaitKeyDown
ff4-spike-gemma4_field_v2-waitkeydown: src/spike_waitkeydown_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitkeydown_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for WaitKeyUp
ff4-spike-gemma4_field_v2-waitkeyup: src/spike_waitkeyup_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitkeyup_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for WaitSpecial
ff4-spike-gemma4_field_v2-waitspecial: src/spike_waitspecial_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for WaitVblankEvent
ff4-spike-gemma4_field_v2-waitvblankevent: src/spike_waitvblankevent_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblankevent_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for WaitVblankLong
ff4-spike-gemma4_field_v2-waitvblanklong: src/spike_waitvblanklong_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblanklong_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for WaitVblankShort
ff4-spike-gemma4_field_v2-waitvblankshort: src/spike_waitvblankshort_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblankshort_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for WipeIn
ff4-spike-gemma4_field_v2-wipein: src/spike_wipein_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_wipein_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for WipeOut
ff4-spike-gemma4_field_v2-wipeout: src/spike_wipeout_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_wipeout_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for WipeScanlineTbl
ff4-spike-gemma4_field_v2-wipescanlinetbl: src/spike_wipescanlinetbl_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_wipescanlinetbl_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00a51a
ff4-spike-gemma4_field_v2-_00a51a: src/spike__00a51a_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00a51a_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00a527
ff4-spike-gemma4_field_v2-_00a527: src/spike__00a527_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00a527_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00bdb0
ff4-spike-gemma4_field_v2-_00bdb0: src/spike__00bdb0_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00bdb0_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00bfe3
ff4-spike-gemma4_field_v2-_00bfe3: src/spike__00bfe3_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00bfe3_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00cb05
ff4-spike-gemma4_field_v2-_00cb05: src/spike__00cb05_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00cb05_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00cb72
ff4-spike-gemma4_field_v2-_00cb72: src/spike__00cb72_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00cb72_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00cc32
ff4-spike-gemma4_field_v2-_00cc32: src/spike__00cc32_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00cc32_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00cec5
ff4-spike-gemma4_field_v2-_00cec5: src/spike__00cec5_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00cec5_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00cf98
ff4-spike-gemma4_field_v2-_00cf98: src/spike__00cf98_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00cf98_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00d95b
ff4-spike-gemma4_field_v2-_00d95b: src/spike__00d95b_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00d95b_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00da79
ff4-spike-gemma4_field_v2-_00da79: src/spike__00da79_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00da79_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00dd3a
ff4-spike-gemma4_field_v2-_00dd3a: src/spike__00dd3a_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00dd3a_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00de04
ff4-spike-gemma4_field_v2-_00de04: src/spike__00de04_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00de04_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00de1b
ff4-spike-gemma4_field_v2-_00de1b: src/spike__00de1b_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00de1b_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00de2a
ff4-spike-gemma4_field_v2-_00de2a: src/spike__00de2a_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00de2a_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00de43
ff4-spike-gemma4_field_v2-_00de43: src/spike__00de43_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00de43_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00df33
ff4-spike-gemma4_field_v2-_00df33: src/spike__00df33_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00df33_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00df43
ff4-spike-gemma4_field_v2-_00df43: src/spike__00df43_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00df43_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00df53
ff4-spike-gemma4_field_v2-_00df53: src/spike__00df53_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00df53_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00e9cf
ff4-spike-gemma4_field_v2-_00e9cf: src/spike__00e9cf_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00e9cf_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00ea14
ff4-spike-gemma4_field_v2-_00ea14: src/spike__00ea14_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00ea14_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00edf6
ff4-spike-gemma4_field_v2-_00edf6: src/spike__00edf6_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00edf6_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _00f922
ff4-spike-gemma4_field_v2-_00f922: src/spike__00f922_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00f922_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _14f380
ff4-spike-gemma4_field_v2-_14f380: src/spike__14f380_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__14f380_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _14f3a1
ff4-spike-gemma4_field_v2-_14f3a1: src/spike__14f3a1_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__14f3a1_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _14f58e
ff4-spike-gemma4_field_v2-_14f58e: src/spike__14f58e_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__14f58e_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _15b866
ff4-spike-gemma4_field_v2-_15b866: src/spike__15b866_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__15b866_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _15b8c9
ff4-spike-gemma4_field_v2-_15b8c9: src/spike__15b8c9_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__15b8c9_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _15bb6a
ff4-spike-gemma4_field_v2-_15bb6a: src/spike__15bb6a_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__15bb6a_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _15c144
ff4-spike-gemma4_field_v2-_15c144: src/spike__15c144_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__15c144_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for _15c23d
ff4-spike-gemma4_field_v2-_15c23d: src/spike__15c23d_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__15c23d_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for fade_song_half
ff4-spike-gemma4_field_v2-fade_song_half: src/spike_fade_song_half_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fade_song_half_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (gemma4_field_v2): spike for update_dlg_irq
ff4-spike-gemma4_field_v2-update_dlg_irq: src/spike_update_dlg_irq_gemma4_field_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_update_dlg_irq_gemma4_field_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for MagicEffect_24
ff4-spike-auto-magiceffect_24: src/spike_magiceffect_24_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_24_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for EventCmd_d0
ff4-spike-auto-eventcmd_d0: src/spike_eventcmd_d0_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_eventcmd_d0_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for TimerDur_02
ff4-spike-auto-timerdur_02: src/spike_timerdur_02_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_timerdur_02_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for MagicEffect_08
ff4-spike-auto-magiceffect_08: src/spike_magiceffect_08_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magiceffect_08_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for AICond_0a
ff4-spike-auto-aicond_0a: src/spike_aicond_0a_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_0a_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for DlgTilesTop
ff4-spike-auto-dlgtilestop: src/spike_dlgtilestop_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_dlgtilestop_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for CmdTbl
ff4-spike-auto-cmdtbl: src/spike_cmdtbl_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmdtbl_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for _13ddd6
ff4-spike-auto-_13ddd6: src/spike__13ddd6_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__13ddd6_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for Special_06
ff4-spike-auto-special_06: src/spike_special_06_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for MapGfxBankTbl
ff4-spike-auto-mapgfxbanktbl: src/spike_mapgfxbanktbl_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mapgfxbanktbl_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for UpdateWhalePal
ff4-spike-auto-updatewhalepal: src/spike_updatewhalepal_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewhalepal_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for WaitSpecial
ff4-spike-auto-waitspecial: src/spike_waitspecial_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for EndCredits
ff4-spike-auto-endcredits: src/spike_endcredits_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endcredits_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for GetDlgPtr1H
ff4-spike-auto-getdlgptr1h: src/spike_getdlgptr1h_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for UpdateCtrl
ff4-spike-auto-updatectrl: src/spike_updatectrl_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for InitCtrl
ff4-spike-auto-initctrl: src/spike_initctrl_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initctrl_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for MagicMultiTarget
ff4-spike-auto-magicmultitarget: src/spike_magicmultitarget_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_magicmultitarget_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for _00bfe3
ff4-spike-auto-_00bfe3: src/spike__00bfe3_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00bfe3_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for AICondTbl
ff4-spike-auto-aicondtbl: src/spike_aicondtbl_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtbl_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for Rand
ff4-spike-auto-rand: src/spike_rand_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_rand_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for SpellListTbl
ff4-spike-auto-spelllisttbl: src/spike_spelllisttbl_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_spelllisttbl_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for Max99
ff4-spike-auto-max99: src/spike_max99_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_max99_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for RotateWord
ff4-spike-auto-rotateword: src/spike_rotateword_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_rotateword_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for CreditsStarsTileTbl
ff4-spike-auto-creditsstarstiletbl: src/spike_creditsstarstiletbl_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_creditsstarstiletbl_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for BtnAction
ff4-spike-t1415menu-btnaction: src/spike_btnaction_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_btnaction_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415field): spike for HexToDec
ff4-spike-t1415field-hextodec: src/spike_hextodec_t1415field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_hextodec_t1415field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for CharOrderTbl
ff4-spike-t1415menu-charordertbl: src/spike_charordertbl_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_charordertbl_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415field): spike for Mult16
ff4-spike-t1415field-mult16: src/spike_mult16_t1415field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult16_t1415field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for ClearText_ext
ff4-spike-t1415menu-cleartext_ext: src/spike_cleartext_ext_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cleartext_ext_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415field): spike for Pow10Hi
ff4-spike-t1415field-pow10hi: src/spike_pow10hi_t1415field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_pow10hi_t1415field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415field): spike for Pow10Lo
ff4-spike-t1415field-pow10lo: src/spike_pow10lo_t1415field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_pow10lo_t1415field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415field): spike for TfrChestDoor
ff4-spike-t1415field-tfrchestdoor: src/spike_tfrchestdoor_t1415field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrchestdoor_t1415field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415field): spike for UpdateZoomHDMA
ff4-spike-t1415field-updatezoomhdma: src/spike_updatezoomhdma_t1415field.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatezoomhdma_t1415field.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for InitCtrl
ff4-spike-t1415menu-initctrl: src/spike_initctrl_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initctrl_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for InitCtrl_ext2
ff4-spike-t1415menu-initctrl_ext2: src/spike_initctrl_ext2_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initctrl_ext2_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for RestoreDlgGfx_ext
ff4-spike-t1415menu-restoredlggfx_ext: src/spike_restoredlggfx_ext_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_restoredlggfx_ext_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for SaveDlgGfx
ff4-spike-t1415menu-savedlggfx: src/spike_savedlggfx_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_savedlggfx_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for SpellListTbl
ff4-spike-t1415menu-spelllisttbl: src/spike_spelllisttbl_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_spelllisttbl_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for UpdateCtrl
ff4-spike-t1415menu-updatectrl: src/spike_updatectrl_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for UpdateCtrl_ext
ff4-spike-t1415menu-updatectrl_ext: src/spike_updatectrl_ext_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_ext_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for UpdateScrollRegs_ext
ff4-spike-t1415menu-updatescrollregs_ext: src/spike_updatescrollregs_ext_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatescrollregs_ext_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for UpdateWindowColor
ff4-spike-t1415menu-updatewindowcolor: src/spike_updatewindowcolor_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewindowcolor_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (t1415menu): spike for UpdateWindowColor_ext
ff4-spike-t1415menu-updatewindowcolor_ext: src/spike_updatewindowcolor_ext_t1415menu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewindowcolor_ext_t1415menu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for AICond_02
ff4-spike-reg_v0-aicond_02: src/spike_aicond_02_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for GetOtherPlanetTile
ff4-spike-reg_v0-getotherplanettile: src/spike_getotherplanettile_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for GetEarthSpritePos
ff4-spike-reg_v0-getearthspritepos: src/spike_getearthspritepos_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for InitStars
ff4-spike-reg_v0-initstars: src/spike_initstars_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for AICond_00
ff4-spike-reg_v0-aicond_00: src/spike_aicond_00_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_00_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for NewLine
ff4-spike-reg_v0-newline: src/spike_newline_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for HexToDec
ff4-spike-reg_v0-hextodec: src/spike_hextodec_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_hextodec_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for DrawPos
ff4-spike-reg_v0-drawpos: src/spike_drawpos_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawpos_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for BtnAction
ff4-spike-reg_v0-btnaction: src/spike_btnaction_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_btnaction_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for Mult16
ff4-spike-reg_v0-mult16: src/spike_mult16_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult16_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for ExecInterrupt
ff4-spike-reg_v0-execinterrupt: src/spike_execinterrupt_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execinterrupt_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for PlayGameSfx
ff4-spike-reg_v0-playgamesfx: src/spike_playgamesfx_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playgamesfx_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for PlaySystemSfx
ff4-spike-reg_v0-playsystemsfx: src/spike_playsystemsfx_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsystemsfx_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (smoke): spike for AICond_00
ff4-spike-smoke-aicond_00: src/spike_aicond_00_smoke.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_00_smoke.c $(CORE_SRC) $(LDFLAGS)

# AUTO (smoke2): spike for AICond_00
ff4-spike-smoke2-aicond_00: src/spike_aicond_00_smoke2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_00_smoke2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (smoke3): spike for AICondTarget_23
ff4-spike-smoke3-aicondtarget_23: src/spike_aicondtarget_23_smoke3.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_23_smoke3.c $(CORE_SRC) $(LDFLAGS)

# AUTO (smoke4): spike for AICond_02
ff4-spike-smoke4-aicond_02: src/spike_aicond_02_smoke4.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_smoke4.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for AICondTarget_25
ff4-spike-reg_v0-aicondtarget_25: src/spike_aicondtarget_25_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for AICond_05
ff4-spike-reg_v0-aicond_05: src/spike_aicond_05_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for AfterBattle
ff4-spike-reg_v0-afterbattle: src/spike_afterbattle_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_afterbattle_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for BoardChoco
ff4-spike-reg_v0-boardchoco: src/spike_boardchoco_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardchoco_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for CheckTilePass
ff4-spike-reg_v0-checktilepass: src/spike_checktilepass_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktilepass_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v0): spike for CloseYesNoWindow
ff4-spike-reg_v0-closeyesnowindow: src/spike_closeyesnowindow_reg_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closeyesnowindow_reg_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v0): spike for _00bfe3
ff4-spike-mut_v0-_00bfe3: src/spike__00bfe3_mut_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00bfe3_mut_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v1): spike for _00bfe3
ff4-spike-mut__candidate_v1-_00bfe3: src/spike__00bfe3_mut__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike__00bfe3_mut__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v0): spike for CmdTbl
ff4-spike-mut_v0-cmdtbl: src/spike_cmdtbl_mut_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmdtbl_mut_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v0): spike for Special_06
ff4-spike-mut_v0-special_06: src/spike_special_06_mut_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_mut_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v1): spike for Special_06
ff4-spike-mut__candidate_v1-special_06: src/spike_special_06_mut__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_mut__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v0): spike for MapGfxBankTbl
ff4-spike-mut_v0-mapgfxbanktbl: src/spike_mapgfxbanktbl_mut_v0.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mapgfxbanktbl_mut_v0.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v1): spike for MapGfxBankTbl
ff4-spike-mut__candidate_v1-mapgfxbanktbl: src/spike_mapgfxbanktbl_mut__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mapgfxbanktbl_mut__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for AICondTarget_25
ff4-spike-reg__candidate_v1-aicondtarget_25: src/spike_aicondtarget_25_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for AICond_02
ff4-spike-reg__candidate_v1-aicond_02: src/spike_aicond_02_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for AICond_05
ff4-spike-reg__candidate_v1-aicond_05: src/spike_aicond_05_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for GetOtherPlanetTile
ff4-spike-reg__candidate_v1-getotherplanettile: src/spike_getotherplanettile_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for GetEarthSpritePos
ff4-spike-reg__candidate_v1-getearthspritepos: src/spike_getearthspritepos_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for AfterBattle
ff4-spike-reg__candidate_v1-afterbattle: src/spike_afterbattle_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_afterbattle_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for NewLine
ff4-spike-reg__candidate_v1-newline: src/spike_newline_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for BoardChoco
ff4-spike-reg__candidate_v1-boardchoco: src/spike_boardchoco_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardchoco_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for InitStars
ff4-spike-reg__candidate_v1-initstars: src/spike_initstars_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for CloseYesNoWindow
ff4-spike-reg__candidate_v1-closeyesnowindow: src/spike_closeyesnowindow_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closeyesnowindow_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for PlaySystemSfx
ff4-spike-reg__candidate_v1-playsystemsfx: src/spike_playsystemsfx_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsystemsfx_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for PlayGameSfx
ff4-spike-reg__candidate_v1-playgamesfx: src/spike_playgamesfx_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playgamesfx_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for CheckTilePass
ff4-spike-reg__candidate_v1-checktilepass: src/spike_checktilepass_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktilepass_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v1): spike for ExecInterrupt
ff4-spike-reg__candidate_v1-execinterrupt: src/spike_execinterrupt_reg__candidate_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execinterrupt_reg__candidate_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v1): spike for CmdTbl
ff4-spike-mut_v1-cmdtbl: src/spike_cmdtbl_mut_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_cmdtbl_mut_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v1): spike for Special_06
ff4-spike-mut_v1-special_06: src/spike_special_06_mut_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_mut_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2): spike for Special_06
ff4-spike-mut__candidate_v2-special_06: src/spike_special_06_mut__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_mut__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v1): spike for MapGfxBankTbl
ff4-spike-mut_v1-mapgfxbanktbl: src/spike_mapgfxbanktbl_mut_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mapgfxbanktbl_mut_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v1): spike for UpdateWhalePal
ff4-spike-mut_v1-updatewhalepal: src/spike_updatewhalepal_mut_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewhalepal_mut_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2): spike for UpdateWhalePal
ff4-spike-mut__candidate_v2-updatewhalepal: src/spike_updatewhalepal_mut__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewhalepal_mut__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v1): spike for WaitSpecial
ff4-spike-mut_v1-waitspecial: src/spike_waitspecial_mut_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_mut_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (iter1_WaitVblank_far): spike for WaitVblank_far
ff4-spike-iter1_WaitVblank_far-waitvblank_far: src/spike_waitvblank_far_iter1_WaitVblank_far.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblank_far_iter1_WaitVblank_far.c $(CORE_SRC) $(LDFLAGS)

# AUTO (iter1_ScreenOff): spike for ScreenOff
ff4-spike-iter1_ScreenOff-screenoff: src/spike_screenoff_iter1_ScreenOff.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_screenoff_iter1_ScreenOff.c $(CORE_SRC) $(LDFLAGS)

# AUTO (iter1_UpdateCtrlMenu): spike for UpdateCtrlMenu
ff4-spike-iter1_UpdateCtrlMenu-updatectrlmenu: src/spike_updatectrlmenu_iter1_UpdateCtrlMenu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrlmenu_iter1_UpdateCtrlMenu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (iter1_OpenWindow): spike for OpenWindow
ff4-spike-iter1_OpenWindow-openwindow: src/spike_openwindow_iter1_OpenWindow.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_openwindow_iter1_OpenWindow.c $(CORE_SRC) $(LDFLAGS)

# AUTO (iter1_CloseWindow): spike for CloseWindow
ff4-spike-iter1_CloseWindow-closewindow: src/spike_closewindow_iter1_CloseWindow.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closewindow_iter1_CloseWindow.c $(CORE_SRC) $(LDFLAGS)

# AUTO (iter1_TfrVRAM): spike for TfrVRAM
ff4-spike-iter1_TfrVRAM-tfrvram: src/spike_tfrvram_iter1_TfrVRAM.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrvram_iter1_TfrVRAM.c $(CORE_SRC) $(LDFLAGS)

# AUTO (iter1_WaitVblankLong): spike for WaitVblankLong
ff4-spike-iter1_WaitVblankLong-waitvblanklong: src/spike_waitvblanklong_iter1_WaitVblankLong.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblanklong_iter1_WaitVblankLong.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2): spike for WaitSpecial
ff4-spike-mut__candidate_v2-waitspecial: src/spike_waitspecial_mut__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_mut__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v1): spike for GetDlgPtr1H
ff4-spike-mut_v1-getdlgptr1h: src/spike_getdlgptr1h_mut_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_mut_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2): spike for GetDlgPtr1H
ff4-spike-mut__candidate_v2-getdlgptr1h: src/spike_getdlgptr1h_mut__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_mut__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v1): spike for UpdateCtrl
ff4-spike-mut_v1-updatectrl: src/spike_updatectrl_mut_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_mut_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2): spike for UpdateCtrl
ff4-spike-mut__candidate_v2-updatectrl: src/spike_updatectrl_mut__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_mut__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v1): spike for EndCredits
ff4-spike-mut_v1-endcredits: src/spike_endcredits_mut_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endcredits_mut_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2): spike for EndCredits
ff4-spike-mut__candidate_v2-endcredits: src/spike_endcredits_mut__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endcredits_mut__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for AICond_05
ff4-spike-reg__candidate_v2-aicond_05: src/spike_aicond_05_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for AICond_02
ff4-spike-reg__candidate_v2-aicond_02: src/spike_aicond_02_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for AICondTarget_25
ff4-spike-reg__candidate_v2-aicondtarget_25: src/spike_aicondtarget_25_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for GetOtherPlanetTile
ff4-spike-reg__candidate_v2-getotherplanettile: src/spike_getotherplanettile_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for GetEarthSpritePos
ff4-spike-reg__candidate_v2-getearthspritepos: src/spike_getearthspritepos_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for NewLine
ff4-spike-reg__candidate_v2-newline: src/spike_newline_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for InitStars
ff4-spike-reg__candidate_v2-initstars: src/spike_initstars_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for AfterBattle
ff4-spike-reg__candidate_v2-afterbattle: src/spike_afterbattle_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_afterbattle_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for BoardChoco
ff4-spike-reg__candidate_v2-boardchoco: src/spike_boardchoco_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardchoco_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for ExecInterrupt
ff4-spike-reg__candidate_v2-execinterrupt: src/spike_execinterrupt_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execinterrupt_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for CloseYesNoWindow
ff4-spike-reg__candidate_v2-closeyesnowindow: src/spike_closeyesnowindow_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closeyesnowindow_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for PlaySystemSfx
ff4-spike-reg__candidate_v2-playsystemsfx: src/spike_playsystemsfx_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsystemsfx_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2): spike for PlayGameSfx
ff4-spike-reg__candidate_v2-playgamesfx: src/spike_playgamesfx_reg__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playgamesfx_reg__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for AICond_05
ff4-spike-reg_v1-aicond_05: src/spike_aicond_05_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for AICond_02
ff4-spike-reg_v1-aicond_02: src/spike_aicond_02_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for AICondTarget_25
ff4-spike-reg_v1-aicondtarget_25: src/spike_aicondtarget_25_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for GetOtherPlanetTile
ff4-spike-reg_v1-getotherplanettile: src/spike_getotherplanettile_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for GetEarthSpritePos
ff4-spike-reg_v1-getearthspritepos: src/spike_getearthspritepos_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for AfterBattle
ff4-spike-reg_v1-afterbattle: src/spike_afterbattle_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_afterbattle_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for BoardChoco
ff4-spike-reg_v1-boardchoco: src/spike_boardchoco_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardchoco_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for NewLine
ff4-spike-reg_v1-newline: src/spike_newline_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for InitStars
ff4-spike-reg_v1-initstars: src/spike_initstars_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for CloseYesNoWindow
ff4-spike-reg_v1-closeyesnowindow: src/spike_closeyesnowindow_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closeyesnowindow_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for ExecInterrupt
ff4-spike-reg_v1-execinterrupt: src/spike_execinterrupt_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execinterrupt_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for PlayGameSfx
ff4-spike-reg_v1-playgamesfx: src/spike_playgamesfx_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playgamesfx_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for PlaySystemSfx
ff4-spike-reg_v1-playsystemsfx: src/spike_playsystemsfx_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsystemsfx_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v1): spike for InitCtrl
ff4-spike-mut_v1-initctrl: src/spike_initctrl_mut_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initctrl_mut_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2): spike for InitCtrl
ff4-spike-mut__candidate_v2-initctrl: src/spike_initctrl_mut__candidate_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initctrl_mut__candidate_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2_a): spike for Special_06
ff4-spike-mut__candidate_v2_a-special_06: src/spike_special_06_mut__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_mut__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2_b): spike for Special_06
ff4-spike-mut__candidate_v2_b-special_06: src/spike_special_06_mut__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_mut__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2_c): spike for Special_06
ff4-spike-mut__candidate_v2_c-special_06: src/spike_special_06_mut__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_mut__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg_v1): spike for CheckTilePass
ff4-spike-reg_v1-checktilepass: src/spike_checktilepass_reg_v1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktilepass_reg_v1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2_a): spike for MapGfxBankTbl
ff4-spike-mut__candidate_v2_a-mapgfxbanktbl: src/spike_mapgfxbanktbl_mut__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mapgfxbanktbl_mut__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for AICondTarget_25
ff4-spike-reg__candidate_v2_a-aicondtarget_25: src/spike_aicondtarget_25_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for AICond_05
ff4-spike-reg__candidate_v2_a-aicond_05: src/spike_aicond_05_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for AICond_02
ff4-spike-reg__candidate_v2_a-aicond_02: src/spike_aicond_02_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for GetOtherPlanetTile
ff4-spike-reg__candidate_v2_a-getotherplanettile: src/spike_getotherplanettile_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for GetEarthSpritePos
ff4-spike-reg__candidate_v2_a-getearthspritepos: src/spike_getearthspritepos_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for NewLine
ff4-spike-reg__candidate_v2_a-newline: src/spike_newline_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for AfterBattle
ff4-spike-reg__candidate_v2_a-afterbattle: src/spike_afterbattle_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_afterbattle_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for BoardChoco
ff4-spike-reg__candidate_v2_a-boardchoco: src/spike_boardchoco_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardchoco_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for InitStars
ff4-spike-reg__candidate_v2_a-initstars: src/spike_initstars_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for CheckTilePass
ff4-spike-reg__candidate_v2_a-checktilepass: src/spike_checktilepass_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktilepass_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for CloseYesNoWindow
ff4-spike-reg__candidate_v2_a-closeyesnowindow: src/spike_closeyesnowindow_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closeyesnowindow_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for PlaySystemSfx
ff4-spike-reg__candidate_v2_a-playsystemsfx: src/spike_playsystemsfx_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsystemsfx_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for ExecInterrupt
ff4-spike-reg__candidate_v2_a-execinterrupt: src/spike_execinterrupt_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execinterrupt_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_a): spike for PlayGameSfx
ff4-spike-reg__candidate_v2_a-playgamesfx: src/spike_playgamesfx_reg__candidate_v2_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playgamesfx_reg__candidate_v2_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2_b): spike for MapGfxBankTbl
ff4-spike-mut__candidate_v2_b-mapgfxbanktbl: src/spike_mapgfxbanktbl_mut__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mapgfxbanktbl_mut__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for AICondTarget_25
ff4-spike-reg__candidate_v2_b-aicondtarget_25: src/spike_aicondtarget_25_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for AICond_05
ff4-spike-reg__candidate_v2_b-aicond_05: src/spike_aicond_05_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for AICond_02
ff4-spike-reg__candidate_v2_b-aicond_02: src/spike_aicond_02_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for AITarget_19
ff4-spike-reg__candidate_v2_b-aitarget_19: src/spike_aitarget_19_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aitarget_19_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for GetOtherPlanetTile
ff4-spike-reg__candidate_v2_b-getotherplanettile: src/spike_getotherplanettile_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for GetEarthSpritePos
ff4-spike-reg__candidate_v2_b-getearthspritepos: src/spike_getearthspritepos_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for NewLine
ff4-spike-reg__candidate_v2_b-newline: src/spike_newline_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for BoardChoco
ff4-spike-reg__candidate_v2_b-boardchoco: src/spike_boardchoco_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardchoco_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for InitStars
ff4-spike-reg__candidate_v2_b-initstars: src/spike_initstars_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for AfterBattle
ff4-spike-reg__candidate_v2_b-afterbattle: src/spike_afterbattle_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_afterbattle_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for CloseYesNoWindow
ff4-spike-reg__candidate_v2_b-closeyesnowindow: src/spike_closeyesnowindow_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closeyesnowindow_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for PlayGameSfx
ff4-spike-reg__candidate_v2_b-playgamesfx: src/spike_playgamesfx_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playgamesfx_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_b): spike for PlaySystemSfx
ff4-spike-reg__candidate_v2_b-playsystemsfx: src/spike_playsystemsfx_reg__candidate_v2_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsystemsfx_reg__candidate_v2_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v2_c): spike for MapGfxBankTbl
ff4-spike-mut__candidate_v2_c-mapgfxbanktbl: src/spike_mapgfxbanktbl_mut__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mapgfxbanktbl_mut__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for AICondTarget_25
ff4-spike-reg__candidate_v2_c-aicondtarget_25: src/spike_aicondtarget_25_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for AICond_05
ff4-spike-reg__candidate_v2_c-aicond_05: src/spike_aicond_05_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for AICond_02
ff4-spike-reg__candidate_v2_c-aicond_02: src/spike_aicond_02_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for GetOtherPlanetTile
ff4-spike-reg__candidate_v2_c-getotherplanettile: src/spike_getotherplanettile_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for GetEarthSpritePos
ff4-spike-reg__candidate_v2_c-getearthspritepos: src/spike_getearthspritepos_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for AfterBattle
ff4-spike-reg__candidate_v2_c-afterbattle: src/spike_afterbattle_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_afterbattle_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for BoardChoco
ff4-spike-reg__candidate_v2_c-boardchoco: src/spike_boardchoco_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardchoco_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for NewLine
ff4-spike-reg__candidate_v2_c-newline: src/spike_newline_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for InitStars
ff4-spike-reg__candidate_v2_c-initstars: src/spike_initstars_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for CheckTilePass
ff4-spike-reg__candidate_v2_c-checktilepass: src/spike_checktilepass_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktilepass_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for CloseYesNoWindow
ff4-spike-reg__candidate_v2_c-closeyesnowindow: src/spike_closeyesnowindow_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closeyesnowindow_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for ExecInterrupt
ff4-spike-reg__candidate_v2_c-execinterrupt: src/spike_execinterrupt_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execinterrupt_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for PlayGameSfx
ff4-spike-reg__candidate_v2_c-playgamesfx: src/spike_playgamesfx_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playgamesfx_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v2_c): spike for PlaySystemSfx
ff4-spike-reg__candidate_v2_c-playsystemsfx: src/spike_playsystemsfx_reg__candidate_v2_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsystemsfx_reg__candidate_v2_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v2): spike for UpdateWhalePal
ff4-spike-mut_v2-updatewhalepal: src/spike_updatewhalepal_mut_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewhalepal_mut_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_a): spike for UpdateWhalePal
ff4-spike-mut__candidate_v3_a-updatewhalepal: src/spike_updatewhalepal_mut__candidate_v3_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewhalepal_mut__candidate_v3_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_b): spike for UpdateWhalePal
ff4-spike-mut__candidate_v3_b-updatewhalepal: src/spike_updatewhalepal_mut__candidate_v3_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewhalepal_mut__candidate_v3_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_c): spike for UpdateWhalePal
ff4-spike-mut__candidate_v3_c-updatewhalepal: src/spike_updatewhalepal_mut__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewhalepal_mut__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for AICondTarget_25
ff4-spike-reg__candidate_v3_c-aicondtarget_25: src/spike_aicondtarget_25_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicondtarget_25_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for AICond_05
ff4-spike-reg__candidate_v3_c-aicond_05: src/spike_aicond_05_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_05_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for AICond_02
ff4-spike-reg__candidate_v3_c-aicond_02: src/spike_aicond_02_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_aicond_02_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for GetOtherPlanetTile
ff4-spike-reg__candidate_v3_c-getotherplanettile: src/spike_getotherplanettile_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getotherplanettile_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for NewLine
ff4-spike-reg__candidate_v3_c-newline: src/spike_newline_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_newline_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for InitStars
ff4-spike-reg__candidate_v3_c-initstars: src/spike_initstars_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initstars_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for AfterBattle
ff4-spike-reg__candidate_v3_c-afterbattle: src/spike_afterbattle_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_afterbattle_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for BoardChoco
ff4-spike-reg__candidate_v3_c-boardchoco: src/spike_boardchoco_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_boardchoco_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for GetEarthSpritePos
ff4-spike-reg__candidate_v3_c-getearthspritepos: src/spike_getearthspritepos_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getearthspritepos_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for CloseYesNoWindow
ff4-spike-reg__candidate_v3_c-closeyesnowindow: src/spike_closeyesnowindow_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closeyesnowindow_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for ExecInterrupt
ff4-spike-reg__candidate_v3_c-execinterrupt: src/spike_execinterrupt_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_execinterrupt_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for PlayGameSfx
ff4-spike-reg__candidate_v3_c-playgamesfx: src/spike_playgamesfx_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playgamesfx_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for PlaySystemSfx
ff4-spike-reg__candidate_v3_c-playsystemsfx: src/spike_playsystemsfx_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsystemsfx_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v2): spike for WaitSpecial
ff4-spike-mut_v2-waitspecial: src/spike_waitspecial_mut_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_mut_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_a): spike for WaitSpecial
ff4-spike-mut__candidate_v3_a-waitspecial: src/spike_waitspecial_mut__candidate_v3_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_mut__candidate_v3_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_b): spike for WaitSpecial
ff4-spike-mut__candidate_v3_b-waitspecial: src/spike_waitspecial_mut__candidate_v3_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_mut__candidate_v3_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_c): spike for WaitSpecial
ff4-spike-mut__candidate_v3_c-waitspecial: src/spike_waitspecial_mut__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_mut__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v2): spike for GetDlgPtr1H
ff4-spike-mut_v2-getdlgptr1h: src/spike_getdlgptr1h_mut_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_mut_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_a): spike for GetDlgPtr1H
ff4-spike-mut__candidate_v3_a-getdlgptr1h: src/spike_getdlgptr1h_mut__candidate_v3_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_mut__candidate_v3_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_b): spike for GetDlgPtr1H
ff4-spike-mut__candidate_v3_b-getdlgptr1h: src/spike_getdlgptr1h_mut__candidate_v3_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_mut__candidate_v3_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut_v2): spike for UpdateCtrl
ff4-spike-mut_v2-updatectrl: src/spike_updatectrl_mut_v2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_mut_v2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_a): spike for UpdateCtrl
ff4-spike-mut__candidate_v3_a-updatectrl: src/spike_updatectrl_mut__candidate_v3_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_mut__candidate_v3_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_b): spike for UpdateCtrl
ff4-spike-mut__candidate_v3_b-updatectrl: src/spike_updatectrl_mut__candidate_v3_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_mut__candidate_v3_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_c): spike for UpdateCtrl
ff4-spike-mut__candidate_v3_c-updatectrl: src/spike_updatectrl_mut__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_mut__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_a): spike for EndCredits
ff4-spike-mut__candidate_v3_a-endcredits: src/spike_endcredits_mut__candidate_v3_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endcredits_mut__candidate_v3_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_b): spike for EndCredits
ff4-spike-mut__candidate_v3_b-endcredits: src/spike_endcredits_mut__candidate_v3_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endcredits_mut__candidate_v3_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_c): spike for EndCredits
ff4-spike-mut__candidate_v3_c-endcredits: src/spike_endcredits_mut__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_endcredits_mut__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (reg__candidate_v3_c): spike for CheckTilePass
ff4-spike-reg__candidate_v3_c-checktilepass: src/spike_checktilepass_reg__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktilepass_reg__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_a): spike for InitCtrl
ff4-spike-mut__candidate_v3_a-initctrl: src/spike_initctrl_mut__candidate_v3_a.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initctrl_mut__candidate_v3_a.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_b): spike for InitCtrl
ff4-spike-mut__candidate_v3_b-initctrl: src/spike_initctrl_mut__candidate_v3_b.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initctrl_mut__candidate_v3_b.c $(CORE_SRC) $(LDFLAGS)

# AUTO (mut__candidate_v3_c): spike for InitCtrl
ff4-spike-mut__candidate_v3_c-initctrl: src/spike_initctrl_mut__candidate_v3_c.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initctrl_mut__candidate_v3_c.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_special_06): spike for Special_06
ff4-spike-hardcore_special_06-special_06: src/spike_special_06_hardcore_special_06.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_hardcore_special_06.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_updatectrl): spike for UpdateCtrl
ff4-spike-hardcore_updatectrl-updatectrl: src/spike_updatectrl_hardcore_updatectrl.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_hardcore_updatectrl.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_openwindow): spike for OpenWindow
ff4-spike-hardcore_openwindow-openwindow: src/spike_openwindow_hardcore_openwindow.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_openwindow_hardcore_openwindow.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_closewindow): spike for CloseWindow
ff4-spike-hardcore_closewindow-closewindow: src/spike_closewindow_hardcore_closewindow.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_closewindow_hardcore_closewindow.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_initctrl): spike for InitCtrl
ff4-spike-hardcore_initctrl-initctrl: src/spike_initctrl_hardcore_initctrl.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initctrl_hardcore_initctrl.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_waitspecial): spike for WaitSpecial
ff4-spike-hardcore_waitspecial-waitspecial: src/spike_waitspecial_hardcore_waitspecial.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_hardcore_waitspecial.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_updatewhalepal): spike for UpdateWhalePal
ff4-spike-hardcore_updatewhalepal-updatewhalepal: src/spike_updatewhalepal_hardcore_updatewhalepal.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewhalepal_hardcore_updatewhalepal.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_getdlgptr1h): spike for GetDlgPtr1H
ff4-spike-hardcore_getdlgptr1h-getdlgptr1h: src/spike_getdlgptr1h_hardcore_getdlgptr1h.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_hardcore_getdlgptr1h.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_getnexteventbyte): spike for GetNextEventByte
ff4-spike-hardcore_getnexteventbyte-getnexteventbyte: src/spike_getnexteventbyte_hardcore_getnexteventbyte.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnexteventbyte_hardcore_getnexteventbyte.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_initcharprop_ext): spike for InitCharProp_ext
ff4-spike-hardcore_initcharprop_ext-initcharprop_ext: src/spike_initcharprop_ext_hardcore_initcharprop_ext.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initcharprop_ext_hardcore_initcharprop_ext.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_inithwregs): spike for InitHWRegs
ff4-spike-hardcore_inithwregs-inithwregs: src/spike_inithwregs_hardcore_inithwregs.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_inithwregs_hardcore_inithwregs.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_checktreasurenpc): spike for CheckTreasureNPC
ff4-spike-hardcore_checktreasurenpc-checktreasurenpc: src/spike_checktreasurenpc_hardcore_checktreasurenpc.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktreasurenpc_hardcore_checktreasurenpc.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_updatelocaltiles): spike for UpdateLocalTiles
ff4-spike-hardcore_updatelocaltiles-updatelocaltiles: src/spike_updatelocaltiles_hardcore_updatelocaltiles.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatelocaltiles_hardcore_updatelocaltiles.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_updatectrlfield_ext): spike for UpdateCtrlField_ext
ff4-spike-hardcore_updatectrlfield_ext-updatectrlfield_ext: src/spike_updatectrlfield_ext_hardcore_updatectrlfield_ext.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrlfield_ext_hardcore_updatectrlfield_ext.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hardcore_loadbattlespeedpostext): spike for LoadBattleSpeedPosText
ff4-spike-hardcore_loadbattlespeedpostext-loadbattlespeedpostext: src/spike_loadbattlespeedpostext_hardcore_loadbattlespeedpostext.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadbattlespeedpostext_hardcore_loadbattlespeedpostext.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_special_06): spike for Special_06
ff4-spike-hcm1_special_06-special_06: src/spike_special_06_hcm1_special_06.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_hcm1_special_06.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_waitspecial): spike for WaitSpecial
ff4-spike-hcm1_waitspecial-waitspecial: src/spike_waitspecial_hcm1_waitspecial.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_hcm1_waitspecial.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_waitspecial): spike for WaitSpecial
ff4-spike-hcm2_waitspecial-waitspecial: src/spike_waitspecial_hcm2_waitspecial.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitspecial_hcm2_waitspecial.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_getdlgptr1h): spike for GetDlgPtr1H
ff4-spike-hcm1_getdlgptr1h-getdlgptr1h: src/spike_getdlgptr1h_hcm1_getdlgptr1h.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_hcm1_getdlgptr1h.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_getdlgptr1h): spike for GetDlgPtr1H
ff4-spike-hcm2_getdlgptr1h-getdlgptr1h: src/spike_getdlgptr1h_hcm2_getdlgptr1h.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdlgptr1h_hcm2_getdlgptr1h.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_pow10hi): spike for Pow10Hi
ff4-spike-hcm1_pow10hi-pow10hi: src/spike_pow10hi_hcm1_pow10hi.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_pow10hi_hcm1_pow10hi.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updatectrl): spike for UpdateCtrl
ff4-spike-hcm1_updatectrl-updatectrl: src/spike_updatectrl_hcm1_updatectrl.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_hcm1_updatectrl.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_updatectrl): spike for UpdateCtrl
ff4-spike-hcm2_updatectrl-updatectrl: src/spike_updatectrl_hcm2_updatectrl.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatectrl_hcm2_updatectrl.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_special_06): spike for Special_06
ff4-spike-hcm2_special_06-special_06: src/spike_special_06_hcm2_special_06.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_special_06_hcm2_special_06.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_loadbggfx): spike for LoadBGGfx
ff4-spike-hcm1_loadbggfx-loadbggfx: src/spike_loadbggfx_hcm1_loadbggfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadbggfx_hcm1_loadbggfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_loadbggfx): spike for LoadBGGfx
ff4-spike-hcm2_loadbggfx-loadbggfx: src/spike_loadbggfx_hcm2_loadbggfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadbggfx_hcm2_loadbggfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_clearbggfx): spike for ClearBGGfx
ff4-spike-hcm1_clearbggfx-clearbggfx: src/spike_clearbggfx_hcm1_clearbggfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearbggfx_hcm1_clearbggfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_clearbggfx): spike for ClearBGGfx
ff4-spike-hcm2_clearbggfx-clearbggfx: src/spike_clearbggfx_hcm2_clearbggfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearbggfx_hcm2_clearbggfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_getnexteventbyte): spike for GetNextEventByte
ff4-spike-hcm1_getnexteventbyte-getnexteventbyte: src/spike_getnexteventbyte_hcm1_getnexteventbyte.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnexteventbyte_hcm1_getnexteventbyte.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_getnexteventbyte): spike for GetNextEventByte
ff4-spike-hcm2_getnexteventbyte-getnexteventbyte: src/spike_getnexteventbyte_hcm2_getnexteventbyte.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnexteventbyte_hcm2_getnexteventbyte.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_findeventterminato): spike for FindEventTerminator
ff4-spike-hcm1_findeventterminato-findeventterminator: src/spike_findeventterminator_hcm1_findeventterminato.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_findeventterminator_hcm1_findeventterminato.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_playsongevent): spike for PlaySongEvent
ff4-spike-hcm1_playsongevent-playsongevent: src/spike_playsongevent_hcm1_playsongevent.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsongevent_hcm1_playsongevent.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_playsongevent): spike for PlaySongEvent
ff4-spike-hcm2_playsongevent-playsongevent: src/spike_playsongevent_hcm2_playsongevent.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsongevent_hcm2_playsongevent.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_getcurrgil): spike for GetCurrGil
ff4-spike-hcm1_getcurrgil-getcurrgil: src/spike_getcurrgil_hcm1_getcurrgil.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getcurrgil_hcm1_getcurrgil.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_getcurrgil): spike for GetCurrGil
ff4-spike-hcm2_getcurrgil-getcurrgil: src/spike_getcurrgil_hcm2_getcurrgil.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getcurrgil_hcm2_getcurrgil.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_drawsecretdoor): spike for DrawSecretDoor
ff4-spike-hcm1_drawsecretdoor-drawsecretdoor: src/spike_drawsecretdoor_hcm1_drawsecretdoor.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawsecretdoor_hcm1_drawsecretdoor.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_drawsecretdoor): spike for DrawSecretDoor
ff4-spike-hcm2_drawsecretdoor-drawsecretdoor: src/spike_drawsecretdoor_hcm2_drawsecretdoor.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawsecretdoor_hcm2_drawsecretdoor.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_getnpcscriptbyte): spike for GetNPCScriptByte
ff4-spike-hcm1_getnpcscriptbyte-getnpcscriptbyte: src/spike_getnpcscriptbyte_hcm1_getnpcscriptbyte.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcscriptbyte_hcm1_getnpcscriptbyte.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_getnpcscriptbyte): spike for GetNPCScriptByte
ff4-spike-hcm2_getnpcscriptbyte-getnpcscriptbyte: src/spike_getnpcscriptbyte_hcm2_getnpcscriptbyte.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcscriptbyte_hcm2_getnpcscriptbyte.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_geteventswitchoffs): spike for GetEventSwitchOffset
ff4-spike-hcm1_geteventswitchoffs-geteventswitchoffset: src/spike_geteventswitchoffset_hcm1_geteventswitchoffs.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_geteventswitchoffset_hcm1_geteventswitchoffs.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_geteventswitchoffs): spike for GetEventSwitchOffset
ff4-spike-hcm2_geteventswitchoffs-geteventswitchoffset: src/spike_geteventswitchoffset_hcm2_geteventswitchoffs.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_geteventswitchoffset_hcm2_geteventswitchoffs.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_getnpcswitchoffset): spike for GetNPCSwitchOffset
ff4-spike-hcm1_getnpcswitchoffset-getnpcswitchoffset: src/spike_getnpcswitchoffset_hcm1_getnpcswitchoffset.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcswitchoffset_hcm1_getnpcswitchoffset.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_getnpcswitchoffset): spike for GetNPCSwitchOffset
ff4-spike-hcm2_getnpcswitchoffset-getnpcswitchoffset: src/spike_getnpcswitchoffset_hcm2_getnpcswitchoffset.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcswitchoffset_hcm2_getnpcswitchoffset.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checkmenu): spike for CheckMenu
ff4-spike-hcm1_checkmenu-checkmenu: src/spike_checkmenu_hcm1_checkmenu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkmenu_hcm1_checkmenu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_checkmenu): spike for CheckMenu
ff4-spike-hcm2_checkmenu-checkmenu: src/spike_checkmenu_hcm2_checkmenu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkmenu_hcm2_checkmenu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updateplayerspeed): spike for UpdatePlayerSpeed
ff4-spike-hcm1_updateplayerspeed-updateplayerspeed: src/spike_updateplayerspeed_hcm1_updateplayerspeed.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updateplayerspeed_hcm1_updateplayerspeed.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_initworld): spike for InitWorld
ff4-spike-hcm1_initworld-initworld: src/spike_initworld_hcm1_initworld.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initworld_hcm1_initworld.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_initinterrupts): spike for InitInterrupts
ff4-spike-hcm1_initinterrupts-initinterrupts: src/spike_initinterrupts_hcm1_initinterrupts.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initinterrupts_hcm1_initinterrupts.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_fadeinmenu): spike for FadeInMenu
ff4-spike-hcm1_fadeinmenu-fadeinmenu: src/spike_fadeinmenu_hcm1_fadeinmenu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeinmenu_hcm1_fadeinmenu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_fadeinmenu): spike for FadeInMenu
ff4-spike-hcm2_fadeinmenu-fadeinmenu: src/spike_fadeinmenu_hcm2_fadeinmenu.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeinmenu_hcm2_fadeinmenu.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_tfrcavewatergfx): spike for TfrCaveWaterGfx
ff4-spike-hcm1_tfrcavewatergfx-tfrcavewatergfx: src/spike_tfrcavewatergfx_hcm1_tfrcavewatergfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrcavewatergfx_hcm1_tfrcavewatergfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_tfrcavewatergfx): spike for TfrCaveWaterGfx
ff4-spike-hcm2_tfrcavewatergfx-tfrcavewatergfx: src/spike_tfrcavewatergfx_hcm2_tfrcavewatergfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrcavewatergfx_hcm2_tfrcavewatergfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_initdma): spike for InitDMA
ff4-spike-hcm1_initdma-initdma: src/spike_initdma_hcm1_initdma.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initdma_hcm1_initdma.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_fadeoutsongfast): spike for FadeOutSongFast
ff4-spike-hcm1_fadeoutsongfast-fadeoutsongfast: src/spike_fadeoutsongfast_hcm1_fadeoutsongfast.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeoutsongfast_hcm1_fadeoutsongfast.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_fadeoutsongfast): spike for FadeOutSongFast
ff4-spike-hcm2_fadeoutsongfast-fadeoutsongfast: src/spike_fadeoutsongfast_hcm2_fadeoutsongfast.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeoutsongfast_hcm2_fadeoutsongfast.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_fadeoutsongslow): spike for FadeOutSongSlow
ff4-spike-hcm1_fadeoutsongslow-fadeoutsongslow: src/spike_fadeoutsongslow_hcm1_fadeoutsongslow.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeoutsongslow_hcm1_fadeoutsongslow.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_fadeoutsongslow): spike for FadeOutSongSlow
ff4-spike-hcm2_fadeoutsongslow-fadeoutsongslow: src/spike_fadeoutsongslow_hcm2_fadeoutsongslow.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeoutsongslow_hcm2_fadeoutsongslow.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_playmapsong): spike for PlayMapSong
ff4-spike-hcm1_playmapsong-playmapsong: src/spike_playmapsong_hcm1_playmapsong.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playmapsong_hcm1_playmapsong.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_playmapsong): spike for PlayMapSong
ff4-spike-hcm2_playmapsong-playmapsong: src/spike_playmapsong_hcm2_playmapsong.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playmapsong_hcm2_playmapsong.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_playsfx): spike for PlaySfx
ff4-spike-hcm1_playsfx-playsfx: src/spike_playsfx_hcm1_playsfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsfx_hcm1_playsfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_playsfx): spike for PlaySfx
ff4-spike-hcm2_playsfx-playsfx: src/spike_playsfx_hcm2_playsfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playsfx_hcm2_playsfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_fadein): spike for FadeIn
ff4-spike-hcm1_fadein-fadein: src/spike_fadein_hcm1_fadein.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadein_hcm1_fadein.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_fadein): spike for FadeIn
ff4-spike-hcm2_fadein-fadein: src/spike_fadein_hcm2_fadein.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadein_hcm2_fadein.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_fadeout): spike for FadeOut
ff4-spike-hcm1_fadeout-fadeout: src/spike_fadeout_hcm1_fadeout.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeout_hcm1_fadeout.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_fadeout): spike for FadeOut
ff4-spike-hcm2_fadeout-fadeout: src/spike_fadeout_hcm2_fadeout.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeout_hcm2_fadeout.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_tfrwatertiles): spike for TfrWaterTiles
ff4-spike-hcm1_tfrwatertiles-tfrwatertiles: src/spike_tfrwatertiles_hcm1_tfrwatertiles.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrwatertiles_hcm1_tfrwatertiles.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_tfrwatertiles): spike for TfrWaterTiles
ff4-spike-hcm2_tfrwatertiles-tfrwatertiles: src/spike_tfrwatertiles_hcm2_tfrwatertiles.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrwatertiles_hcm2_tfrwatertiles.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updatewaterlavaani): spike for UpdateWaterLavaAnim
ff4-spike-hcm1_updatewaterlavaani-updatewaterlavaanim: src/spike_updatewaterlavaanim_hcm1_updatewaterlavaani.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewaterlavaanim_hcm1_updatewaterlavaani.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_tfrwatergfx): spike for TfrWaterGfx
ff4-spike-hcm1_tfrwatergfx-tfrwatergfx: src/spike_tfrwatergfx_hcm1_tfrwatergfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrwatergfx_hcm1_tfrwatergfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_tfrwatergfx): spike for TfrWaterGfx
ff4-spike-hcm2_tfrwatergfx-tfrwatergfx: src/spike_tfrwatergfx_hcm2_tfrwatergfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrwatergfx_hcm2_tfrwatergfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_calcmode7rot): spike for CalcMode7Rot
ff4-spike-hcm1_calcmode7rot-calcmode7rot: src/spike_calcmode7rot_hcm1_calcmode7rot.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calcmode7rot_hcm1_calcmode7rot.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_calcmode7rot): spike for CalcMode7Rot
ff4-spike-hcm2_calcmode7rot-calcmode7rot: src/spike_calcmode7rot_hcm2_calcmode7rot.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calcmode7rot_hcm2_calcmode7rot.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_calcsine): spike for CalcSine
ff4-spike-hcm1_calcsine-calcsine: src/spike_calcsine_hcm1_calcsine.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calcsine_hcm1_calcsine.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_calcsine): spike for CalcSine
ff4-spike-hcm2_calcsine-calcsine: src/spike_calcsine_hcm2_calcsine.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calcsine_hcm2_calcsine.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updatemode7regs): spike for UpdateMode7Regs
ff4-spike-hcm1_updatemode7regs-updatemode7regs: src/spike_updatemode7regs_hcm1_updatemode7regs.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatemode7regs_hcm1_updatemode7regs.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_waitvblanklong): spike for WaitVblankLong
ff4-spike-hcm1_waitvblanklong-waitvblanklong: src/spike_waitvblanklong_hcm1_waitvblanklong.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblanklong_hcm1_waitvblanklong.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_waitvblankshort): spike for WaitVblankShort
ff4-spike-hcm1_waitvblankshort-waitvblankshort: src/spike_waitvblankshort_hcm1_waitvblankshort.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitvblankshort_hcm1_waitvblankshort.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_waitframe): spike for WaitFrame
ff4-spike-hcm1_waitframe-waitframe: src/spike_waitframe_hcm1_waitframe.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_waitframe_hcm1_waitframe.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updatewipeirq): spike for UpdateWipeIRQ
ff4-spike-hcm1_updatewipeirq-updatewipeirq: src/spike_updatewipeirq_hcm1_updatewipeirq.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewipeirq_hcm1_updatewipeirq.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_updatewipeirq): spike for UpdateWipeIRQ
ff4-spike-hcm2_updatewipeirq-updatewipeirq: src/spike_updatewipeirq_hcm2_updatewipeirq.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewipeirq_hcm2_updatewipeirq.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updatewipenmi): spike for UpdateWipeNMI
ff4-spike-hcm1_updatewipenmi-updatewipenmi: src/spike_updatewipenmi_hcm1_updatewipenmi.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewipenmi_hcm1_updatewipenmi.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_updatewipenmi): spike for UpdateWipeNMI
ff4-spike-hcm2_updatewipenmi-updatewipenmi: src/spike_updatewipenmi_hcm2_updatewipenmi.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatewipenmi_hcm2_updatewipenmi.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_loadplayergfxworld): spike for LoadPlayerGfxWorld
ff4-spike-hcm1_loadplayergfxworld-loadplayergfxworld: src/spike_loadplayergfxworld_hcm1_loadplayergfxworld.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadplayergfxworld_hcm1_loadplayergfxworld.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_loadplayergfxsub): spike for LoadPlayerGfxSub
ff4-spike-hcm1_loadplayergfxsub-loadplayergfxsub: src/spike_loadplayergfxsub_hcm1_loadplayergfxsub.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadplayergfxsub_hcm1_loadplayergfxsub.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_inczoommult): spike for IncZoomMult
ff4-spike-hcm1_inczoommult-inczoommult: src/spike_inczoommult_hcm1_inczoommult.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_inczoommult_hcm1_inczoommult.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_inczoommult): spike for IncZoomMult
ff4-spike-hcm2_inczoommult-inczoommult: src/spike_inczoommult_hcm2_inczoommult.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_inczoommult_hcm2_inczoommult.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updatezoomhdma): spike for UpdateZoomHDMA
ff4-spike-hcm1_updatezoomhdma-updatezoomhdma: src/spike_updatezoomhdma_hcm1_updatezoomhdma.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatezoomhdma_hcm1_updatezoomhdma.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_updatezoomhdma): spike for UpdateZoomHDMA
ff4-spike-hcm2_updatezoomhdma-updatezoomhdma: src/spike_updatezoomhdma_hcm2_updatezoomhdma.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatezoomhdma_hcm2_updatezoomhdma.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_drawpos): spike for DrawPos
ff4-spike-hcm1_drawpos-drawpos: src/spike_drawpos_hcm1_drawpos.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawpos_hcm1_drawpos.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_drawpos): spike for DrawPos
ff4-spike-hcm2_drawpos-drawpos: src/spike_drawpos_hcm2_drawpos.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawpos_hcm2_drawpos.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_tfrchestdoor): spike for TfrChestDoor
ff4-spike-hcm1_tfrchestdoor-tfrchestdoor: src/spike_tfrchestdoor_hcm1_tfrchestdoor.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrchestdoor_hcm1_tfrchestdoor.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_tfrchestdoor): spike for TfrChestDoor
ff4-spike-hcm2_tfrchestdoor-tfrchestdoor: src/spike_tfrchestdoor_hcm2_tfrchestdoor.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrchestdoor_hcm2_tfrchestdoor.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_tfrpal): spike for TfrPal
ff4-spike-hcm1_tfrpal-tfrpal: src/spike_tfrpal_hcm1_tfrpal.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrpal_hcm1_tfrpal.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_tfrvram): spike for TfrVRAM
ff4-spike-hcm1_tfrvram-tfrvram: src/spike_tfrvram_hcm1_tfrvram.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrvram_hcm1_tfrvram.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_clearvram): spike for ClearVRAM
ff4-spike-hcm1_clearvram-clearvram: src/spike_clearvram_hcm1_clearvram.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearvram_hcm1_clearvram.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_tfrsprites): spike for TfrSprites
ff4-spike-hcm1_tfrsprites-tfrsprites: src/spike_tfrsprites_hcm1_tfrsprites.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrsprites_hcm1_tfrsprites.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checkplayermovesub): spike for CheckPlayerMoveSub
ff4-spike-hcm1_checkplayermovesub-checkplayermovesub: src/spike_checkplayermovesub_hcm1_checkplayermovesub.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkplayermovesub_hcm1_checkplayermovesub.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_checkplayermovesub): spike for CheckPlayerMoveSub
ff4-spike-hcm2_checkplayermovesub-checkplayermovesub: src/spike_checkplayermovesub_hcm2_checkplayermovesub.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkplayermovesub_hcm2_checkplayermovesub.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_opendoor): spike for OpenDoor
ff4-spike-hcm1_opendoor-opendoor: src/spike_opendoor_hcm1_opendoor.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_opendoor_hcm1_opendoor.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_opendoor): spike for OpenDoor
ff4-spike-hcm2_opendoor-opendoor: src/spike_opendoor_hcm2_opendoor.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_opendoor_hcm2_opendoor.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_setplayernpcmap): spike for SetPlayerNPCMap
ff4-spike-hcm1_setplayernpcmap-setplayernpcmap: src/spike_setplayernpcmap_hcm1_setplayernpcmap.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_setplayernpcmap_hcm1_setplayernpcmap.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_getbg1vramptr): spike for GetBG1VRAMPtr
ff4-spike-hcm1_getbg1vramptr-getbg1vramptr: src/spike_getbg1vramptr_hcm1_getbg1vramptr.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbg1vramptr_hcm1_getbg1vramptr.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_getbg1vramptr): spike for GetBG1VRAMPtr
ff4-spike-hcm2_getbg1vramptr-getbg1vramptr: src/spike_getbg1vramptr_hcm2_getbg1vramptr.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbg1vramptr_hcm2_getbg1vramptr.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_getdoortiles): spike for GetDoorTiles
ff4-spike-hcm1_getdoortiles-getdoortiles: src/spike_getdoortiles_hcm1_getdoortiles.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdoortiles_hcm1_getdoortiles.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_getdoortiles): spike for GetDoorTiles
ff4-spike-hcm2_getdoortiles-getdoortiles: src/spike_getdoortiles_hcm2_getdoortiles.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getdoortiles_hcm2_getdoortiles.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checkplayermovewor): spike for CheckPlayerMoveWorld
ff4-spike-hcm1_checkplayermovewor-checkplayermoveworld: src/spike_checkplayermoveworld_hcm1_checkplayermovewor.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkplayermoveworld_hcm1_checkplayermovewor.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_checkplayermovewor): spike for CheckPlayerMoveWorld
ff4-spike-hcm2_checkplayermovewor-checkplayermoveworld: src/spike_checkplayermoveworld_hcm2_checkplayermovewor.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkplayermoveworld_hcm2_checkplayermovewor.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checkvehicleblock): spike for CheckVehicleBlock
ff4-spike-hcm1_checkvehicleblock-checkvehicleblock: src/spike_checkvehicleblock_hcm1_checkvehicleblock.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkvehicleblock_hcm1_checkvehicleblock.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checknpcmap1): spike for CheckNPCMap1
ff4-spike-hcm1_checknpcmap1-checknpcmap1: src/spike_checknpcmap1_hcm1_checknpcmap1.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checknpcmap1_hcm1_checknpcmap1.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checknpcmap2): spike for CheckNPCMap2
ff4-spike-hcm1_checknpcmap2-checknpcmap2: src/spike_checknpcmap2_hcm1_checknpcmap2.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checknpcmap2_hcm1_checknpcmap2.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_getnpcmapptr): spike for GetNPCMapPtr
ff4-spike-hcm1_getnpcmapptr-getnpcmapptr: src/spike_getnpcmapptr_hcm1_getnpcmapptr.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcmapptr_hcm1_getnpcmapptr.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_getnpcmapptr): spike for GetNPCMapPtr
ff4-spike-hcm2_getnpcmapptr-getnpcmapptr: src/spike_getnpcmapptr_hcm2_getnpcmapptr.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getnpcmapptr_hcm2_getnpcmapptr.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_loadnpcgfx): spike for LoadNPCGfx
ff4-spike-hcm1_loadnpcgfx-loadnpcgfx: src/spike_loadnpcgfx_hcm1_loadnpcgfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadnpcgfx_hcm1_loadnpcgfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_loadnpcgfx): spike for LoadNPCGfx
ff4-spike-hcm2_loadnpcgfx-loadnpcgfx: src/spike_loadnpcgfx_hcm2_loadnpcgfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadnpcgfx_hcm2_loadnpcgfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_loadspritegfx): spike for LoadSpriteGfx
ff4-spike-hcm1_loadspritegfx-loadspritegfx: src/spike_loadspritegfx_hcm1_loadspritegfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadspritegfx_hcm1_loadspritegfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_loadspritegfx): spike for LoadSpriteGfx
ff4-spike-hcm2_loadspritegfx-loadspritegfx: src/spike_loadspritegfx_hcm2_loadspritegfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadspritegfx_hcm2_loadspritegfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checknpcswitch): spike for CheckNPCSwitch
ff4-spike-hcm1_checknpcswitch-checknpcswitch: src/spike_checknpcswitch_hcm1_checknpcswitch.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checknpcswitch_hcm1_checknpcswitch.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_checknpcswitch): spike for CheckNPCSwitch
ff4-spike-hcm2_checknpcswitch-checknpcswitch: src/spike_checknpcswitch_hcm2_checknpcswitch.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checknpcswitch_hcm2_checknpcswitch.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updatezoompal): spike for UpdateZoomPal
ff4-spike-hcm1_updatezoompal-updatezoompal: src/spike_updatezoompal_hcm1_updatezoompal.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatezoompal_hcm1_updatezoompal.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_updatezoompal): spike for UpdateZoomPal
ff4-spike-hcm2_updatezoompal-updatezoompal: src/spike_updatezoompal_hcm2_updatezoompal.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatezoompal_hcm2_updatezoompal.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updatetopchar): spike for UpdateTopChar
ff4-spike-hcm1_updatetopchar-updatetopchar: src/spike_updatetopchar_hcm1_updatetopchar.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatetopchar_hcm1_updatetopchar.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_updatetopchar): spike for UpdateTopChar
ff4-spike-hcm2_updatetopchar-updatetopchar: src/spike_updatetopchar_hcm2_updatetopchar.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatetopchar_hcm2_updatetopchar.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_gettopcharptr): spike for GetTopCharPtr
ff4-spike-hcm1_gettopcharptr-gettopcharptr: src/spike_gettopcharptr_hcm1_gettopcharptr.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettopcharptr_hcm1_gettopcharptr.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_gettopcharptr): spike for GetTopCharPtr
ff4-spike-hcm2_gettopcharptr-gettopcharptr: src/spike_gettopcharptr_hcm2_gettopcharptr.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettopcharptr_hcm2_gettopcharptr.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_loadplayergfx): spike for LoadPlayerGfx
ff4-spike-hcm1_loadplayergfx-loadplayergfx: src/spike_loadplayergfx_hcm1_loadplayergfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadplayergfx_hcm1_loadplayergfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_loadplayergfx): spike for LoadPlayerGfx
ff4-spike-hcm2_loadplayergfx-loadplayergfx: src/spike_loadplayergfx_hcm2_loadplayergfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadplayergfx_hcm2_loadplayergfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checkbattle): spike for CheckBattle
ff4-spike-hcm1_checkbattle-checkbattle: src/spike_checkbattle_hcm1_checkbattle.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkbattle_hcm1_checkbattle.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_checkbattle): spike for CheckBattle
ff4-spike-hcm2_checkbattle-checkbattle: src/spike_checkbattle_hcm2_checkbattle.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkbattle_hcm2_checkbattle.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updatebattleparams): spike for UpdateBattleParams
ff4-spike-hcm1_updatebattleparams-updatebattleparams: src/spike_updatebattleparams_hcm1_updatebattleparams.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatebattleparams_hcm1_updatebattleparams.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_updatebattleparams): spike for UpdateBattleParams
ff4-spike-hcm2_updatebattleparams-updatebattleparams: src/spike_updatebattleparams_hcm2_updatebattleparams.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatebattleparams_hcm2_updatebattleparams.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_getbattlebg): spike for GetBattleBG
ff4-spike-hcm1_getbattlebg-getbattlebg: src/spike_getbattlebg_hcm1_getbattlebg.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbattlebg_hcm1_getbattlebg.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_getbattlebg): spike for GetBattleBG
ff4-spike-hcm2_getbattlebg-getbattlebg: src/spike_getbattlebg_hcm2_getbattlebg.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_getbattlebg_hcm2_getbattlebg.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_chooserandbattle): spike for ChooseRandBattle
ff4-spike-hcm1_chooserandbattle-chooserandbattle: src/spike_chooserandbattle_hcm1_chooserandbattle.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_chooserandbattle_hcm1_chooserandbattle.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_chooserandbattle): spike for ChooseRandBattle
ff4-spike-hcm2_chooserandbattle-chooserandbattle: src/spike_chooserandbattle_hcm2_chooserandbattle.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_chooserandbattle_hcm2_chooserandbattle.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_fadeinsong): spike for FadeInSong
ff4-spike-hcm1_fadeinsong-fadeinsong: src/spike_fadeinsong_hcm1_fadeinsong.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeinsong_hcm1_fadeinsong.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_fadeinsong): spike for FadeInSong
ff4-spike-hcm2_fadeinsong-fadeinsong: src/spike_fadeinsong_hcm2_fadeinsong.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_fadeinsong_hcm2_fadeinsong.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_loadoverworldlevia): spike for LoadOverworldLeviathan
ff4-spike-hcm1_loadoverworldlevia-loadoverworldleviathan: src/spike_loadoverworldleviathan_hcm1_loadoverworldlevia.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadoverworldleviathan_hcm1_loadoverworldlevia.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_loadoverworldlevia): spike for LoadOverworldLeviathan
ff4-spike-hcm2_loadoverworldlevia-loadoverworldleviathan: src/spike_loadoverworldleviathan_hcm2_loadoverworldlevia.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadoverworldleviathan_hcm2_loadoverworldlevia.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_loadmapstack): spike for LoadMapStack
ff4-spike-hcm1_loadmapstack-loadmapstack: src/spike_loadmapstack_hcm1_loadmapstack.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadmapstack_hcm1_loadmapstack.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_loadmapstack): spike for LoadMapStack
ff4-spike-hcm2_loadmapstack-loadmapstack: src/spike_loadmapstack_hcm2_loadmapstack.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadmapstack_hcm2_loadmapstack.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_drawdestroyeddamcy): spike for DrawDestroyedDamcyan
ff4-spike-hcm1_drawdestroyeddamcy-drawdestroyeddamcyan: src/spike_drawdestroyeddamcyan_hcm1_drawdestroyeddamcy.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawdestroyeddamcyan_hcm1_drawdestroyeddamcy.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_drawredwings): spike for DrawRedWings
ff4-spike-hcm1_drawredwings-drawredwings: src/spike_drawredwings_hcm1_drawredwings.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawredwings_hcm1_drawredwings.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_decbrightness): spike for DecBrightness
ff4-spike-hcm1_decbrightness-decbrightness: src/spike_decbrightness_hcm1_decbrightness.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_decbrightness_hcm1_decbrightness.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_decbrightness): spike for DecBrightness
ff4-spike-hcm2_decbrightness-decbrightness: src/spike_decbrightness_hcm2_decbrightness.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_decbrightness_hcm2_decbrightness.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_calcvehiclespritep): spike for CalcVehicleSpritePos
ff4-spike-hcm1_calcvehiclespritep-calcvehiclespritepos: src/spike_calcvehiclespritepos_hcm1_calcvehiclespritep.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calcvehiclespritepos_hcm1_calcvehiclespritep.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_playerspritetiles): spike for PlayerSpriteTiles
ff4-spike-hcm1_playerspritetiles-playerspritetiles: src/spike_playerspritetiles_hcm1_playerspritetiles.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_playerspritetiles_hcm1_playerspritetiles.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_updatebg2scroll): spike for UpdateBG2Scroll
ff4-spike-hcm1_updatebg2scroll-updatebg2scroll: src/spike_updatebg2scroll_hcm1_updatebg2scroll.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatebg2scroll_hcm1_updatebg2scroll.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_updatebg2scroll): spike for UpdateBG2Scroll
ff4-spike-hcm2_updatebg2scroll-updatebg2scroll: src/spike_updatebg2scroll_hcm2_updatebg2scroll.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatebg2scroll_hcm2_updatebg2scroll.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_initbg2tilemap): spike for InitBG2Tilemap
ff4-spike-hcm1_initbg2tilemap-initbg2tilemap: src/spike_initbg2tilemap_hcm1_initbg2tilemap.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initbg2tilemap_hcm1_initbg2tilemap.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_initbg2tilemap): spike for InitBG2Tilemap
ff4-spike-hcm2_initbg2tilemap-initbg2tilemap: src/spike_initbg2tilemap_hcm2_initbg2tilemap.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initbg2tilemap_hcm2_initbg2tilemap.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_tfrbg1tilemap): spike for TfrBG1Tilemap
ff4-spike-hcm1_tfrbg1tilemap-tfrbg1tilemap: src/spike_tfrbg1tilemap_hcm1_tfrbg1tilemap.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrbg1tilemap_hcm1_tfrbg1tilemap.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_tfrbg1tilemap): spike for TfrBG1Tilemap
ff4-spike-hcm2_tfrbg1tilemap-tfrbg1tilemap: src/spike_tfrbg1tilemap_hcm2_tfrbg1tilemap.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrbg1tilemap_hcm2_tfrbg1tilemap.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_tfrworldtilemap): spike for TfrWorldTilemap
ff4-spike-hcm1_tfrworldtilemap-tfrworldtilemap: src/spike_tfrworldtilemap_hcm1_tfrworldtilemap.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrworldtilemap_hcm1_tfrworldtilemap.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_tfrworldtilemap): spike for TfrWorldTilemap
ff4-spike-hcm2_tfrworldtilemap-tfrworldtilemap: src/spike_tfrworldtilemap_hcm2_tfrworldtilemap.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrworldtilemap_hcm2_tfrworldtilemap.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_decodeworldtilemap): spike for DecodeWorldTilemap
ff4-spike-hcm1_decodeworldtilemap-decodeworldtilemap: src/spike_decodeworldtilemap_hcm1_decodeworldtilemap.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_decodeworldtilemap_hcm1_decodeworldtilemap.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_decodeworldtilemap): spike for DecodeWorldTilemap
ff4-spike-hcm2_decodeworldtilemap-decodeworldtilemap: src/spike_decodeworldtilemap_hcm2_decodeworldtilemap.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_decodeworldtilemap_hcm2_decodeworldtilemap.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_decodebg1tilemap): spike for DecodeBG1Tilemap
ff4-spike-hcm1_decodebg1tilemap-decodebg1tilemap: src/spike_decodebg1tilemap_hcm1_decodebg1tilemap.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_decodebg1tilemap_hcm1_decodebg1tilemap.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_clearprologuevram): spike for ClearPrologueVRAM
ff4-spike-hcm1_clearprologuevram-clearprologuevram: src/spike_clearprologuevram_hcm1_clearprologuevram.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearprologuevram_hcm1_clearprologuevram.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_clearprologuevram): spike for ClearPrologueVRAM
ff4-spike-hcm2_clearprologuevram-clearprologuevram: src/spike_clearprologuevram_hcm2_clearprologuevram.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearprologuevram_hcm2_clearprologuevram.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_tfrtitlecrystaltil): spike for TfrTitleCrystalTiles
ff4-spike-hcm1_tfrtitlecrystaltil-tfrtitlecrystaltiles: src/spike_tfrtitlecrystaltiles_hcm1_tfrtitlecrystaltil.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrtitlecrystaltiles_hcm1_tfrtitlecrystaltil.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_tfrtitlecrystaltil): spike for TfrTitleCrystalTiles
ff4-spike-hcm2_tfrtitlecrystaltil-tfrtitlecrystaltiles: src/spike_tfrtitlecrystaltiles_hcm2_tfrtitlecrystaltil.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_tfrtitlecrystaltiles_hcm2_tfrtitlecrystaltil.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checktreasurenpc): spike for CheckTreasureNPC
ff4-spike-hcm1_checktreasurenpc-checktreasurenpc: src/spike_checktreasurenpc_hcm1_checktreasurenpc.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktreasurenpc_hcm1_checktreasurenpc.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_checktreasurenpc): spike for CheckTreasureNPC
ff4-spike-hcm2_checktreasurenpc-checktreasurenpc: src/spike_checktreasurenpc_hcm2_checktreasurenpc.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktreasurenpc_hcm2_checktreasurenpc.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checktreasureswitc): spike for CheckTreasureSwitch
ff4-spike-hcm1_checktreasureswitc-checktreasureswitch: src/spike_checktreasureswitch_hcm1_checktreasureswitc.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktreasureswitch_hcm1_checktreasureswitc.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_checktreasureswitc): spike for CheckTreasureSwitch
ff4-spike-hcm2_checktreasureswitc-checktreasureswitch: src/spike_checktreasureswitch_hcm2_checktreasureswitc.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktreasureswitch_hcm2_checktreasureswitc.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_gettreasureid): spike for GetTreasureID
ff4-spike-hcm1_gettreasureid-gettreasureid: src/spike_gettreasureid_hcm1_gettreasureid.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettreasureid_hcm1_gettreasureid.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_checktriggersub): spike for CheckTriggerSub
ff4-spike-hcm1_checktriggersub-checktriggersub: src/spike_checktriggersub_hcm1_checktriggersub.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checktriggersub_hcm1_checktriggersub.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_loadtitlegfx): spike for LoadTitleGfx
ff4-spike-hcm1_loadtitlegfx-loadtitlegfx: src/spike_loadtitlegfx_hcm1_loadtitlegfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadtitlegfx_hcm1_loadtitlegfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm2_loadtitlegfx): spike for LoadTitleGfx
ff4-spike-hcm2_loadtitlegfx-loadtitlegfx: src/spike_loadtitlegfx_hcm2_loadtitlegfx.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_loadtitlegfx_hcm2_loadtitlegfx.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_maptitletilestop): spike for MapTitleTilesTop
ff4-spike-hcm1_maptitletilestop-maptitletilestop: src/spike_maptitletilestop_hcm1_maptitletilestop.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_maptitletilestop_hcm1_maptitletilestop.c $(CORE_SRC) $(LDFLAGS)

# AUTO (hcm1_maptitletilesbtm): spike for MapTitleTilesBtm
ff4-spike-hcm1_maptitletilesbtm-maptitletilesbtm: src/spike_maptitletilesbtm_hcm1_maptitletilesbtm.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_maptitletilesbtm_hcm1_maptitletilesbtm.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for DrawMonsterSprite
ff4-spike-auto-drawmonstersprite: src/spike_drawmonstersprite_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_drawmonstersprite_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for BuildOAMEntries
ff4-spike-auto-buildoamentries: src/spike_buildoamentries_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_buildoamentries_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for IncrTextPtr
ff4-spike-auto-incrtextptr: src/spike_incrtextptr_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_incrtextptr_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for Mult8_btlgfx
ff4-spike-auto-mult8_btlgfx: src/spike_mult8_btlgfx_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_mult8_btlgfx_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for BackAttackYOffset_l
ff4-spike-auto-backattackyoffset_l: src/spike_backattackyoffset_l_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_backattackyoffset_l_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for HardMult_btlgfx
ff4-spike-auto-hardmult_btlgfx: src/spike_hardmult_btlgfx_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_hardmult_btlgfx_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for BackAttackYOffset_s
ff4-spike-auto-backattackyoffset_s: src/spike_backattackyoffset_s_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_backattackyoffset_s_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for CheckSpriteVisible
ff4-spike-auto-checkspritevisible: src/spike_checkspritevisible_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_checkspritevisible_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for UpdateMonsterAnim
ff4-spike-auto-updatemonsteranim: src/spike_updatemonsteranim_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatemonsteranim_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for InitMonsterAnim
ff4-spike-auto-initmonsteranim: src/spike_initmonsteranim_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_initmonsteranim_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for UpdateBG2ScrollSkip
ff4-spike-auto-updatebg2scrollskip: src/spike_updatebg2scrollskip_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_updatebg2scrollskip_auto.c $(CORE_SRC) $(LDFLAGS)


# AUTO (auto): spike for CalcDmg
ff4-spike-auto-calcdmg: src/spike_calcdmg_auto.c $(CORE_SRC) ../../ff4-gnw/battle/Add16.c ../../ff4-gnw/battle/Mult16.c ../../ff4-gnw/battle/Div16.c
	$(CC) $(CFLAGS) -o $@ src/spike_calcdmg_auto.c $(CORE_SRC) ../../ff4-gnw/battle/Add16.c ../../ff4-gnw/battle/Mult16.c ../../ff4-gnw/battle/Div16.c $(LDFLAGS)

# AUTO (auto): spike for CalcObjScreenPos
ff4-spike-auto-calcobjscreenpos: src/spike_calcobjscreenpos_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_calcobjscreenpos_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for SetNpcMapPtr
ff4-spike-auto-setnpcmapptr: src/spike_setnpcmapptr_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_setnpcmapptr_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for ClearNpcMapCell
ff4-spike-auto-clearnpcmapcell: src/spike_clearnpcmapcell_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_clearnpcmapcell_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for SetNpcMapCell
ff4-spike-auto-setnpcmapcell: src/spike_setnpcmapcell_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_setnpcmapcell_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for GetTileProps
ff4-spike-auto-gettileprops: src/spike_gettileprops_auto.c $(CORE_SRC)
	$(CC) $(CFLAGS) -o $@ src/spike_gettileprops_auto.c $(CORE_SRC) $(LDFLAGS)

# AUTO (auto): spike for UpdateLocalTilesEntry
ff4-spike-auto-updatelocaltilesentry: src/spike_updatelocaltilesentry_auto.c $(CORE_SRC) ../../ff4-gnw/field/GetTileProps.c
	$(CC) $(CFLAGS) -o $@ src/spike_updatelocaltilesentry_auto.c $(CORE_SRC) ../../ff4-gnw/field/GetTileProps.c $(LDFLAGS)
