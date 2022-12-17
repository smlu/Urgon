#include "serialization/indywv.h"
#include "serialization/sound_ser_helper.h"
#include "sound_data.h"
#include "../sound.h"

#include <libim/io/stream.h>
#include <exception>

using namespace libim;
using namespace libim::content;
using namespace libim::content::audio;

namespace audio = libim::content::audio;

Sound::Sound()
{
    ptrData_ = std::make_unique<Sound::SoundData>();
}

Sound::Sound(std::weak_ptr<SoundCache> wptrCacheData, std::size_t pathOffset,
             std::size_t nameOffset, std::size_t sndDataOffset, std::size_t sndDataSize)
    : Sound()
{

    ptrData_->wptrData   = std::move(wptrCacheData);
    ptrData_->pathOffset = pathOffset;
    ptrData_->nameOffset = nameOffset;
    ptrData_->dataOffset = sndDataOffset;
    ptrData_->dataSize   = sndDataSize;
}

Sound::Sound(SoundHandle handle, uint32_t idx, uint32_t sampleRate, uint32_t sampleBitSize, uint32_t numChannels,
             std::weak_ptr<SoundCache> wptrCacheData, std::size_t pathOffset, std::size_t nameOffset,
             std::size_t sndDataOffset, std::size_t sndDataSize, bool isCompressed) : Sound()
{
    ptrData_->handle        = handle;
    ptrData_->idx           = idx;
    ptrData_->sampleRate    = sampleRate;
    ptrData_->sampleBitSize = sampleBitSize;
    ptrData_->numChannels   = numChannels;
    ptrData_->isCompressed  = isCompressed;
    ptrData_->wptrData      = std::move(wptrCacheData);
    ptrData_->pathOffset    = pathOffset;
    ptrData_->nameOffset    = nameOffset;
    ptrData_->dataOffset    = sndDataOffset;
    ptrData_->dataSize      = sndDataSize;
}

Sound::Sound(Sound&& other) noexcept
{
    ptrData_ = std::move(other.ptrData_);
}

Sound& Sound::operator=(Sound&& rhs) noexcept
{
    if (&rhs != this) {
        ptrData_ = std::move(rhs.ptrData_);
    }

    return *this;
}

Sound::Sound(const Sound& rhs)
{
    ptrData_ = std::make_unique<Sound::SoundData>(*rhs.ptrData_);
}

Sound& Sound::operator=(const Sound& rhs)
{
    if (&rhs != this) {
        ptrData_ = std::make_unique<Sound::SoundData>(*rhs.ptrData_);
    }

    return *this;
}

SoundHandle Sound::handle() const
{
    return ptrData_->handle;
}

uint32_t Sound::idx() const
{
    return ptrData_->idx;
}

std::string_view Sound::name() const
{
    return ptrData_->name();
}

std::size_t Sound::sampleRate() const
{
    return ptrData_->sampleRate;
}

std::size_t Sound::sampleBitSize() const
{
    return ptrData_->sampleBitSize;
}

std::size_t Sound::channels() const
{
    return ptrData_->numChannels;
}

std::size_t Sound::dataSize() const
{
    return ptrData_->dataSize;
}

bool Sound::isCompressed() const
{
    return ptrData_->isCompressed;
}

bool Sound::isValid() const
{
    return ptrData_->isValid();
}

std::shared_ptr<SoundCache> Sound::lockOrThrow() const
{
    return ptrData_->lockOrThrow();
}

bool Sound::isValid(const SoundCache& data) const
{
    return ptrData_->isValid(data);
}

ByteArray Sound::data() const
{
    return ptrData_->data();
}

void audio::wavWrite(OutputStream& ostream, const Sound& sound)
{
    sound.ptrData_->wavWrite(ostream);
}

void audio::wavWrite(OutputStream&& ostream, const Sound& sound)
{
    sound.ptrData_->wavWrite(std::move(ostream));
}

void audio::wvWrite(OutputStream& ostream, const Sound& sound)
{
    sound.ptrData_->wvWrite(ostream);
}

void audio::wvWrite(OutputStream&& ostream, const Sound& sound)
{
    sound.ptrData_->wvWrite(std::move(ostream));
}
