#ifndef LIBIM_INDYWV_H
#define LIBIM_INDYWV_H
#include <array>
#include <assert.h>
#include <cstdint>

#include <libim/common.h>
#include <libim/log/log.h>
#include <libim/io/binarystream.h>
#include <libim/io/stream.h>


namespace libim::content::audio {
    constexpr std::array<char, 4> kWVSM = { 'W', 'V', 'S', 'M' };
    constexpr std::array<char, 6> kIndyWV = { 'I', 'N', 'D', 'Y', 'W', 'V' };

    PACKED(struct IndyWVFileHeader
    {
        std::array<char, 6> tag = kIndyWV;
        int32_t sampleRate     = 0;
        int32_t sampleBitSize  = 0;
        int32_t numChannels    = 0;
        int32_t dataSize       = 0;
        int32_t unknown        = 0;
    });
    static_assert(sizeof(IndyWVFileHeader) == 26);

    struct VWCompressorState
    {
        byte_t  unknown1;
        int16_t unknown2;
        byte_t  unknown3;
        int16_t unknown4;
    };

    struct IndyVW
    {
        static int16_t swap16(int16_t x) // TODO: move to utils
        {
            int16_t hi = (x & 0xff00) >> 8;
            int16_t lo = (x & 0xff);
            return static_cast<int16_t>(lo << 8) | hi;
        }

        static ByteArray inflate(const InputStream& istream)
        {
            std::size_t mode = 1;
            VWCompressorState state{};

            std::size_t infSize  = istream.read<uint32_t>();
            auto unknown1 = istream.read<int8_t>();
            auto unknown2 = istream.read<int16_t>();

            if(unknown1 < 0)
            {
                state.unknown1 = ~unknown1;
                mode = 2;
            }

            state.unknown2 = swap16(unknown2);
            if(mode > 1)
            {
                state.unknown3 = istream.read<byte_t>();
                state.unknown4 = swap16(istream.read<int16_t>());
            }

            ByteArray data;
            data.reserve(infSize);
            OutputBinaryStream obs(data);

            if(mode == 2 &&
               state.unknown2 == 0x1111 &&
               state.unknown3 == 0x64   &&
               state.unknown4 == 0x2222 &&
               istream.peek<decltype(kWVSM)>() == kWVSM)
            {
                // WVSM decompression
                istream.advance(kWVSM.size());

                constexpr std::size_t nFrameSize = 4096;
                for (std::size_t i = 0; i < infSize / nFrameSize; i++) {
                    wvsmInflateBlock(istream, nFrameSize, obs);
                }

                /* Read the remaining data, shorter than one frame */
                wvsmInflateBlock(istream, infSize % nFrameSize, obs);
                assert(obs.size() == infSize && obs.tell() == obs.size());
            }
            else {
                // TODO: Implement ADPCM decompression method
                LOG_ERROR("IndyVW: Cannot inflate sound data, unknown compression mode!");
            }

            return data;
        }

    private:
        static void wvsmInflateBlock(const InputStream& istream, std::size_t nFrameSize, OutputStream& dest)
        {
            std::size_t nSamples = nFrameSize / 2;
            if(nSamples == 0){
                return;
            }

            [[maybe_unused]]auto unknown = istream.read<uint16_t>();
            const byte_t sampleExpander = istream.read<byte_t>();
            const int16_t salo = sampleExpander & 0xF;
            const int16_t sahi = sampleExpander >> 4;

            auto get_sample = [&](int16_t expander) -> int16_t
            {
                int16_t val = istream.read<byte_t>();
                if(val == 0x80)
                {
                    auto s = istream.read<int16_t>();
                    val = swap16(s);
                }
                else {
                    val = static_cast<int8_t>(val) << expander;
                }
                return val;
            };

            for(std::size_t i = 0; i < nSamples; i += 2)
            {
                dest << get_sample(sahi);
                if( i + 1 >= nSamples) return;
                dest << get_sample(salo);
            }
        }
    };
}
#endif // LIBIM_INDYWV_H
