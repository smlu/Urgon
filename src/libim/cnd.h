#ifndef CND_H
#define CND_H
#include <array>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "material/bmp.h"
#include "material/colorformat.h"
#include "material/material.h"
#include "material/texture.h"
#include "common.h"

constexpr uint32_t CND_VERSION = 3;
constexpr std::array<char, 1217> CND_COPYRIGHT = {
    "................................" \
    "................@...@...@...@..." \
    ".............@...@..@..@...@...." \
    "................@.@.@.@.@.@....." \
    "@@@@@@@@......@...........@....." \
    "@@@@@@@@....@@......@@@....@...." \
    "@@.....@.....@......@@@.....@@.." \
    "@@.@@@@@......@.....@@@......@@." \
    "@@@@@@@@.......@....@@.....@@..." \
    "@@@@@@@@.........@@@@@@@@@@....." \
    "@@@@@@@@..........@@@@@@........" \
    "@@.....@..........@@@@@........." \
    "@@.@@@@@.........@@@@@@........." \
    "@@.....@.........@@@@@@........." \
    "@@@@@@@@.........@@@@@@........." \
    "@@@@@@@@.........@@@@@@@........" \
    "@@@...@@.........@@@@@@@........" \
    "@@.@@@.@.........@.....@........" \
    "@@..@..@........@.......@......." \
    "@@@@@@@@........@.......@......." \
    "@@@@@@@@.......@........@......." \
    "@@..@@@@.......@........@......." \
    "@@@@..@@......@.........@......." \
    "@@@@.@.@......@.........@......." \
    "@@....@@........................" \
    "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" \
    "@@@@@@@@@@@@@.@@@@@@@@@@@@@@@@@@" \
    "@@.@@..@@@@@..@@@@@@@@@@.@@@@@@@" \
    "@@.@.@.@@@@.@.@@@.@..@@...@@@..@" \
    "@@..@@@@@@....@@@..@@@@@.@@@@.@@" \
    "@@@@@@@@...@@.@@@.@@@@@..@@...@@" \
    "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" \
    "@.(c).lucasarts.entertainment..@" \
    "@.........company.llc..........@" \
    "@....(c).lucasfilm.ltd.&.tm....@" \
    "@.....all.rights.reserved......@" \
    "@...used.under.authorization...@" \
    "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
};



PACKED(
struct CndHeader
{
    uint32_t fileSize;
    char     copyright[1216];
    char     filePath[64];
    uint32_t type;               // 0xD = container type (jones3dstatic.cnd), 0xC = game world
    uint32_t version;
    float    worldGravity;
    float    ceilingSky_Z;
    float    horizonDistance;
    float    horizonSkyOffset[2]; // x,y
    float    ceilingSkyOffset[2]; // x,y
    float    LOD_Distances[4];
    struct {
        int32_t enabled;
        float color[4]; //rgba
        float startDepth;
        float endDepth;
    } fog;
    uint32_t unknown2;
    uint32_t numMaterials;
    uint32_t sizeMaterials;
    uint32_t aMaterials;
    uint32_t unknown4[13];
    uint32_t aSelectors;
    uint32_t unknown5;
    uint32_t worldAIClasses;
    uint32_t unknown6[2];
    uint32_t worldModels;
    uint32_t unknown7[2];
    uint32_t worldSprites;
    uint32_t unknown8[2];
    uint32_t worldKeyframes;
    uint32_t unknown9[22];
    uint32_t worldSounds;
    uint32_t worldSoundUnknown; // Size of sound data
});

struct CndMatHeader
{
    char name[64];
    int width;
    int height;
    int mipmapCount;
    int texturesPerMipmap;
    struct ColorFormat colorInfo;
};




