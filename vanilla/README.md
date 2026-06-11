# vanilla/

This directory is where you place your **legally obtained** Final Fantasy IV
ROM. The ROM is **never** committed to the repo (see `.gitignore`).

## Required ROM

The `everything8215/ff4` disassembly accepts any of these four CRC32 values
(headerless `.sfc` / `.bin`, 1 MB exact):

| CRC32      | Version                  |
|------------|--------------------------|
| `21027C5D` | Final Fantasy IV 1.0 (J) |
| `CAA15E97` | Final Fantasy IV 1.1 (J) |
| `65D0A825` | Final Fantasy II 1.0 (U) |
| `23084FCD` | Final Fantasy II 1.1 (U) |

This project was developed against **`CAA15E97` (JP 1.1)** but the build
pipeline supports all four targets via `make ff4-{jp,jp1,en,en1}`.

## How to use

1. Place your ROM file in this directory (any filename — `decode-ff4.js`
   detects the version by CRC32).
2. From the project root:
   ```bash
   cp vanilla/<your-rom>.bin upstream/vanilla/ff4-jp.sfc
   cd upstream && make rip && make ff4-jp1
   ```
3. The built ROM appears at `upstream/rom/ff4-jp1.sfc`.

## ROM acquisition

Acquiring a ROM legally:
- Dump your own cartridge with a Retrode or SD2SNES
- TOSEC archives for preservation purposes (jurisdiction-dependent)
- Nintendo Switch Online + Save State extraction

This project will NEVER provide a ROM and refuses to discuss illegal sources.
