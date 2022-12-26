#include "../soundbank.h"
#include "../soundbank_error.h"
#include "sbtrack.h"
#include "sound_data.h"
#include "serialization/soundbank_serializer.h"

#include <libim/common.h>
#include <libim/content/asset/world/impl/serialization/cnd/cnd.h>
#include <libim/utils/utils.h>

using namespace libim;
using namespace libim::content;
using namespace libim::content::asset;
using namespace libim::content::audio;
using namespace std::string_view_literals;

struct SoundBank::SoundBankImpl
{
    SoundHandle nextHandle = SoundHandle(0);
    std::vector<SoundBankTrack> tracks;

    // Returns next free sound handle
    SoundHandle getNextHandle()
    {
        auto handle = utils::to_underlying(nextHandle);
        if ((handle & 1) != 0) {
            handle = (handle + 1) % 1111111;
        }
        auto hSnd = handle + 1234;
        nextHandle = static_cast<SoundHandle>((handle + 1) % 1111111);
        return static_cast<SoundHandle>(hSnd);
    }
};

SoundBank::SoundBank(std::size_t nTracks)
{
    ptrImpl_ = std::make_unique<SoundBankImpl>();
    ptrImpl_->tracks.resize(nTracks);
}

SoundBank::~SoundBank()
{}

std::size_t SoundBank::count() const
{
    return ptrImpl_->tracks.size();
}

void SoundBank::setHandleSeed(SoundHandle seed)
{
    ptrImpl_->nextHandle = seed;
}

const IndexMap<Sound>& SoundBank::getTrack(std::size_t trackIdx) const
{
    if (trackIdx >= ptrImpl_->tracks.size()) {
        throw SoundBankError("trackIdx out of range!");
    }
    return ptrImpl_->tracks.at(trackIdx).sounds;
}

const Sound& SoundBank::loadSound(InputStream& istream, std::size_t trackIdx)
{
    auto& snd = ptrImpl_->tracks.at(trackIdx).loadSound(istream);
    snd.ptrData_->handle = ptrImpl_->getNextHandle();
    return snd;
}

bool SoundBank::importTrack(std::size_t trackIdx, const InputStream& istream)
{
    LOG_DEBUG("SoundBank: Importing sound track % from stream: %", trackIdx, istream.name());
    if (trackIdx >= ptrImpl_->tracks.size()) {
        throw SoundBankError("Sound track index out of range!");
    }

    auto& track = ptrImpl_->tracks.at(trackIdx);
    readSoundBankTrack(istream, track);

    static_assert(sizeof(SoundHandle) == 4);
    SoundHandle handle = istream.read<SoundHandle>();

    LOG_DEBUG("SoundBank: Imported % sound(s) to track: %, nextHandle: % ", track.sounds.size(), trackIdx, utils::to_underlying(handle));
    if (handle == SoundHandle(0) && !track.sounds.isEmpty()) {
        return false;
    }

    ptrImpl_->nextHandle = handle;
    return true;
}

bool SoundBank::importTrack(std::size_t trackIdx, InputStream&& istream)
{
    return importTrack(trackIdx, istream);
}

bool SoundBank::exportTrack(std::size_t trackIdx, OutputStream& ostream) const
{
    LOG_DEBUG("SoundBank: Exporting track % to stream: %", trackIdx, ostream.name());
    if (trackIdx >= ptrImpl_->tracks.size()) {
        throw SoundBankError("Sound track index out of range!");
    }

    // Write the track data to the stream
    const auto& track = ptrImpl_->tracks.at(trackIdx);
    if (track.sounds.isEmpty()) {
        return false;
    }

    writeSoundBankTrack(ostream, track, trackIdx);

    // Write the current handle seed to the end of the stream
    static_assert(sizeof(SoundHandle) == 4);
    ostream.write(ptrImpl_->nextHandle);

    LOG_DEBUG("SoundBank: Exported % sound(s) from track: %", track.sounds.size(), trackIdx);
    return true;
}

bool SoundBank::exportTrack(std::size_t trackIdx, OutputStream&& ostream) const
{
    return exportTrack(trackIdx, ostream);
}
void SoundBank::setStaticTrack(std::size_t trackIdx, bool isStatic)
{
    if (trackIdx >= ptrImpl_->tracks.size()) {
        throw SoundBankError("Sound track index out of range!");
    }
    ptrImpl_->tracks.at(trackIdx).isStatic = isStatic;
}
