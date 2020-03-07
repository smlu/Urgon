#ifndef LIBIM_SOUND_SER_HELPER_H
#define LIBIM_SOUND_SER_HELPER_H
#include "wav.h"
#include "indywv.h"
#include "../sound.h"
#include <libim/io/stream.h>
#include <libim/types/safe_cast.h>

namespace libim::content::audio {
    void SoundSerializeAsWAV(OutputStream& ostream, std::size_t numChannels, std::size_t sampleRate, std::size_t bitsPerSample, ByteArray data)
    {
        WavHeader wavHeader;
        wavHeader.fmt.size          = 16;
        wavHeader.fmt.audioFormat   = AudioFormat::LPCM;
        wavHeader.fmt.numChannels   = safe_cast<decltype(wavHeader.fmt.numChannels)>  (numChannels);
        wavHeader.fmt.sampleRate    = safe_cast<decltype(wavHeader.fmt.sampleRate)>   (sampleRate);
        wavHeader.fmt.blockAlign    = safe_cast<decltype(wavHeader.fmt.blockAlign)>   (numChannels * bitsPerSample / 8);
        wavHeader.fmt.byteRate      = safe_cast<decltype(wavHeader.fmt.byteRate)>     (sampleRate  * wavHeader.fmt.blockAlign);
        wavHeader.fmt.bitsPerSample = safe_cast<decltype(wavHeader.fmt.bitsPerSample)>(bitsPerSample);

        WavDataChunk dataChunk;
        dataChunk.data = std::move(data);
        dataChunk.size = safe_cast<decltype(dataChunk.size)>(dataChunk.data.size());
        wavHeader.size = 36 + dataChunk.size;

        // Write to output
        ostream << wavHeader
                << dataChunk;
    }

    void SoundSerializeAsIndyWV(OutputStream& ostream, std::size_t numChannels, std::size_t sampleRate, std::size_t bitsPerSample,std::shared_ptr<ByteArray> ptrData, std::size_t dataOffset, std::size_t dataSize)
    {
        IndyWVFileHeader wv;
        wv.sampleRate     = safe_cast<decltype(wv.sampleRate)>(sampleRate);
        wv.sampleBitSize  = safe_cast<decltype(wv.sampleBitSize)>(bitsPerSample);
        wv.numChannels    = numChannels;
        wv.dataSize       = dataSize;

        // Write to output
        ostream.write(wv);
        ostream.write(&ptrData->at(dataOffset), dataSize);
    }
}
#endif // LIBIM_SOUND_SER_HELPER_H
