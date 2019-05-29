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
        SoundBank(std::size_t nTracks);
        SoundBank(const SoundBank&) = delete;
        SoundBank& operator=(const SoundBank&) = delete;
        ~SoundBank();

        std::size_t count() const; // !< Returns number of tracks

        const std::unordered_map<std::string, Sound>& getTrack(std::size_t trackIdx) const;

        /** Imports sounbank data to track at index Idx */
        bool importTrack(std::size_t trackIdx, const InputStream& istream);

    private:
        struct SoundBankImpl;
        std::unique_ptr<SoundBankImpl> ptrImpl_;
    };
}


#endif // LIBIM_SOUNDBANK_H
