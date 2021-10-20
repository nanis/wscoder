#ifndef WSCODER_HPP_INCLUDED

constexpr size_t DECODED_BUFSIZE = 1 * 1024 * 1024;
constexpr size_t ENCODED_BUFSIZE = 4 * DECODED_BUFSIZE;
constexpr size_t NUM_THREADS = WS_NUM_THREADS;
constexpr uint8_t BASE_CHR = static_cast<uint8_t>('\t');

#define WSCODER_HPP_INCLUDED
#endif
