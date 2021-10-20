#ifdef __STDC_NO_THREADS__
#error Need support for C11 threading https://en.cppreference.com/w/c/thread
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define INPUT_BUFSIZE (1 * 1024 * 1024)
#define OUTPUT_BUFSIZE (4 * INPUT_BUFSIZE)

#define BASE_CHR ((uint8_t)'\t')

struct encoder_arg
{
    uint8_t* bufin;
    uint8_t* bufout;
    size_t limit;
}

#if 0

We want a function that maps:

     x |   y
--------------
0 (00) |  9
1 (01) | 10
2 (10) | 13
3 (11) | 32

Given 0 -> 9, we need an intercept term. 0, 1, and 2 are easily mapped to 9 +
0² = 9, 9 + 1² = 10, and 9 + 2² = 13, respectively. But, 3 maps to 9 + 3² = 18,
which means we need to add 14 only when x == 3. We'd like to do this without
using a conditional. Since bits 0 and 1 are both set only when x == 3,
(c & (c >> 1)) only evaluates to 1 in that case and gives us a way to
map 3 to 32.

#endif

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
ws_encode(uint8_t c, uint8_t* restrict out)
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
encode_buffer_interleaved(void *arg)
{
    const struct encoder_arg *argp = arg;

    for (size_t i = 0; i < argp.limit; i += 2) {
        ws_encode(argp.bufin[i], argp.bufout + 4 * i);
    }
}

static void encode_stream(FILE* fin, FILE* fout)
{
    uint8_t* bufin = alloc_with_fail(INPUT_BUFSIZE, "input buffer");
    uint8_t* bufout = alloc_with_fail(OUTPUT_BUFSIZE, "output buffer");

    size_t n;

    while ((n = fread(bufin, 1, INPUT_BUFSIZE, fin))) {
        struct encoder_arg args[] {
            { bufin, bufout, n },
            { bufin + 1, bufout + 4, n },
        };

        thrd_t encoder_threads[sizeof(args)/sizeof(args[0])];

        for (size_t i = 0; i < sizeof(args)/sizeof(args[0]); ++i) {
            int status = thread_create(
              encoder_threads + i, encode_buffer_interleaved, args + i);
            if (status != thrd_success) {
                fprintf(stderr, "Failed to create thread %zu\n", i);
                perror("Error creating thread");
                exit(EXIT_FAILURE);
            }
        }

        for (size_t i = 0; i < sizeof(args) / sizeof(args[0]); ++i) {
            int status = thread_join(threads[i], NULL);
            if (status != thrd_success) {
                fprintf(stderr, "Failed to join thread %zu\n", i);
                perror("Error joining thread");
                exit(EXIT_FAILURE);
            }
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
