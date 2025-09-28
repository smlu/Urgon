#ifndef LIBIM_SOUNDBANK_H
#define LIBIM_SOUNDBANK_H
#include "sound.h"
#include <libim/common.h>
#include <libim/io/stream.h>
#include <libim/types/indexmap.h>

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace libim::content::audio
{
    class SoundBank final
    {
    public:
        SoundBank(std::size_t nTracks);
        SoundBank(const SoundBank&) = delete;
        SoundBank& operator=(const SoundBank&) = delete;
        ~SoundBank();

        /**
         * Returns the number of tracks in soundbank.
         * @return Number of tracks.
         */
        std::size_t count() const;

        void setHandleSeed(SoundHandle seed);

        /**
         * Returns UniqueTable of sounds in track.
         *
         * @param trackIdx - Track index.
         * @return Reference to UniqueTable of track sounds.
         *
         * @throw SoundBankError - If trackIdx is out of range.
         */
        const UniqueTable<Sound>& getTrack(std::size_t trackIdx) const;

        /**
         * Loads sound from stream to track.
         * Supported formats: WAV, WV (IndyWV)
         *
         * @param istream  - Input stream to read data from.
         * @param trackIdx - Track index.
         * @return Reference to Sound object.
         *
         * @throw SoundBankError - If trackIdx is out of range or data in stream is corrupted.
         *                       - If trying to load unsupported sound format.
         *                       - If unable to read data from stream.
         * @throw StreamError    - If IO error occurs while reading from stream.
         */
        const Sound& loadSound(InputStream& istream, std::size_t trackIdx);

        /**
         * Imports soundbank data to track.
         * @param trackIdx - Track index.
         * @param istream  - Input stream to read data from.
         * @return True if import was successful, false otherwise.
         *
         * @throw SoundBankError - If trackIdx is out of range or data in stream is corrupted.
         * @throw StreamError    - If IO error occurs while reading from stream.
         */
        bool importTrack(std::size_t trackIdx, const InputStream& istream);

        /**
         * Imports soundbank data to track.
         * @param trackIdx - Track index.
         * @param istream  - R-value reference to the input stream to read data from.
         * @return True if import was successful, false otherwise.
         *
         * @throw SoundBankError - If trackIdx is out of range or data in stream is corrupted.
         * @throw StreamError    - If IO error occurs while reading from stream.
         */
        bool importTrack(std::size_t trackIdx, InputStream&& istream);

        /**
         * Exports soundbank data from track.
         * @param trackIdx - Track index.
         * @param ostream  - Output stream to write data to.
         * @return True if export was successful, false otherwise.
         *
         * @throw SoundBankError - If trackIdx is out of range.
         * @throw StreamError    - If IO error occurs while writing to stream.
         */
        bool exportTrack(std::size_t trackIdx, OutputStream& ostream) const;

        /**
         * Exports soundbank data from track.
         * @param trackIdx - Track index.
         * @param ostream  - R-value reference to the output stream to write data to.
         * @return True if export was successful, false otherwise.
         *
         * @throw SoundBankError - If trackIdx is out of range.
         * @throw StreamError    - If IO error occurs while writing to stream.
         */
        bool exportTrack(std::size_t trackIdx, OutputStream&& ostream) const;

        void setStaticTrack(std::size_t trackIdx, bool isStatic);

    private:
        struct SoundBankImpl;
        std::unique_ptr<SoundBankImpl> ptrImpl_;
    };
}


#endif // LIBIM_SOUNDBANK_H
