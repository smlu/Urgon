#ifndef LIBIM_SOUND_SER_HELPER_H
#define LIBIM_SOUND_SER_HELPER_H
#include "wav.h"
#include "indywv.h"
#include "../sound.h"
#include "../../../io/stream.h"


namespace libim::content::audio {
    void SoundSerializeAsWAV(OutputStream& ostream, std::size_t numChannels, std::size_t sampleRate, std::size_t bitsPerSample, ByteArray data)
    {
        WavHeader wavHeader;
        wavHeader.fmt.size          = 16;
        wavHeader.fmt.audioFormat   = AudioFormat::LPCM;
        wavHeader.fmt.numChannels   = numChannels;                            // TODO: safe cast
        wavHeader.fmt.sampleRate    = sampleRate;                             // TODO: safe cast
        wavHeader.fmt.blockAlign    = numChannels * bitsPerSample / 8;        // TODO: safe cast
        wavHeader.fmt.byteRate      = sampleRate  * wavHeader.fmt.blockAlign; // TODO: safe cast
        wavHeader.fmt.bitsPerSample = bitsPerSample;                          // TODO: safe cast

        WavDataChunk dataChunk;
        dataChunk.data = std::move(data);
        dataChunk.size = dataChunk.data.size(); // TODO: safe cast
        wavHeader.size = 36 + dataChunk.size;

        // Write to output
        ostream << wavHeader
                << dataChunk;
    }

    void SoundSerializeAsIndyWV(OutputStream& ostream, std::size_t numChannels, std::size_t sampleRate, std::size_t bitsPerSample,std::shared_ptr<ByteArray> ptrData, std::size_t dataOffset, std::size_t dataSize)
    {
        IndyWVFileHeader wv;
        wv.sampleRate     = sampleRate;    // TODO: safe cast
        wv.sampleBitSize  = bitsPerSample; // TODO: safe cast
        wv.numChannels    = numChannels;
        wv.dataSize       = dataSize;

        // Write to output
        ostream.write(wv);
        ostream.write(&ptrData->at(dataOffset), dataSize);
    }
}
#endif // LIBIM_SOUND_SER_HELPER_H
