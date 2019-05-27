#include "../soundbank.h"
#include "../soundbank_error.h"
#include "soundbank_instance.h"
#include "../../asset/world/impl/serialization/cnd/cnd.h"
#include "../../../common.h"
#include "../../../utils/utils.h"

using namespace libim;
using namespace libim::content;
using namespace libim::content::asset;
using namespace libim::content::audio;
using namespace libim::content::audio::impl;
using namespace std::string_view_literals;

struct SoundBank::SoundBankImpl
{
    uint32_t nonceFileId = 0;
    std::vector<SoundBankInstance> vecInstances;

    uint32_t getNextFileId()
    {
        uint32_t v0 = nonceFileId;
        if ( nonceFileId & 1 ){
            v0 = (nonceFileId + 1) % 1111111;
        }
        uint32_t fileId = v0 + 1234;
        nonceFileId = (v0 + 1) % 1111111;
        return fileId;
    }
};


SoundBank::SoundBank(std::size_t nInstances)
{
    ptrImpl_ = std::make_unique<SoundBankImpl>();
    ptrImpl_->vecInstances.reserve(nInstances);
    ptrImpl_->vecInstances.resize(nInstances);
}

SoundBank::~SoundBank()
{}

std::size_t SoundBank::count() const
{
    return ptrImpl_->vecInstances.size();
}

const std::unordered_map<std::string, Sound>& SoundBank::getSounds(std::size_t instanceIdx) const
{
    if(instanceIdx >= ptrImpl_->vecInstances.size()) {
        throw SoundBankError("instanceIdx out of range!");
    }
    return ptrImpl_->vecInstances.at(instanceIdx).sounds;
}

bool SoundBank::importBank(std::size_t instanceIdx, const InputStream& istream)
{
    if(instanceIdx >= ptrImpl_->vecInstances.size()) {
        throw SoundBankError("instanceIdx out of range!");
    }

    auto& inst = ptrImpl_->vecInstances.at(instanceIdx);
    if(FileExtMatch(istream.name(), ".cnd"sv))
    {
        auto nonce = CND::ParseSectionSounds(inst, istream);
        if(nonce == 0 && !inst.sounds.empty()) {
            return false;
        }

        ptrImpl_->nonceFileId = nonce;
        return true;
    }
    else {
        throw SoundBankError("Cannot import soundbank, unknown stream!");
    }
}

