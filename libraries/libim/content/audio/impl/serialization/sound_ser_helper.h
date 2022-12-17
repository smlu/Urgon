#ifndef LIBIM_SOUND_SER_HELPER_H
#define LIBIM_SOUND_SER_HELPER_H
#include "wav.h"
#include "indywv.h"
#include "../sound.h"

#include <libim/common.h>
#include <libim/io/stream.h>
#include <libim/types/safe_cast.h>

namespace libim::content::audio {
    enum class SoundFormatType
    {
        WAV,
        IndyWV,
        Unknown
    };

    static SoundFormatType parseWavHeader(const InputStream& istream, std::size_t& numChannels, std::size_t& sampleRate, std::size_t& sampleBitSize, std::size_t& dataSize)
    {
        if (istream.peek<decltype(kRiffChunkId)> () == kRiffChunkId) // parse wav header
        {
            /* Read whole WAV header */
            WavHeader wavHeader = istream.read<WavHeader>();
            if (wavHeader.format   != kWavFormatId ||
                wavHeader.fmt.tag  != kFmtChunkId  ||
                wavHeader.fmt.size != kWavFmtSize  ||
                wavHeader.fmt.audioFormat != AudioFormat::LPCM) {
                return SoundFormatType::Unknown;
            }

            /* Read data chunk header */
            auto data = istream.read<WavDataChunkHeader>();
            if (data.tag != kDataChunkId) {
                return SoundFormatType::Unknown;
            }

            numChannels   = wavHeader.fmt.numChannels;
            sampleRate    = wavHeader.fmt.sampleRate;
            sampleBitSize = wavHeader.fmt.sampleBitSize;
            dataSize      = data.size;
            return SoundFormatType::WAV;
        }
        else if (istream.peek<decltype(kIndyWV)> () == kIndyWV) // parse indy wv header
        {
            IndyWVHeader header = istream.read<IndyWVHeader>();
            numChannels   = header.numChannels;
            sampleRate    = header.sampleRate;
            sampleBitSize = header.sampleBitSize;
            dataSize      = header.dataSize;
            return SoundFormatType::IndyWV;
        }

        return SoundFormatType::Unknown;
    }

    static void wavWrite(OutputStream& ostream, std::size_t numChannels, std::size_t sampleRate, std::size_t sampleBitSize, ByteView data)
    {
        WavHeader wavHeader;
        wavHeader.fmt.size          = 16;
        wavHeader.fmt.audioFormat   = AudioFormat::LPCM;
        wavHeader.fmt.numChannels   = safe_cast<decltype(wavHeader.fmt.numChannels)>  (numChannels);
        wavHeader.fmt.sampleRate    = safe_cast<decltype(wavHeader.fmt.sampleRate)>   (sampleRate);
        wavHeader.fmt.blockAlign    = safe_cast<decltype(wavHeader.fmt.blockAlign)>   (numChannels * sampleBitSize / 8);
        wavHeader.fmt.byteRate      = safe_cast<decltype(wavHeader.fmt.byteRate)>     (sampleRate  * wavHeader.fmt.blockAlign);
        wavHeader.fmt.sampleBitSize = safe_cast<decltype(wavHeader.fmt.sampleBitSize)>(sampleBitSize);

        WavDataChunkView dataChunk;
        dataChunk.data = std::move(data);
        dataChunk.size = safe_cast<decltype(dataChunk.size)>(dataChunk.data.size());
        wavHeader.size = 36 + dataChunk.size; // 36 = sizeof(WavHeader) + sizeof riff header

        // Write to output
        ostream << wavHeader
                << dataChunk;
    }

    inline void wvWrite(OutputStream& ostream, std::size_t numChannels, std::size_t sampleRate, std::size_t sampleBitSize, ByteView sndData)
    {
        IndyWVHeader wv;
        wv.sampleRate     = safe_cast<decltype(wv.sampleRate)>(sampleRate);
        wv.sampleBitSize  = safe_cast<decltype(wv.sampleBitSize)>(sampleBitSize);
        wv.numChannels    = safe_cast<decltype(wv.numChannels)>(numChannels);
        wv.dataSize       = safe_cast<decltype(wv.dataSize)>(sndData.size());

        // Write to output
        ostream.write(wv);
        ostream.write(sndData);
    }
}
#endif // LIBIM_SOUND_SER_HELPER_H