static std::vector<Material> LoadMaterialsFromCndFile(const std::string& filename)
{
    std::vector<Material> materials;

    std::ifstream ifs(filename, std::ios::in | std::ios::binary);
    if (!ifs.is_open())
    {
        std::cerr << "Error opening CND file for reading!\n";
        return materials;
    }

    /* Read cnd file header */
    CndHeader cndHeader{};
    if(!ifs.read(reinterpret_cast<char*>(&cndHeader), sizeof(cndHeader)))
    {
        std::cerr << "Error reading CND file header: " << IosErrorStr(ifs) << "!\n";
        return materials;
    }

    /* Verify file copyright notice  */
    if(strncmp(cndHeader.copyright, &CND_COPYRIGHT[0], CND_COPYRIGHT.size() - 1))
    {
        std::cerr << "Error bad CND file copyright!\n";
        return materials;
    }

    /* Verify file version */
    if(cndHeader.version != CND_VERSION)
    {
        std::cerr << "Error wrong CND file version: " << cndHeader.version << std::endl;
        return materials;
    }

    /* Return if no materials are present in file*/
    if(cndHeader.numMaterials < 1)
    {
        std::cout << "No materials found in CND file!\n";
        return materials;
    }

    /* Seek to materials position */
    if(!ifs.seekg(cndHeader.worldSoundUnknown + 48 * cndHeader.worldSounds, std::ios::cur) // 48 is sound file descriptor len (header)
            || ifs.peek() == EOF)
    {
        std::cerr << "Cnd file error could not seek to position of materials: " << IosErrorStr(ifs) << "!\n";
        return materials;
    }

    uint32_t unknown; // unknown 4 bytes
    ifs.read(reinterpret_cast<char*>(&unknown), sizeof(unknown));

    /* Read materials pixel data size */
    uint32_t nBitmapBuffSize;
    if(!ifs.read(reinterpret_cast<char*>(&nBitmapBuffSize), sizeof(nBitmapBuffSize)))
    {
        std::cerr<< "Cnd file error could not read materials pixel data size: " << IosErrorStr(ifs) << "!\n";
        return materials;
    }
    else if(nBitmapBuffSize == 0)
    {
        std::cerr << "Warning read materials bitmap data size == 0!\n";
        return materials;
    }

    /* Read material header list from file stream */
    std::vector<CndMatHeader> matHeaders(cndHeader.numMaterials);
    if(!ifs.read(reinterpret_cast<char*>(&matHeaders[0]), sizeof(CndMatHeader) * matHeaders.size()))
    {
        std::cerr<< "Cnd file error could not read material headers: " << IosErrorStr(ifs) << "!\n";
        return materials;
    }

    /* Read materials pixel data from file stream */
    Bitmap vecBitmapBuff(nBitmapBuffSize);
    if(!ifs.read(reinterpret_cast<char*>(&vecBitmapBuff[0]), vecBitmapBuff.size()))
    {
        std::cerr<< "Cnd file error could not read materials bitmap data into buffer: " << IosErrorStr(ifs) << "!\n";
        return materials;
    }

    /* Close stream */
    ifs.close();

    /* Extract materials from pixel data buffer */
    for(auto&& matHeader : matHeaders)
    {
        if(matHeader.mipmapCount < 1 || matHeader.texturesPerMipmap < 1)
        {
            std::cerr << "Warning: No pixel data found for material: " << matHeader.name << std::endl;
            continue;
        }

        if(matHeader.colorInfo.bpp % 8 != 0)
        {
            std::cerr << "Error: Cannot extract material " << matHeader.name << " from buffer. Wrong bitdepth size: " <<  matHeader.colorInfo.bpp << std::endl;
            materials.clear();
            return materials;
        }

        /* Read mipmaps from buffer */
        std::vector<Mipmap> mipmaps(matHeader.mipmapCount);
        for(auto&& mipmap : mipmaps) {
            mipmap = MoveMipmapFromBuffer(vecBitmapBuff, matHeader.texturesPerMipmap, matHeader.width, matHeader.height, matHeader.colorInfo);
        }

        Material mat(matHeader.name);
        mat.setSize(matHeader.width, matHeader.height);
        mat.setColorFormat(matHeader.colorInfo);
        mat.setMipmaps(std::move(mipmaps));

        materials.emplace_back(std::move(mat));
    }

    if(!vecBitmapBuff.empty()) {
        std::cerr << "Warning: Not all bitmap data was copied from buffer!\n";
    }

    return materials;
}

