#ifndef LIBIM_SBTRACK_H
#define LIBIM_SBTRACK_H
#include "../sound.h"
#include "../../../utils/hashmap.h"
#include <memory>
#include <unordered_map>


namespace libim::content::audio::impl {
    struct SbTrack
    {
        utils::HashMap<Sound> sounds;
        std::shared_ptr<ByteArray> ptrData;
    };
}
#endif // LIBIM_SBTRACK_H
