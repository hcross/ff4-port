// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: ram[$AB] = signed timer value (8-bit)
// Logic:
//   if (timer < 0) timer = 0
//   store result in ram[$D4]
static void SetTimerDur_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    int8_t timer = (int8_t)ram[0xAB];  // Load as signed 8-bit
    if (timer < 0) {                   // bpl @9fd5 → taken when timer >= 0
        timer = 0;                     // clr_ay → Y = 0 (A is not used)
    }
    ram[0xD4] = (uint8_t)timer;        // sty $d4 (Y is 16-bit, but only low byte matters)
}

// PITFALLS: 6 (mode A assumed 8-bit based on typical battle conventions),
//           8 (X/Y mode inherited as 16-bit from caller)
// HELPERS: none
// CONTRACT:
//   inputs_ram:  0xAB=1
//   output_ram:  0xD4=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: n=auto, z=auto  // set by caller based on ram[$AB]
REVERSED_FUNCTION: battle::SetTimerDur ($9F:CF)