static bool ReplaceMaterialInCndFile(const Material& mat, const std::string& filename)
{
    if(mat.mipmaps().empty() || mat.mipmaps().at(0).empty()) return false;

    std::ifstream ifs(filename, std::ios::in | std::ios::binary);
    if (!ifs.is_open())
    {
        std::cerr << "Error opening CND file for reading!\n";
        return false;
    }

    /* Read header */
    CndHeader cndHeader{};
    if(!ifs.read(reinterpret_cast<char*>(&cndHeader), sizeof(cndHeader)))
    {
        std::cerr << "Error reading cnd file header: " << IosErrorStr(ifs) << "!\n";
        return false;
    }

    /* Verify file copyright notice  */
    if(strncmp(cndHeader.copyright, &CND_COPYRIGHT[0], CND_COPYRIGHT.size() - 1))
    {
        std::cerr << "Error bad CND file copyright!\n";
        return false;
    }

    /* Verify file version */
    if(cndHeader.version != CND_VERSION)
    {
        std::cerr << "Error wrong CND file version: " << cndHeader.version << std::endl;
        return false;
    }

    /* If no materials are present in file, return */
    if(cndHeader.numMaterials < 1)
    {
        std::cout << "No materials found in CND file!\n";
        return false;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Find material that is being patched
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* Seek to material header list */
    std::fstream::pos_type offMaterialList = sizeof(CndHeader) + cndHeader.worldSoundUnknown + 48 * cndHeader.worldSounds; // 48 - sizeof sound file header
    if(!ifs.seekg(offMaterialList, std::ios::beg) || ifs.peek() == EOF)
    {
        std::cerr << "Cnd file error, could not seek to the material headers position: " << IosErrorStr(ifs);
        return false;
    }

    uint32_t unknown; // unknown 4 bytes
    ifs.read(reinterpret_cast<char*>(&unknown), sizeof(unknown));

    /* Read size of materials bitmap data */
    uint32_t nBitmapBufSize;
    if(!ifs.read(reinterpret_cast<char*>(&nBitmapBufSize), sizeof(nBitmapBufSize)))
    {
        std::cerr<< "Cnd file error could not read materials pixel data size: " << IosErrorStr(ifs) << "!\n";
        return false;
    }
    else if(nBitmapBufSize == 0)
    {
        std::cerr << "Warning read materials bitmap data size == 0!\n";
        return false;
    }

    /* Read material header list from file stream */
    std::vector<CndMatHeader> matHeaders(cndHeader.numMaterials);
    if(!ifs.read(reinterpret_cast<char*>(&matHeaders[0]), sizeof(CndMatHeader) * matHeaders.size()))
    {
        std::cerr<< "Cnd file error could not read material headers: " << IosErrorStr(ifs) << "!\n";
        return false;
    }

    /* Get get patching material pixel data offset and size */
    std::size_t offMat = 0;
    std::size_t matSize = 0;
    for(auto&& matHeader : matHeaders)
    {
        if(matHeader.mipmapCount < 1 || matHeader.texturesPerMipmap < 1)
        {
            std::cerr << "Warning: No pixel data found for material: " << matHeader.name << std::endl;
            continue;
        }

        if(matHeader.colorInfo.bpp % 8 != 0)
        {
            std::cerr << "Error: Cannot extract material " << matHeader.name << " from buffer. Wrong bitdepth size: " <<  matHeader.colorInfo.bpp << std::endl;
            return false;
        }

        /* Do we have patching material header? */
        if(matHeader.name == mat.name())
        {
            /* Calculate material's mipmap size */
            for(int32_t i = 0; i < matHeader.mipmapCount; i++){
                matSize += GetMipmapPixelDataSize(matHeader.texturesPerMipmap, matHeader.width, matHeader. height, matHeader.colorInfo.bpp);
            }

            matHeader.width  = mat.width();
            matHeader.height = mat.height();
            matHeader.mipmapCount = mat.mipmaps().size();
            matHeader.texturesPerMipmap = mat.mipmaps().at(0).size();
            matHeader.colorInfo = mat.colorFormat();

            break;
        }

        for(int32_t i = 0; i < matHeader.mipmapCount; i++){
            offMat += GetMipmapPixelDataSize(matHeader.texturesPerMipmap, matHeader.width, matHeader. height, matHeader.colorInfo.bpp);
        }
    }

    if(offMat >=  nBitmapBufSize || matSize == 0)
    {
        std::cerr << "Error cannot replace material in cnd file: material not found!\n";
        return false;
    }


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Patch cnd file
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* Open output file stream */
    std::ofstream ofs(filename + ".tmp", std::ios::out | std::ios::trunc | std::ios::binary);
    if (!ofs.is_open())
    {
        std::cerr << "Error opening cnd file for writing!\n";
        return false;
    }

    /* Copy input cnd file to output stream until material list offset */
    ifs.clear();
    ifs.seekg(0, std::ios::beg);

    std::vector<byte_t> buffer(4096);
    while(ifs.tellg() < offMaterialList)
    {
        if(ifs.tellg() + (std::fstream::pos_type)buffer.size() >= offMaterialList) {
            buffer.resize(offMaterialList - ifs.tellg());
        }

        if(!ifs.read(reinterpret_cast<char*>(&buffer[0]), buffer.size()))
        {
            std::cerr << "Error reading CND file into buffer:" << IosErrorStr(ifs) << "!\n";
            return false;
        }

        if(!ofs.write(reinterpret_cast<const char*>(&buffer[0]), buffer.size()))
        {
            std::cerr << "Error writing buffer to CND file:" << IosErrorStr(ofs) << "!\n";
            return false;
        }
    }

// Read material list
    /* Seek to materials pixel data */
    if(!ifs.seekg(sizeof(unknown) + sizeof(nBitmapBufSize) + (cndHeader.numMaterials * sizeof(CndMatHeader)), std::ios::cur)
            || ifs.peek() == EOF)
    {
        std::cerr << "Cnd file error, could not seek to materials bitmap data: " << IosErrorStr(ifs) << std::endl;
        return false;
    }

    /* Read materials pixel data from input file stream */
    buffer.resize(nBitmapBufSize);
    if(!ifs.read(reinterpret_cast<char*>(&buffer[0]), buffer.size()))
    {
        std::cerr<< "Cnd file error, could not read materials bitmap data into buffer: " << IosErrorStr(ifs) << "!\n";
        return false;
    }

// Write material list
    if(!ofs.write(reinterpret_cast<const char*>(&unknown), 4))
    {
        std::cerr << "Error writing buffer to CND file:" << IosErrorStr(ofs) << "!\n";
        return false;
    }

    /* Write pixel data size */
    if(!ofs.write(reinterpret_cast<const char*>(&nBitmapBufSize), 4))
    {
        std::cerr << "Error writing buffer to CND file:" << IosErrorStr(ofs) << "!\n";
        return false;
    }

    /* Write headers */
    if(!ofs.write(reinterpret_cast<const char*>(matHeaders.data()), sizeof(CndMatHeader) * matHeaders.size()))
    {
        std::cerr << "Error writing material headers to CND file:" << IosErrorStr(ofs) << "!\n";
        return false;
    }

    /* Write pixel data until offset of material that's being patched */
    if(!ofs.write(reinterpret_cast<const char*>(buffer.data()), offMat))
    {
        std::cerr << "Error writing materials to CND file:" << IosErrorStr(ofs) << "!\n";
        return false;
    }

    /* Write patched material mipmaps to Cnd file */
    for(const auto& mipmap : mat.mipmaps())
    {
        for(const auto& tex : mipmap)
        {
            if(!ofs.write(reinterpret_cast<const char*>(tex.bitmap()->data()), tex.bitmap()->size()))
            {
                std::cerr << "Error writing new material to CND file:" << IosErrorStr(ofs) << "!\n";
                return false;
            }
        }
    }

    /* Write the rest of materials bitmamp data to cnd file */
    if(!ofs.write(reinterpret_cast<const char*>(&buffer[offMat + matSize]), buffer.size() - (offMat + matSize)))
    {
        std::cerr << "Error writing materials to CND file:" << IosErrorStr(ofs) << "!\n";
        return false;
    }

// Write the rest of cnd file to output stream
    buffer.resize(4096);
    while(ifs.tellg() < cndHeader.fileSize)
    {
        if((std::size_t)ifs.tellg() + buffer.size() >= cndHeader.fileSize) {
            buffer.resize(cndHeader.fileSize - ifs.tellg());
        }

        if(!ifs.read(reinterpret_cast<char*>(&buffer[0]), buffer.size()))
        {
            std::cerr << "Error reading CND file into buffer:" << IosErrorStr(ifs) << "!\n";
            return false;
        }

        if(!ofs.write(reinterpret_cast<const char*>(&buffer[0]), buffer.size()))
        {
            std::cerr << "Error writing buffer to CND file:" << IosErrorStr(ofs) << "!\n";
            return false;
        }
    }

    remove(filename.c_str());
    std::rename((filename + ".tmp").c_str(), filename.c_str());
    return true;
}

#endif // CND_H
