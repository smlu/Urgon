#ifndef LIBIM_SOUNDBANK_SERIALIZER_H
#define LIBIM_SOUNDBANK_SERIALIZER_H
#include "soundinfo.h"
#include "../sbtrack.h"
#include "../sound_data.h"

#include <libim/common.h>
#include <libim/content/audio/soundbank_error.h>
#include <libim/log/log.h>
#include <libim/io/stream.h>
#include <libim/types/safe_cast.h>

#include <sstream>
#include <string>

using namespace libim;
using namespace libim::utils;

namespace libim::content::audio {

    static void readSoundBankTrack(const InputStream& istream, SoundBankTrack& track)
    {
        try
        {
            std::size_t nSounds     = istream.read<uint32_t>();
            std::size_t sndDataSize = istream.read<uint32_t>();

            auto sndInfos = istream.read<std::vector<SoundInfo>>(nSounds);
            *track.data   = istream.read<ByteArray>(sndDataSize);

            /* Read and convert sound headers */
            track.sounds.clear();
            track.sounds.reserve(nSounds);
            for(const auto& sndInfo : sndInfos) {
                track.addSound(sndInfo);
            }
        }
        catch(const std::exception& e) {
            throw SoundBankError(
                format("An exception was encountered while importing soundbank track: %", e.what())
            );
        }
    }

    static void writeSoundBankTrack(OutputStream& ostream, const SoundBankTrack& track, std::size_t trackIdx)
    {
        try
        {
            /* Make list of sound infos */
            std::vector<SoundInfo> sndInfos;
            sndInfos.reserve(track.sounds.size());
            for (const auto& snd : track.sounds)
            {
                SoundInfo sndInfo = snd;
                if (track.isStatic) {
                    sndInfo.idx = makeStaticResourceIdx(sndInfo.idx); // required by the engine. dumb, dumb, dumb...
                }

                sndInfo.bankIdx = safe_cast<decltype(sndInfo.bankIdx)>(trackIdx);
                sndInfos.push_back(std::move(sndInfo));
            }

            /* Write data to stream */
            ostream.write<uint32_t>(safe_cast<uint32_t>(sndInfos.size()));
            ostream.write<uint32_t>(safe_cast<uint32_t>(track.data->size()));
            ostream.write(sndInfos);
            ostream.write(track.data->getDataView(0, track.data->size()));
        }
        catch(const std::exception& e)
        {
            throw SoundBankError(
                format("An exception was encountered while writing soundbank track: %", e.what())
            );
        }
    }

    static void skipSerializedSoundBank(const InputStream& istream)
    {
        std::size_t numSoundInfos = istream.read<uint32_t>();
        std::size_t sizeSoundData = istream.read<uint32_t>();
        constexpr std::size_t sizeNextHandleField = sizeof(uint32_t);

        const std::size_t size =
            numSoundInfos * sizeof(SoundInfo) +
            sizeSoundData                     +
            sizeNextHandleField;

        istream.advance(size);
    }
}
#endif // LIBIM_SOUNDBANK_SERIALIZER_H
