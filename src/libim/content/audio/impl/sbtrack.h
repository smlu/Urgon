#ifndef LIBIM_SBTRACK_H
#define LIBIM_SBTRACK_H
#include "../soundbank.h"

#include <memory>
#include <unordered_map>


namespace libim::content::audio::impl {
    struct SbTrack
    {
        std::unordered_map<std::string, Sound> sounds;
        std::shared_ptr<ByteArray> ptrData;
    };
}
#endif // LIBIM_SBTRACK_H
