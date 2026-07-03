/* FF4 desktop — PPM pixel-diff.
 *
 * Closes WF-VALID's mandatory-but-untooled screenshot pixel-diff step (§5,
 * "convert + compare (sips → png, visual diff)"): compares two binary PPM
 * (P6) images byte-for-byte per channel, counts differing pixels above a
 * tolerance, and returns a scriptable exit code instead of requiring an
 * eyeball comparison. The PPM writer already exists in harness_headless.c /
 * oracle_ab.c / main_sdl.c (--out f.ppm) — this tool only reads.
 *
 * Usage: ppmdiff <a.ppm> <b.ppm> [--tolerance N] [--threshold N]
 *   --tolerance N   per-channel (0-255) difference below which a pixel is
 *                   NOT counted as differing (default 0 = exact match)
 *   --threshold N   max allowed differing-pixel count before failing
 *                   (default 0 = any differing pixel fails)
 *
 * Exit codes: 0 = differing pixels <= threshold, 1 = exceeds threshold,
 * 2 = usage/IO/format error (unreadable file, dimension mismatch).
 */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { int w, h; uint8_t *rgb; } Ppm;

/* Skip PPM whitespace and '#' comments between header tokens (PPM spec
 * allows both; our own writers never emit comments, but a golden captured by
 * another tool might). */
static void skip_ws(FILE *f) {
    int c;
    for (;;) {
        c = fgetc(f);
        if (c == '#') { while (c != '\n' && c != EOF) c = fgetc(f); continue; }
        if (c == EOF || !isspace(c)) { if (c != EOF) ungetc(c, f); return; }
    }
}

static int read_ppm(const char *path, Ppm *out) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "error: cannot open '%s'\n", path); return 0; }
    char magic[3] = {0};
    if (fread(magic, 1, 2, f) != 2 || strcmp(magic, "P6") != 0) {
        fprintf(stderr, "error: '%s' is not a binary PPM (P6)\n", path);
        fclose(f); return 0;
    }
    skip_ws(f);
    int w = 0, h = 0, maxval = 0;
    if (fscanf(f, "%d", &w) != 1) { fclose(f); goto bad; }
    skip_ws(f);
    if (fscanf(f, "%d", &h) != 1) { fclose(f); goto bad; }
    skip_ws(f);
    if (fscanf(f, "%d", &maxval) != 1) { fclose(f); goto bad; }
    fgetc(f);  /* exactly one whitespace byte separates the header from binary data */
    if (w <= 0 || h <= 0 || maxval != 255) { fclose(f); goto bad; }
    size_t n = (size_t)w * (size_t)h * 3;
    uint8_t *buf = malloc(n);
    if (!buf || fread(buf, 1, n, f) != n) {
        free(buf); fclose(f);
        fprintf(stderr, "error: '%s' truncated (expected %zu pixel bytes)\n", path, n);
        return 0;
    }
    fclose(f);
    out->w = w; out->h = h; out->rgb = buf;
    return 1;
bad:
    fprintf(stderr, "error: '%s' has a malformed PPM header\n", path);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <a.ppm> <b.ppm> [--tolerance N] [--threshold N]\n", argv[0]);
        return 2;
    }
    int tolerance = 0, threshold = 0;
    for (int i = 3; i < argc; i++) {
        if      (!strcmp(argv[i], "--tolerance") && i + 1 < argc) tolerance = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--threshold") && i + 1 < argc) threshold = atoi(argv[++i]);
        else { fprintf(stderr, "error: bad arg '%s'\n", argv[i]); return 2; }
    }

    Ppm a = {0}, b = {0};
    if (!read_ppm(argv[1], &a)) return 2;
    if (!read_ppm(argv[2], &b)) { free(a.rgb); return 2; }
    if (a.w != b.w || a.h != b.h) {
        fprintf(stderr, "error: dimension mismatch %dx%d vs %dx%d\n", a.w, a.h, b.w, b.h);
        free(a.rgb); free(b.rgb);
        return 2;
    }

    size_t npixels = (size_t)a.w * (size_t)a.h;
    long ndiff = 0;
    for (size_t p = 0; p < npixels; p++) {
        int dr = abs((int)a.rgb[p * 3 + 0] - (int)b.rgb[p * 3 + 0]);
        int dg = abs((int)a.rgb[p * 3 + 1] - (int)b.rgb[p * 3 + 1]);
        int db = abs((int)a.rgb[p * 3 + 2] - (int)b.rgb[p * 3 + 2]);
        if (dr > tolerance || dg > tolerance || db > tolerance) ndiff++;
    }

    printf("dimensions       : %dx%d\n", a.w, a.h);
    printf("tolerance        : %d (per channel)\n", tolerance);
    printf("differing pixels : %ld / %zu\n", ndiff, npixels);
    printf("threshold        : %d\n", threshold);
    printf("verdict          : %s\n", ndiff > threshold ? "FAIL (exceeds threshold)" : "PASS");

    free(a.rgb);
    free(b.rgb);
    return ndiff > threshold ? 1 : 0;
}
