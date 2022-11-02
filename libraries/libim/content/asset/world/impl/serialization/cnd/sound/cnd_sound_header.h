#ifndef LIBIM_CND_SOUND_HEADER_H
#define LIBIM_CND_SOUND_HEADER_H
#include <cstdint>

namespace libim::content::asset {
    struct CndSoundHeader final
    {
        uint32_t hSnd;          // sound handle
        uint32_t bankIdx;
        uint32_t dirNameOffset;
        uint32_t fileNameOffset;
        uint32_t dataOffset;
        uint32_t pLipSyncData;  // unused in serialization
        uint32_t sampleRate;
        uint32_t bitsPerSample;
        uint32_t numChannels;
        uint32_t dataSize;
        uint32_t bCompressed;
        uint32_t idx;
    };
    static_assert(sizeof(CndSoundHeader) == 48);
}

#endif // LIBIM_CND_SOUND_HEADER_H
