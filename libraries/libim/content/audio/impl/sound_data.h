#ifndef LIBIM_SOUND_DATA_H
#define LIBIM_SOUND_DATA_H
#include <cstdint>
#include <memory>

#include "../sound.h"
#include "soundcache.h"

#include "serialization/sound_ser_helper.h"

namespace libim::content::audio
{
    struct Sound::SoundData
    {
        SoundHandle handle = SoundHandle(0);
        uint32_t idx = 0;
        uint32_t sampleRate = 0;
        uint32_t sampleBitSize = 0;
        uint32_t numChannels = 0;
        bool isCompressed = false;

        std::size_t pathOffset;
        std::size_t nameOffset;
        std::size_t dataOffset;
        std::size_t dataSize;

        std::weak_ptr<SoundCache> wptrData;

        std::string_view name() const
        {
            auto ptrData = lockOrThrow();
            return ptrData->getString(nameOffset);
        }

        inline bool isValid() const
        {
            auto ptrData = lock();
            return ptrData && isValid(*ptrData);
        }

        bool isValid(const SoundCache& data) const
        {
            return
                dataOffset + dataSize  <= data.size() &&
                pathOffset    < data.size()            &&
                pathOffset    <= nameOffset            &&
                nameOffset    < data.size()            &&
                sampleRate    > 0                      &&
                sampleBitSize > 0                      &&
                numChannels   > 0;
        }

        std::shared_ptr<SoundCache> lock() const
        {
            return wptrData.lock();
        }

        std::shared_ptr<SoundCache> lockOrThrow() const
        {
            auto ptrData = lock();
            if (!ptrData) {
                throw std::logic_error("Dead sound data cache");
            }
            return ptrData;
        }

        /** Returns uncompressed sound data */
        ByteArray data() const
        {
            // Returns decompressed sound data.
            ByteArray sndData;
            auto ptrData = lockOrThrow();
            if (!isValid(*ptrData)) {
                return sndData;
            }

            if (isCompressed) {
                sndData = IndyVW::inflate(InputBinaryStream(ptrData->getDataView(dataOffset, dataSize)));
            }
            else {
                sndData = ptrData->read(dataOffset, dataSize);
            }

            return sndData;
        }

        void wavWrite(OutputStream& ostream) const
        {
            auto data = this->data(); // Note, data is returned as decompressed data.
            if(data.empty() && dataSize > 0) {
                throw StreamError("Cannot write invalid sound to stream in WAV format");
            }
            audio::wavWrite(ostream, numChannels, sampleRate, sampleBitSize, data);
        }

        inline void wavWrite(OutputStream&& ostream) const
        {
            wavWrite(ostream);
        }

        void wvWrite(OutputStream& ostream) const
        {
             // TODO: implement converting of data to indyWV format
            if (!isCompressed) {
                throw StreamError("Cannot write uncompressed sound to stream in WV format");
            }

            auto ptrData = lockOrThrow();
            if(!isValid(*ptrData)) {
                throw StreamError("Cannot write invalid sound to stream in WV format");
            }
            audio::wvWrite(ostream, numChannels, sampleRate, sampleBitSize, ptrData->getDataView(dataOffset, dataSize));
        }

        inline void wvWrite(OutputStream&& ostream) const
        {
            wvWrite(ostream);
        }
    };
}
#endif // LIBIM_SOUND_DATA_H
