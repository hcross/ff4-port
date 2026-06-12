// This is not a functional routine but a data script used by the AutoBattle engine.
// It defines sequences of actions (magic/items) for specific characters.
// Structure: [Command, Value], [Command, Value], ..., [0xFF (End)]
const uint8_t auto_battle_script_0007[] = {
    // Character 1 (Generic/First)
    0x00, 0x2F, // Use Magic: Meteo
    0x00, 0x0C, // Use Magic: Dispel
    0xFF,       // End of script

    // Golbez
    0x00, 0x2F, // Use Magic: Meteo
    0x01, 0xC8, // Use Item: Crystal
    0xFF        // End of script
};

// PITFALLS: None. This is static data, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  n/a
//   entry_flags: n/a

REVERSED_FUNCTION: battle::AutoBattle_0007 ($FE:5D)