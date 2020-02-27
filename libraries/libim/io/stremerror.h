#ifndef LIBIM_STREMERROR_H
#define LIBIM_STREMERROR_H
#include <stdexcept>

namespace libim {
    struct StreamError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    struct FileStreamError : public StreamError {
        using StreamError::StreamError;
    };
}

#endif // LIBIM_STREMERROR_H
