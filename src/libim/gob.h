#ifndef GOB_H
#define GOB_H
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "common.h"

constexpr std::array<char,4> GOB_FILE_SIGNATURE      = {'G','O','B',' '};
constexpr uint32_t           GOB_FILE_VERSION        = 0x14;
constexpr uint32_t           GOB_ENTRY_NAME_MAX_SIZE = 128;


struct GobFileHeader {
    char signature[4]{};
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
    std::ifstream stream;
    std::vector<GobFileEntry> entries;
};

std::shared_ptr<GobFileDirectory> LoadGobFromFile(const std::string& filepath)
{
    std::ifstream ifs(filepath, std::ios::in | std::ios::binary);
    if (!ifs.is_open())
    {
        std::cerr << "Error opening GOB file for reading!\n";
        return nullptr;
    }

    /* Read Header */
    GobFileHeader header;
    if(!ifs.read(reinterpret_cast<char*>(&header), sizeof(header)))
    {
        std::cerr << "Error reading GOB file header: " << IosErrorStr(ifs) << "!\n";
        return nullptr;
    }

    /* Verify file signature */
    if(strncmp(header.signature, GOB_FILE_SIGNATURE.data(), GOB_FILE_SIGNATURE.size()) != 0)
    {
        std::cerr << "Error unknown GOB file!\n";
        return nullptr;
    }

    /* Verify file version */
    if(header.version!= GOB_FILE_VERSION)
    {
        std::cerr << "Error wrong GOB file version: " << header.version << std::endl;
        return nullptr;
    }

    /* Seek to directory */
    if(!ifs.seekg(header.directoryOffset))
    {
        std::cerr << "GOB file error could not seek to directory position: " << IosErrorStr(ifs) << "!\n";
        return nullptr;
    }

    /* Read Directory size*/
    uint32_t nDirSize = 0;
    if(!ifs.read(reinterpret_cast<char*>(&nDirSize), sizeof(nDirSize)))
    {
        std::cerr << "GOB file error could not read directory size: " << IosErrorStr(ifs) << "!\n";
        return nullptr;
    }

    /* Read Directory */
    auto directory = std::make_shared<GobFileDirectory>();
    directory->entries.resize(nDirSize);
    if(!ifs.read(reinterpret_cast<char*>(directory->entries.data()), nDirSize * sizeof(GobFileEntry)))
    {
        std::cerr << "GOB file error could not read directory: " << IosErrorStr(ifs) << "!\n";
        return nullptr;
    }

    directory->stream = std::move(ifs);
    return directory;
}

#endif // GOB_H
