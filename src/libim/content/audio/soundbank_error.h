#ifndef LIBIM_SOUNDBANK_ERROR_H
#define LIBIM_SOUNDBANK_ERROR_H
#include <stdexcept>

namespace libim::content::audio {
    struct SoundBankError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

}

#endif // LIBIM_SOUNDBANK_ERROR_H
