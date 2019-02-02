#ifndef LIBIM_SOUNDBANK_H
#define LIBIM_SOUNDBANK_H
#include "sound.h"
#include "../../common.h"
#include "../../io/stream.h"

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace libim::content::audio
{
    class SoundBank final
    {
    public:
        SoundBank(std::size_t nInstances);
        SoundBank(const SoundBank&) = delete;
        SoundBank& operator =(const SoundBank&) = delete;
        ~SoundBank();

        std::size_t count() const; // !< Returns number of instances

        const std::unordered_map<std::string, Sound>& getSounds(std::size_t instanceIdx) const;

        /** Imports sounbank data to instance at index Idx */
        bool importBank(std::size_t instanceIdx, const InputStream& istream);

    private:
        struct SoundBankImpl;
        std::unique_ptr<SoundBankImpl> ptrImpl_;
    };
}


#endif // LIBIM_SOUNDBANK_H
