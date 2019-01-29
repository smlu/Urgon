#ifndef LIBIM_SOUNDBANK_INSTANCE_H
#define LIBIM_SOUNDBANK_INSTANCE_H
#include "../soundbank.h"

#include <memory>
#include <unordered_map>


namespace libim::content::audio::impl {
    struct SoundBankInstance
    {
        std::unordered_map<std::string, Sound> sounds;
        std::shared_ptr<ByteArray> ptrData;
    };
}
#endif // LIBIM_SOUNDBANK_INSTANCE_H
