#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <libim/io/stream.h>
#include <libim/io/filestream.h>
#include <libim/io/vfstream.h>
#include <libim/types/fixed_string.h>
#include <libim/types/sharedref.h>

using namespace libim;

static constexpr std::array<char,4> kGobFileMagic       = {{'G','O','B',' '}};
static constexpr uint32_t           kGobFileVersion     = 0x14;
static constexpr uint32_t           kGobFilePathMaxSize = 128;


struct GobFileHeader
{
    std::array<char,4> magic;
    uint32_t version         = 0;
    uint32_t directoryOffset = 0;
};

struct GobFileEntry
{
    uint32_t offset = 0;
    uint32_t size   = 0;
    FixedString<kGobFilePathMaxSize> filePath;
};


template<>
GobFileHeader Stream::read() const
{
    GobFileHeader header;
    const auto nRead = read(reinterpret_cast<byte_t*>(&header), sizeof(header));
    if(nRead != sizeof(header)) {
        throw StreamError("Failed to read 'GobFileHeader'");
    }

    return header;
}

template<>
GobFileEntry Stream::read() const
{
    GobFileEntry entry;
    const auto nRead = read(reinterpret_cast<byte_t*>(&entry), sizeof(entry));
    if(nRead != sizeof(entry)) {
        throw StreamError("Failed to read 'GobFileEntry'");
    }

    return entry;
}

VfContainer libim::gobLoad(SharedRef<InputStream> is)
{
    /* Read Header */
    auto header = is->read<GobFileHeader>();

    /* Verify file signature */
    if(header.magic != kGobFileMagic) {
        throw StreamError("Unknown or invalid GOB file");
    }

    /* Verify file version */
    if(header.version != kGobFileVersion) {
        throw StreamError(std::string("Wrong GOB file version: ") + utils::to_string(header.version));
    }

    /* Seek to directory */
    is->seek(header.directoryOffset);

    /* Read directory size and entries */
    const auto numEntries = is->read<uint32_t>();
    const auto entries    = is->read<std::vector<GobFileEntry>>(numEntries);

    VfContainer c;
    for (const auto& e : entries) {
        c.add(e.filePath, makeSharedRef<VirtualFile>(is, e.offset, e.size));
    }
    return c;
}

VfContainer libim::gobLoad(const std::filesystem::path& gobFilePath)
{
    return gobLoad(makeSharedRef<InputFileStream>(gobFilePath));
}