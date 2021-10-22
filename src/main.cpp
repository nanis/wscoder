#include <cstdio>
#include <cstdlib>
#include <string>

#include "ws.hpp"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
static void
set_binmode_io()
{
    _setmode(fileno(stdin), O_BINARY);
    _setmode(fileno(stdout), O_BINARY);
    _setmode(fileno(stderr), O_BINARY);
}
#else
static void
set_binmode_io()
{}
#endif

static size_t
num_threads_from_env()
{
    if (const char* v = std::getenv(NUM_THREADS_VAR)) {
        return std::stoull(v);
    }

    return NUM_THREADS_DEFAULT;
}

int
main(void)
{
    set_binmode_io();

    try {
        WS_OPERATION(stdin, stdout, num_threads_from_env());
    } catch (const std::exception& e) {
        fputs(e.what(), stderr);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
