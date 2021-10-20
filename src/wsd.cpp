#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <thread>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include "ws.hpp"

// We want a function that maps:

//              x |  y
// -------------------
//  9 (0000_1001) | 0
// 10 (0000_1010) | 1
// 13 (0000_1101) | 2
// 32 (0010_0000) | 3

// In integer arithmetic, x/10 gets us there but we have to pad the result when
// x == 13. Division is costly, so instead of dividing by 10, we multiply by
// ceil(64/10) = 7 and look at how many 64s are in the result. Using 7 (instead
// of, e.g., 13, 26, or 0x1999999a) means we can do it all with uint8_t values
// with no up/downcasting. Note 13 is the only x value for which !!(ws & 4) is
// 1.

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
decode_buffer_interleaved(const uint8_t* const bufin,
                          uint8_t* bufout,
                          const size_t limit)
{
    for (size_t i = 0; i < limit; i += 4 * NUM_THREADS) {
        bufout[i / 4] = ws_decode(bufin + i);
    }
}

static void
decode_stream(FILE* fin, FILE* fout)
{
    uint8_t* bufin = (uint8_t*)alloc_with_fail(ENCODED_BUFSIZE, "input buffer");
    uint8_t* bufout =
      (uint8_t*)alloc_with_fail(DECODED_BUFSIZE, "output buffer");

    std::thread decoders[NUM_THREADS];

    size_t n;

    while ((n = fread(bufin, 1, ENCODED_BUFSIZE, fin))) {
        for (size_t i = 0; i < NUM_THREADS; ++i) {
            decoders[i] = std::thread(
              decode_buffer_interleaved, bufin + 4 * i, bufout + i, n);
        }

        for (auto& decoder : decoders) {
            decoder.join();
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
