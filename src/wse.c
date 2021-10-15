#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define INPUT_BUFSIZE (1 * 1024 * 1024)
#define OUTPUT_BUFSIZE (4 * INPUT_BUFSIZE)

static uint8_t
half_nibble_to_ws(uint8_t b)
{
    const uint8_t b0 = b & 1;
    const uint8_t b1 = b & 2;
    const uint8_t b3 = (b0 & (b1 >> 1));
    return '\t' + b0 + 2 * b1 + 2 * b3 + 16 * b3;
}

static void
ws_encode(uint8_t c, uint8_t* out)
{
    out[0] = half_nibble_to_ws(c >> 6);
    out[1] = half_nibble_to_ws(c >> 4);
    out[2] = half_nibble_to_ws(c >> 2);
    out[3] = half_nibble_to_ws(c);
}

static void*
alloc_with_fail(size_t sz, const char* purpose)
{
    void* p = malloc(sz);
    if (!p) {
        fprintf(
          stderr, "Failed to allocated '%zu' bytes for %s\n", sz, purpose);
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    return p;
}

static void
encode_stream(FILE* fin, FILE* fout)
{
    uint8_t* bufin = alloc_with_fail(INPUT_BUFSIZE, "input buffer");
    uint8_t* bufout = alloc_with_fail(OUTPUT_BUFSIZE, "output buffer");

    size_t n;

    while ((n = fread(bufin, 1, INPUT_BUFSIZE, fin))) {
        for (size_t i = 0; i < n; ++i) {
            ws_encode(bufin[i], bufout + 4 * i);
        }

        if (fout) {
            fwrite(bufout, 1, 4 * n, fout);
        }
    }
}

int
main(void)
{
#ifdef _WIN32
    _setmode(fileno(stdin), O_BINARY);
    _setmode(fileno(stdout), O_BINARY);
#endif
    encode_stream(stdin, stdout);
}
