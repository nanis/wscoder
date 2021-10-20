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

//      x |   y
// --------------
// 0 (00) |  9
// 1 (01) | 10
// 2 (10) | 13
// 3 (11) | 32

// Given 0 -> 9, we need an intercept term. 0, 1, and 2 are easily mapped to 9
// + 0² = 9, 9 + 1² = 10, and 9 + 2² = 13, respectively. But, 3 maps to 9 + 3²
// = 18, which means we need to add 14 only when x == 3. We'd like to do this
// without using a conditional. Since bits 0 and 1 are both set only when x ==
// 3, (c & (c >> 1)) only evaluates to 1 in that case and gives us a way to map
// 3 to 32.

// Caller is responsible for ensuring 0 <= c <= 3
static uint8_t
half_nibble_to_ws(uint8_t c)
{
    return (c * c) + 14 * (c & (c >> 1)) + BASE_CHR;
}

// I went big endian (highest half nibble first) without thinking much
// about it. The choice should not matter performance-wise, but the decoder
// needs to do the same. Alternatively, we can add a header to encoded files
// to detect endianness at decoding time. The encoding of "\t" would be suitable
// for this as would many others :-)

static void
ws_encode(const uint8_t& c, uint8_t* out)
{
    out[0] = half_nibble_to_ws((c >> 6) & 3);
    out[1] = half_nibble_to_ws((c >> 4) & 3);
    out[2] = half_nibble_to_ws((c >> 2) & 3);
    out[3] = half_nibble_to_ws(c & 3);
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
encode_buffer_interleaved(
        const uint8_t * const bufin,
        uint8_t * bufout,
        const size_t limit
        )
{
    for (size_t i = 0; i < limit; i += NUM_THREADS) {
        ws_encode(bufin[i], bufout + 4 * i);
    }
}

static void
encode_stream(FILE* fin, FILE* fout)
{
    uint8_t* bufin = (uint8_t*)alloc_with_fail(DECODED_BUFSIZE, "input buffer");

    uint8_t* bufout =
      (uint8_t*)alloc_with_fail(ENCODED_BUFSIZE, "output buffer");

    std::thread encoders[NUM_THREADS];

    size_t n;

    while ((n = fread(bufin, 1, DECODED_BUFSIZE, fin))) {
        for (size_t i = 0; i < NUM_THREADS; ++i) {
            encoders[i] = std::thread(
              encode_buffer_interleaved, bufin + i, bufout + 4 * i, n);
        }

        for (auto& encoder : encoders) {
            encoder.join();
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
