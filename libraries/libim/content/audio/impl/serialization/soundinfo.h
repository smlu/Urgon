#ifndef LIBIM_SOUND_INFO_H
#define LIBIM_SOUND_INFO_H
#include <cstdint>
#include <libim/content/audio/sound.h>

#include "../sound_data.h"

namespace libim::content::audio {
    // Sound info structure stored in CND files.
    struct SoundInfo final
    {
        SoundHandle hSnd;          // sound handle
        uint32_t bankIdx;
        uint32_t pathOffset;
        uint32_t nameOffset;
        uint32_t dataOffset;
        uint32_t pLipSyncData;  // unused in serialization
        uint32_t sampleRate;
        uint32_t sampleBitSize;
        uint32_t numChannels;
        uint32_t dataSize;
        uint32_t bCompressed;
        uint32_t idx;

        SoundInfo() = default;
        SoundInfo(const Sound::SoundData& sound)
        {
            hSnd          = sound.handle;
            pathOffset    = sound.pathOffset;
            nameOffset    = sound.nameOffset;
            dataOffset    = sound.dataOffset;
            pLipSyncData  = 0;
            sampleRate    = sound.sampleRate;
            sampleBitSize = sound.sampleBitSize;
            numChannels   = sound.numChannels;
            dataSize      = sound.dataSize;
            bCompressed   = sound.isCompressed;
            idx           = sound.idx;
        }

        SoundInfo(const Sound& sound) : SoundInfo(*sound.ptrData_)
        {}
    };
    static_assert(sizeof(SoundInfo) == 48);
}
#endif // LIBIM_CND_SOUND_INFO_H
