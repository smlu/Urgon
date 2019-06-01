#include "../sound.h"
#include "wav.h"
#include "indywv.h"
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
            dirNameOffset_ < data.size() &&
            dirNameOffset_ <= nameOffset_ &&
            nameOffset_ < data.size() &&
            sampleRate_ > 0 &&
            bitsPerSample_ > 0 &&
            numChannels_ > 0;
}

//Sound& Sound::deserialize(const InputStream& istream)
//{
//    return *this;
//}

//Sound& Sound::deserialize(const InputStream&& istream)
//{

//    return *this;
//}

bool Sound::serialize(OutputStream&& ostream) const
{
    return serialize(ostream);
}

bool Sound::serialize(OutputStream& ostream) const
{
    WavHeader wavHeader;
    wavHeader.fmt.size          = 16;
    wavHeader.fmt.audioFormat   = AudioFormat::LPCM;
    wavHeader.fmt.numChannels   = numChannels_; // TODO: safe cast
    wavHeader.fmt.sampleRate    = sampleRate_; // TODO: safe cast
    wavHeader.fmt.blockAlign    = numChannels_ * bitsPerSample_/ 8; // TODO: safe cast
    wavHeader.fmt.byteRate      = sampleRate_ * wavHeader.fmt.blockAlign; // TODO: safe cast
    wavHeader.fmt.bitsPerSample = bitsPerSample_; // TODO: safe cast

    WavDataChunk dataChunk;
    dataChunk.data = data();
    if(dataChunk.data.empty() && dataSize_ > 0) {
        std::logic_error("Cannot serialize invalid sound object!");
    }

    dataChunk.size = dataChunk.data.size(); // TODO: safe cast
    wavHeader.size = 36 + dataChunk.size;

    ostream << wavHeader
            << dataChunk;

    return true;
}

ByteArray Sound::data() const
{
    ByteArray bytes;
    auto ptrData = lock_or_throw();
    if(!is_valid(*ptrData)) {
        return bytes;
    }

    auto itBegin = ptrData->begin() + dataOffset_; //TODO: safe cast
    auto itEnd = itBegin + dataSize_; //TODO: safe cast
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

