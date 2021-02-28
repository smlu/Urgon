#include "indywv.h"
#include "sound_ser_helper.h"
#include "../sound.h"

#include <libim/io/stream.h>
#include <exception>

using namespace libim;
using namespace libim::content;
using namespace libim::content::audio;

Sound::Sound(std::weak_ptr<ByteArray> wptrBankData, std::size_t dirNameOffset, std::size_t nameOffset, std::size_t dataOffset, std::size_t dataSize) :
    dirNameOffset_(dirNameOffset),
    nameOffset_(nameOffset),
    dataOffset_(dataOffset),
    dataSize_(dataSize),
    wptrData_(wptrBankData)
{}

std::string_view Sound::name() const
{
    auto ptrData = lockOrThrow();
    const char* pName = reinterpret_cast<char*>(&ptrData->at(nameOffset_));
    return std::string_view(pName);
}

bool Sound::isValid() const
{
    auto ptrData = wptrData_.lock();
    return ptrData && isValid(*ptrData);

}

std::shared_ptr<ByteArray> Sound::lockOrThrow() const
{
    auto ptrData = wptrData_.lock();
    if(!ptrData) {
        throw std::logic_error("Dead sound object");
    }
    return ptrData;
}

bool Sound::isValid(const ByteArray& data) const
{
    return dataOffset_ + dataSize_  < data.size() &&
        dirNameOffset_ < data.size()              &&
        dirNameOffset_ <= nameOffset_             &&
        nameOffset_    < data.size()              &&
        sampleRate_    > 0                        &&
        bitsPerSample_ > 0                        &&
        numChannels_   > 0;
}

ByteArray Sound::data() const
{
    // Returns decompressed sound data.

    ByteArray bytes;
    auto ptrData = lockOrThrow();
    if(!isValid(*ptrData)) {
        return bytes;
    }

    auto itBegin = ptrData->begin() + dataOffset_; //TODO: safe cast to difference_type
    auto itEnd = itBegin + dataSize_; //TODO: safe cast to difference_type
    if(isIndyWVFormat_) {
        bytes = IndyVW::inflate(InputBinaryStream(*ptrData, itBegin, itEnd));
    }
    else
    {
        bytes.reserve(dataSize_);
        std::copy(itBegin, itEnd, std::back_inserter(bytes));
    }

    return bytes;
}

void libim::content::audio::wavWrite(OutputStream& ostream, const Sound& sound)
{
    auto data = sound.data();
    if(data.empty() && sound.dataSize_ > 0) {
        throw StreamError("Cannot write invalid sound to stream as WAV");
    }
    soundSerializeAsWAV(ostream, sound.channels(), sound.sampleRate(), sound.bitsPerSample(), std::move(data));
}

void libim::content::audio::wavWrite(OutputStream&& ostream, const Sound& sound)
{
    wavWrite(ostream, sound);
}

void libim::content::audio::iwvWrite(OutputStream& ostream, const Sound& sound)
{
     auto ptrData = sound.lockOrThrow();
    if(!sound.isValid(*ptrData) || !sound.isIndyWVFormat()) { // TODO: implement converting of data to indyWV format
        throw StreamError("Cannot write invalid sound to stream as IndyWV");
    }
    soundSerializeAsIndyWV(ostream, sound.channels(), sound.sampleRate(), sound.bitsPerSample(), ptrData, sound.dataOffset_, sound.dataSize_);
}

void libim::content::audio::iwvWrite(OutputStream&& ostream, const Sound& sound)
{
    iwvWrite(ostream, sound);
}
