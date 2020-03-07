#include "indywv.h"
#include "sound_ser_helper.h"
#include "../sound.h"
#include "../../../io/stream.h"
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
    auto ptrData = lock_or_throw();
    const char* pName = reinterpret_cast<char*>(&ptrData->at(nameOffset_));
    return std::string_view(pName);
}

bool Sound::isValid() const
{
    auto ptrData = wptrData_.lock();
    return ptrData && is_valid(*ptrData);

}

std::shared_ptr<ByteArray> Sound::lock_or_throw() const
{
    auto ptrData = wptrData_.lock();
    if(!ptrData) {
        std::logic_error("Dead sound object");
    }
    return ptrData;
}

bool Sound::is_valid(const ByteArray& data) const
{
    return dataOffset_ + dataSize_  < data.size() &&
        dirNameOffset_ < data.size()              &&
        dirNameOffset_ <= nameOffset_             &&
        nameOffset_    < data.size()              &&
        sampleRate_    > 0                        &&
        bitsPerSample_ > 0                        &&
        numChannels_   > 0;
}

//Sound& Sound::deserialize(const InputStream& istream)
//{
//    return *this;
//}

//Sound& Sound::deserialize(const InputStream&& istream)
//{

//    return *this;
//}

void Sound::serialize(OutputStream&& ostream, SerializeFormat format) const
{
    serialize(ostream, format);
}

void Sound::serialize(OutputStream& ostream, SerializeFormat format) const
{
    switch (format) {
        case SerializeFormat::WAV:
        {
            auto data = this->data();
            if(data.empty() && dataSize_ > 0) {
                std::logic_error("Cannot serialize invalid sound object as WAV");
            }
            SoundSerializeAsWAV(ostream, numChannels_, sampleRate_, bitsPerSample_, std::move(data));
        } break;
        case SerializeFormat::IndyWV:
        {
            auto ptrData = lock_or_throw();
            if(!is_valid(*ptrData) || !isIndyWVFormat_) { // TODO: implement converting of data to indyWV format
                std::logic_error("Cannot serialize invalid sound object as IndyWV");
            }
            SoundSerializeAsIndyWV(ostream, numChannels_, sampleRate_, bitsPerSample_, ptrData, dataOffset_, dataSize_);
        } break;
        default:
            std::logic_error("Cannot serialize sound, unknown serialization format");
    }
}

ByteArray Sound::data() const
{
    // Returns decompressed sound data.

    ByteArray bytes;
    auto ptrData = lock_or_throw();
    if(!is_valid(*ptrData)) {
        return bytes;
    }

    auto itBegin = ptrData->begin() + dataOffset_; //TODO: safe cast to difference_type
    auto itEnd = itBegin + dataSize_; //TODO: safe cast to difference_type
    if(isIndyWVFormat_) {
        bytes = IndyVW::Inflate(InputBinaryStream(*ptrData, itBegin, itEnd));
    }
    else
    {
        bytes.reserve(dataSize_);
        std::copy(itBegin, itEnd, std::back_inserter(bytes));
    }

    return bytes;
}
