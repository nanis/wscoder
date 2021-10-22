#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

#include "ws.hpp"

static uint8_t
half_nibble_to_ws(const uint8_t& c)
{
    static const uint32_t x = (' ' << 24) |
        ('\r' << 16) |
        ('\n' << 8) |
        '\t'
    ;

    return (x >> (8 * c)) & 0xff;
}

// I went big endian (highest half nibble first) without thinking much
// about it. The choice should not matter performance-wise, but the decoder
// needs to do the same. Alternatively, we can add a header to encoded files
// to detect endianness at decoding time. The encoding of "\t" would be suitable
// for this as would many others :-)

static void
encode_one(uint8_t c, uint8_t* out)
{
    out[0] = half_nibble_to_ws((c >> 6) & 3);
    out[1] = half_nibble_to_ws((c >> 4) & 3);
    out[2] = half_nibble_to_ws((c >> 2) & 3);
    out[3] = half_nibble_to_ws(c & 3);
}

static void
encode_buffer_interleaved(const uint8_t* bufin,
                          uint8_t* bufout,
                          size_t limit,
                          size_t num_threads)
{
    for (size_t i = 0; i < limit; i += num_threads) {
        encode_one(bufin[i], bufout + 4 * i);
    }
}

void
encode_stream(std::FILE* in, std::FILE* out, size_t num_threads)
{
    uint8_t* bufin = new uint8_t[DECODED_BUFSIZE];
    uint8_t* bufout = new uint8_t[ENCODED_BUFSIZE];

    std::vector<std::thread> encoders(num_threads);
    size_t n;

    while ((n = fread(bufin, 1, DECODED_BUFSIZE, in))) {
        for (size_t i = 0; i < num_threads; ++i) {
            encoders[i] = std::thread(encode_buffer_interleaved,
                                      bufin + i,
                                      bufout + 4 * i,
                                      n,
                                      num_threads);
        }

        for (auto& encoder : encoders) {
            encoder.join();
        }

        std::fwrite(bufout, 1, 4 * n, out);
    }

    std::fflush(out);

    delete[] bufout;
    delete[] bufin;
}
