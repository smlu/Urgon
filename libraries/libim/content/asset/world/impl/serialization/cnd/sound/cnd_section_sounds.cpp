#include "cnd_sound_header.h"
#include "../cnd.h"

#include <cstring>
#include <libim/content/audio/sound.h>
#include <libim/log/log.h>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::audio;
using namespace libim::content::audio::impl;

uint32_t CND::parseSectionSounds(SbTrack& track, const InputStream& istream)
{
    uint32_t nonce = 0;

    try
    {
        track.sounds.clear();
        track.ptrData.reset();

        if(istream.tell() != sizeof(CndHeader)){
            istream.seek(sizeof(CndHeader));
        }

        std::size_t nSounds = istream.read<uint32_t>();
        std::size_t nSoundDataSize = istream.read<uint32_t>();

        auto vecHeaders = istream.read<std::vector<CndSoundHeader>>(nSounds);
        track.ptrData   = istream.read<decltype(track.ptrData)>(nSoundDataSize);

        /* Read and convert sound headers */
        track.sounds.reserve(nSounds);
        for(const auto& h : vecHeaders)
        {
            Sound s (
                track.ptrData,
                h.dirNameOffset,
                h.fileNameOffset,
                h.dataOffset,
                h.dataSize
            );

            s.setId(h.fileID);
            s.setIdx(h.index);
            s.setSampleRate(h.sampleRate);
            s.setBitsPerSample(h.bitsPerSample);
            s.setChannels(h.numChannels);
            s.setIndyWVFormat(h.isIndyWVFormat);

            std::string name(s.name());
            auto r = track.sounds.pushBack(name, std::move(s));
            if(!r.second)
            {
                LOG_ERROR("CND Error: Soundbank already contains sound file: '%'!", name);
                return nonce;
            }
        }

        /* Read nonce */
        nonce = istream.read<uint32_t>();
        return nonce;
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("CND Error: An exception was thrown while importing soundbank from CND file stream: %!", e.what());
        return nonce;
    }
}
