#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define INPUT_BUFSIZE (4 * 1024 * 1024)
#define OUTPUT_BUFSIZE (INPUT_BUFSIZE / 4)

#if 0

We want a function that maps:

             x |  y
-------------------
 9 (0000_1001) | 0
10 (0000_1010) | 1
13 (0000_1101) | 2
32 (0010_0000) | 3

In integer arithmetic, x/10 gets us there but we have to pad the result when x
== 13. Division is costly, so instead of dividing by 10, we multiply by
ceil(64/10) = 7 and look at how many 64s are in the result. Using 7 (instead
of, e.g., 13, 26, or 0x1999999a) means we can do it all with uint8_t values with
no up/downcasting. Note 13 is the only x value for which !!(ws & 4) is 1.

#endif

static uint8_t
ws_to_half_nibble(uint8_t ws)
{
    return ((7 * ws) >> 6) + !!(ws & 4);
}

static uint8_t
ws_decode(const uint8_t* buf)
{
    return (ws_to_half_nibble(buf[0]) << 6) | (ws_to_half_nibble(buf[1]) << 4) |
           (ws_to_half_nibble(buf[2]) << 2) | ws_to_half_nibble(buf[3]);
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
decode_stream(FILE* fin, FILE* fout)
{
    uint8_t* bufin = alloc_with_fail(INPUT_BUFSIZE, "input buffer");
    uint8_t* bufout = alloc_with_fail(OUTPUT_BUFSIZE, "output buffer");

    size_t n;

    while ((n = fread(bufin, 1, INPUT_BUFSIZE, fin))) {
        for (size_t i = 0; i < n; i += 4) {
            bufout[i / 4] = ws_decode(bufin + i);
        }

        if (fout) {
            fwrite(bufout, 1, n / 4, fout);
        }
    }

    if (fout) {
        fflush(fout);
    }
}

int
main(void)
{
#ifdef _WIN32
    _setmode(fileno(stdin), O_BINARY);
    _setmode(fileno(stdout), O_BINARY);
#endif
    decode_stream(stdin, stdout);
}
