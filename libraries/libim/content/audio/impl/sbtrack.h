#ifndef LIBIM_SBTRACK_H
#define LIBIM_SBTRACK_H
#include "../sound.h"
#include <libim/types/indexmap.h>

#include <memory>
#include <unordered_map>


namespace libim::content::audio::impl {
    struct SbTrack
    {
        IndexMap<Sound> sounds;
        std::shared_ptr<ByteArray> ptrData;
    };
}
#endif // LIBIM_SBTRACK_H
