#include "snes/snes.h"

uint16_t val_43 = read16(ram, 0x43);  // includes ram[0x44]
uint16_t new_y = (uint16_t)y + val_43;  // clc means no carry in