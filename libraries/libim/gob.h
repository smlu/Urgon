#ifndef LIBIM_GOB_H
#define LIBIM_GOB_H
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "common.h"
#include "io/stream.h"
#include "io/filestream.h"

namespace libim {
    static constexpr std::array<char,4> GOB_FILE_SIGNATURE      = {{'G','O','B',' '}};
    static constexpr uint32_t           GOB_FILE_VERSION        = 0x14;
    static constexpr uint32_t           GOB_ENTRY_NAME_MAX_SIZE = 128;


    struct GobFileHeader {
        std::array<char,4> signature;
        uint32_t version = 0;
        uint32_t directoryOffset = 0;
    };

    struct GobFileEntry {
        uint32_t offset = 0;
        uint32_t size   = 0;
        char name[GOB_ENTRY_NAME_MAX_SIZE];
    };

    struct GobFileDirectory
    {
        StreamPtr<Stream> stream;
        std::vector<GobFileEntry> entries;
    };

    template<>
    GobFileHeader Stream::read() const
    {
        GobFileHeader header;
        const auto nRead = read(reinterpret_cast<byte_t*>(&header), sizeof(header));
        if(nRead != sizeof(header)) {
            throw StreamError("Faild to read 'GobFileHeader'");
        }

        return header;
    }

    template<>
    GobFileEntry Stream::read() const
    {
        GobFileEntry entry;
        const auto nRead = read(reinterpret_cast<byte_t*>(&entry), sizeof(entry));
        if(nRead != sizeof(entry)) {
            throw StreamError("Faild to read 'GobFileEntry'");
        }

        return entry;
    }

    std::shared_ptr<GobFileDirectory> LoadGobFromFile(const std::string& filepath)
    {
        try
        {
            auto ifs = MakeStreamPtr<InputFileStream>(filepath);

            /* Read Header */
            auto header = ifs->read<GobFileHeader>();

            /* Verify file signature */
            if(header.signature != GOB_FILE_SIGNATURE)
            {
                std::cerr << "Error unknown GOB file!\n";
                return nullptr;
            }

            /* Verify file version */
            if(header.version != GOB_FILE_VERSION)
            {
                std::cerr << "Error wrong GOB file version: " << header.version << std::endl;
                return nullptr;
            }

            /* Seek to directory */
            ifs->seek(header.directoryOffset);

            /* Read Directory size */
            const auto nDirSize = ifs->read<uint32_t>();

            /* Read Directory */
            auto directory = std::make_shared<GobFileDirectory>();
            directory->entries = ifs->read<std::vector<GobFileEntry>>(nDirSize);

            // TODO: measure if below method is faster
    //       // directory->entries.resize(nDirSize);
    //        const auto nToRead = nDirSize * sizeof(GobFileEntry);
    //        if(!ifs->read(reinterpret_cast<byte_t*>(directory->entries.data()), nDirSize * sizeof(GobFileEntry)))
    //        {
    //            std::cerr << "GOB file error could not read directory: " << IosErrorStr(ifs) << "!\n";
    //            return nullptr;
    //        }

            directory->stream = std::move(ifs);
            return directory;

        }
        catch (const std::exception& e)
        {
            std::cerr << "GOB Error: " << e.what();
            return nullptr;
        }
    }
}
#endif // LIBIM_GOB_H
