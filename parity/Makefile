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
