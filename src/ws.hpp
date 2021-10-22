#ifndef WSCODER_HPP_INCLUDED

#include <cstdlib>

constexpr uint8_t BASE_CHR = static_cast<uint8_t>('\t');
constexpr size_t DECODED_BUFSIZE = 1 * 1024 * 1024;
constexpr size_t ENCODED_BUFSIZE = 4 * DECODED_BUFSIZE;
constexpr size_t NUM_THREADS_DEFAULT = 2;

#define NUM_THREADS_VAR "WS_NUM_THREADS"

void(WS_OPERATION)(FILE*, FILE*, size_t);

#define WSCODER_HPP_INCLUDED
#endif
