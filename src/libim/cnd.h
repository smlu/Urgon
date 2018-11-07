#ifndef LIBIM_CND_H
#define LIBIM_CND_H
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
#include "io/filestream.h"
#include "io/stream.h"

namespace libim::CND {

//PACKED(
struct CndHeader
{
    uint32_t fileSize;
    std::array<char, 1216> copyright;
    std::array<char, 64>   filePath;
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
    uint32_t aMaterials;     // 32-bit void*
    uint32_t unknown4[13];
    uint32_t aSelectors;
    uint32_t unknown5;
    uint32_t worldAIClasses;
    uint32_t unknown6[2];
    uint32_t numModels;
    uint32_t sizeModels;
    uint32_t aModels;
    uint32_t numSprites;
    uint32_t sizeSprites;
    uint32_t aSprites;      // 32-bit void*
    uint32_t numKeyframes;
    uint32_t sizeKeyframes;
    uint32_t aKeyframes;    // 32-bit void*
    uint32_t unknown9[20];
    uint32_t worldSounds;
    uint32_t worldSoundUnknown; // Size of sound data
};//);

struct CndMatHeader
{
    char name[64];
    int width;
    int height;
    int mipmapCount;
    int texturesPerMipmap;
    ColorFormat colorInfo;
};


CndHeader LoadHeader(const InputStream& istream);

uint32_t GetMatSectionOffset(const CndHeader& header);
std::vector<Material> LoadMaterials(const InputStream& istream);
bool ReplaceMaterial(const Material& mat, const std::string& filename);

}}
#endif // LIBIM_CND_H
