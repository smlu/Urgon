#include "../cnd.h"
#include "../../world_ser_common.h"
#include "cnd_sound_header.h"

#include <cstring>
#include <string>

#include <libim/content/audio/sound.h>
#include <libim/log/log.h>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::audio;
using namespace libim::content::audio::impl;
using namespace std::string_literals;

void CND::parseSection_Sounds(const InputStream& istream, SbTrack& track, uint32_t& fileIdNonce)
{
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
            world_ser_assert(h.dirNameOffset + kCndMaxNameLen <= track.ptrData->size(),
                "Sound dir name offset out of bounds"
            );
            world_ser_assert(h.dirNameOffset + kCndMaxNameLen <= track.ptrData->size(),
                "Sound file name offset out of bounds"
            );
            world_ser_assert(h.dataOffset + h.dataSize <= track.ptrData->size(),
                "Sound data offset out of bounds"
            );

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
            if(!r.second) {
                throw CNDError("parseSection_Sounds",
                    "Found duplicated sound file in soundbank with name: " + name
                );
            }
        }

        /* Read next file ID nonce */
        fileIdNonce = istream.read<uint32_t>();
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_Sounds",
            "An exception was encountered while importing soundbank: "s + e.what()
        );
    }
}
