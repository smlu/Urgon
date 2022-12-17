#ifndef LIBIM_SOUND_H
#define LIBIM_SOUND_H
#include <libim/common.h>
#include <libim/io/stream.h>

#include <memory>
#include <string_view>

namespace libim::content::audio {
    class SoundBank;
    class SoundCache;

    enum class SoundHandle : uint32_t {};

    class Sound
    {
    public:
        Sound(Sound&&) noexcept;
        Sound& operator =(Sound&&) noexcept;
        Sound(const Sound&);
        Sound& operator =(const Sound&);

        std::string_view name() const; // Warning, dangling reference if underlying data is destroyed before this object.

        SoundHandle handle() const;
        uint32_t idx() const;
        std::size_t sampleRate() const;
        std::size_t sampleBitSize() const;
        std::size_t channels() const;
        std::size_t dataSize() const;
        bool isCompressed() const;
        bool isValid() const;

    protected:
        Sound();
        Sound(std::weak_ptr<SoundCache> wptrCacheData, std::size_t pathOffset,
              std::size_t nameOffset, std::size_t sndDataOffset, std::size_t sndDataSize);

        Sound(SoundHandle handle, uint32_t idx, uint32_t sampleRate, uint32_t sampleBitSize, uint32_t numChannels,
              std::weak_ptr<SoundCache> wptrCacheData, std::size_t pathOffset, std::size_t nameOffset,
              std::size_t sndDataOffset, std::size_t sndDataSize, bool isCompressed);

        std::shared_ptr<SoundCache> lockOrThrow() const;
        bool isValid(const SoundCache& data) const;
        ByteArray data() const;

    private:
        friend struct SoundInfo;
        friend struct SoundBank;
        friend struct SoundBankTrack;
        friend void wavWrite(OutputStream& ostream,  const Sound& sound);
        friend void wavWrite(OutputStream&& ostream, const Sound& sound);
        friend void wvWrite(OutputStream& ostream,  const Sound& sound);
        friend void wvWrite(OutputStream&& ostream, const Sound& sound);

        struct SoundData;
        std::unique_ptr<SoundData> ptrData_;
    };

    void wavWrite(OutputStream& ostream, const Sound& sound);
    void wavWrite(OutputStream&& ostream, const Sound& sound);

    void wvWrite(OutputStream& ostream, const Sound& sound);
    void wvWrite(OutputStream&& ostream, const Sound& sound);
}
#endif // LIBIM_SOUND_H
