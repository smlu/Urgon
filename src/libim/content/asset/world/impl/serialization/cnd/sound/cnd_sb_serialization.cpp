#include "cnd_sound_header.h"
#include "../cnd.h"
#include "../../../../../../audio/sound.h"
#include "../../../../../../../log/log.h"
#include <cstring>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::audio;
using namespace libim::content::audio::impl;

uint32_t CND::ParseSectionSounds(SoundBankInstance& sbInstance, const InputStream& istream)
{
    uint32_t nonce = 0;

    try
    {
        sbInstance.sounds.clear();
        sbInstance.ptrData.reset();

        if(istream.tell() != sizeof(CndHeader)){
            istream.seek(sizeof(CndHeader));
        }

        std::size_t nSounds = istream.read<uint32_t>();
        std::size_t nSoundDataSize = istream.read<uint32_t>();

        auto vecHeaders    = istream.read<std::vector<CndSoundHeader>>(nSounds);
        sbInstance.ptrData = istream.read<decltype(sbInstance.ptrData)>(nSoundDataSize);

        /* Read and convert sound headers */
        sbInstance.sounds.reserve(nSounds);
        for(const auto& h : vecHeaders)
        {
            Sound s
            (
                sbInstance.ptrData,
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
            auto r = sbInstance.sounds.emplace(name, std::move(s));
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
