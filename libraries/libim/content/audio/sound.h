#ifndef LIBIM_SOUND_H
#define LIBIM_SOUND_H
#include <libim/common.h>
#include <libim/io/stream.h>

#include <memory>
#include <string_view>

namespace libim::content::audio {

    class Sound
    {
    public:
        Sound() = default;
        Sound(std::weak_ptr<ByteArray> wptrBankData, std::size_t dirNameOffset, std::size_t nameOffset, std::size_t dataOffset, std::size_t dataSize);
        Sound(Sound&&) noexcept = default;
        Sound& operator =(Sound&&) noexcept = default;
        Sound(const Sound&) = default;
        Sound& operator =(const Sound&) = default;

        std::string_view name() const;

        uint32_t setId(uint32_t id)
        {
            return id_ = id;
        }

        uint32_t id() const
        {
            return id_;
        }

        void setIdx(uint32_t idx)
        {
            idx_ = idx;
        }

        uint32_t idx() const
        {
            return idx_;
        }

        void setSampleRate(std::size_t rate)
        {
            sampleRate_ = rate;
        }

        std::size_t sampleRate() const
        {
            return sampleRate_;
        }

        void setBitsPerSample(std::size_t bps)
        {
            bitsPerSample_ = bps;
        }

        std::size_t bitsPerSample() const
        {
            return bitsPerSample_;
        }

        void setChannels(std::size_t num)
        {
            numChannels_ = num;
        }

        std::size_t channels() const
        {
            return numChannels_;
        }

        void setIndyWVFormat(bool isiwvFormat)
        {
            isIndyWVFormat_ = isiwvFormat;
        }

        bool isIndyWVFormat() const
        {
            return isIndyWVFormat_;
        }

        bool isValid() const;

    private:
        std::shared_ptr<ByteArray> lockOrThrow() const;
        bool isValid(const ByteArray& data) const;
        ByteArray data() const;

    private:
        uint32_t id_  = 0;
        uint32_t idx_ = 0;
        std::size_t dirNameOffset_;
        std::size_t nameOffset_;
        std::size_t dataOffset_;
        std::size_t dataSize_;
        std::size_t sampleRate_;
        std::size_t bitsPerSample_;
        std::size_t numChannels_;
        bool isIndyWVFormat_;

        std::weak_ptr<ByteArray> wptrData_;

        friend void wavWrite(OutputStream& ostream, const Sound& sound);
        friend void wavWrite(OutputStream&& ostream, const Sound& sound);
        friend void iwvWrite(OutputStream& ostream, const Sound& sound);
        friend void iwvWrite(OutputStream&& ostream, const Sound& sound);
    };

    void wavWrite(OutputStream& ostream, const Sound& sound);
    void wavWrite(OutputStream&& ostream, const Sound& sound);

    void iwvWrite(OutputStream& ostream, const Sound& sound);
    void iwvWrite(OutputStream&& ostream, const Sound& sound);
}


#endif // LIBIM_SOUND_H
