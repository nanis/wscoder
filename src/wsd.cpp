#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

#include "ws.hpp"

static uint8_t
ws_to_half_nibble(const uint8_t& ws)
{
    return ((7 * ws) >> 6) + !!(ws & 4);
}

static uint8_t
ws_decode(const uint8_t* buf)
{
    return (ws_to_half_nibble(buf[0]) << 6) | (ws_to_half_nibble(buf[1]) << 4) |
           (ws_to_half_nibble(buf[2]) << 2) | ws_to_half_nibble(buf[3]);
}

static void
decode_buffer_interleaved(const uint8_t* const bufin,
                          uint8_t* bufout,
                          const size_t limit,
                          size_t num_threads)
{
    for (size_t i = 0; i < limit; i += 4 * num_threads) {
        bufout[i / 4] = ws_decode(bufin + i);
    }
}

void
decode_stream(std::FILE* in, std::FILE* out, size_t num_threads)
{
    uint8_t* bufin = new uint8_t[ENCODED_BUFSIZE];
    uint8_t* bufout = new uint8_t[DECODED_BUFSIZE];

    std::vector<std::thread> decoders(num_threads);
    size_t n;

    while ((n = std::fread(bufin, 1, ENCODED_BUFSIZE, in))) {
        for (size_t i = 0; i < num_threads; ++i) {
            decoders[i] = std::thread(decode_buffer_interleaved,
                                      bufin + 4 * i,
                                      bufout + i,
                                      n,
                                      num_threads);
        }

        for (auto& decoder : decoders) {
            decoder.join();
        }

        std::fwrite(bufout, 1, n / 4, out);
    }

    std::fflush(out);
    delete[] bufout;
    delete[] bufin;
}
