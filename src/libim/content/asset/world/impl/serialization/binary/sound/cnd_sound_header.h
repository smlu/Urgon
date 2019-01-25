#ifndef LIBIM_CND_SOUND_HEADER_H
#define LIBIM_CND_SOUND_HEADER_H
#include <cstdint>

namespace libim::content::asset {
    struct CndSoundHeader final
    {
        uint32_t fileID;
        uint32_t bankIdx;
        uint32_t dirNameOffset;
        uint32_t fileNameOffset;
        uint32_t dataOffset;
        uint32_t unknown2;
        uint32_t sampleRate;
        uint32_t bitsPerSample;
        uint32_t numChannels;
        uint32_t dataSize;
        uint32_t isIndyWVFormat;
        uint32_t index;
    };
}

#endif // LIBIM_CND_SOUND_HEADER_H
