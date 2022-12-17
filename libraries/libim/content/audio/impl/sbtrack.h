#ifndef LIBIM_SBTRACK_H
#define LIBIM_SBTRACK_H
#include "serialization/soundinfo.h"
#include "serialization/sound_ser_helper.h"
#include "soundcache.h"
#include "../sound.h"
#include "../soundbank_error.h"

#include <libim/log/log.h>
#include <libim/io/stream.h>
#include <libim/math/math.h>
#include <libim/types/indexmap.h>
#include <libim/utils/utils.h>

#include <sstream>
#include <string>
#include <memory>

namespace libim::content::audio {
    template<typename ExceptionT = SoundBankError, typename... Args>
    inline const void check(bool pred, const char* message,  Args&&... args)
    {
        if(!pred)
        {
            using namespace libim::utils;
            auto msg = format(message, std::forward<Args>(args)...);
            throw ExceptionT(msg);
        }
    };

    struct SoundBankTrack
    {
        IndexMap<Sound> sounds;
        std::shared_ptr<SoundCache> data;

        SoundBankTrack() :
            data(std::make_shared<SoundCache>())
        {}

        Sound& addSound(const SoundInfo& sndInfo)
        {
            check(sndInfo.pathOffset + SoundCache::kMaxStringLen <= data->size(),
                "Sound path offset out of bounds (offset=%, size=%, data_size=%)",
                 sndInfo.pathOffset, SoundCache::kMaxStringLen, data->size()
            );
            check(sndInfo.nameOffset + SoundCache::kMaxStringLen <= data->size(),
                "Sound name offset out of bounds (offset=%, size=%, data_size=%)",
                 sndInfo.nameOffset, SoundCache::kMaxStringLen, data->size()
            );
            check(sndInfo.dataOffset + sndInfo.dataSize <= data->size(),
                "Sound data offset out of bounds (offset=%, size=%, data_size=%)",
                 sndInfo.dataOffset, sndInfo.dataSize, data->size()
            );

            Sound snd (
                sndInfo.hSnd,
                sndInfo.idx,
                sndInfo.sampleRate,
                sndInfo.sampleBitSize,
                sndInfo.numChannels,
                this->data,
                sndInfo.pathOffset,
                sndInfo.nameOffset,
                sndInfo.dataOffset,
                sndInfo.dataSize,
                sndInfo.bCompressed
            );

            std::string name(snd.name());
            auto r = sounds.pushBack(name, std::move(snd));
            if (!r.second) {
                LOG_WARNING("SoundBankTrack:addSound: Found duplicate sound name '%', skipping!", name);
            }
            return *r.first;
        }

        Sound& loadSound(const InputStream& istream)
        {
            auto getNameOffset =[](std::string_view path) {
                auto offset = path.find_last_of('\\');
                if (offset == std::string_view::npos) {
                    offset = path.find_last_of('/');
                }
                return offset == std::string_view::npos ? 0 : offset + 1;
            };

            auto soundFilePath = "sound\\" + istream.name();
            auto nameOffset = getNameOffset(soundFilePath);
            if (auto it = sounds.find(std::string_view{ &soundFilePath[nameOffset] }); it != sounds.end()) {
                return *it;
            }

            std::size_t numChannels, sampleRate,
                        sampleBitSize, dataSize = 0;
            auto sndType = parseWavHeader(istream, numChannels, sampleRate, sampleBitSize, dataSize);
            if (sndType == SoundFormatType::Unknown) {
                throw SoundBankError(
                    utils::format("SoundBank: Can't load sound '%' from stream, unknown sound format!", istream.name())
                );
            }

            auto pathOffset = data->write(soundFilePath);
            auto dataOffset = data->write(dataSize, [&](byte_t* pOut, std::size_t size){
                if (istream.read(pOut, size) != size) {
                    throw SoundBankError(
                        utils::format("SoundBank: Failed to read sound data '%' from stream!", istream.name())
                    );
                }
                return size;
            });

            nameOffset += pathOffset;
            const auto soundIdx = sounds.size();
            const bool isCompressed = sndType == SoundFormatType::IndyWV;

            Sound snd(
                SoundHandle(0),
                soundIdx,
                sampleRate,
                sampleBitSize,
                numChannels,

                data,
                pathOffset,
                nameOffset,
                dataOffset,
                dataSize,
                isCompressed
            );

            std::string name(snd.name());
            return *sounds.pushBack(name, std::move(snd)).first;
        }
    };
}
#endif // LIBIM_SBTRACK_H
