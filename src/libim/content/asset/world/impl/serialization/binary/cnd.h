#ifndef LIBIM_CND_H
#define LIBIM_CND_H
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>


#include "../../../../animation/animation.h"
#include "../../../../material/material.h"

#include "../../../../../../common.h"
#include "../../../../../../io/stream.h"
#include "../../../../../../utils/hashmap.h"

namespace libim::content::asset {

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




    struct CND final
    {
        static CndHeader LoadHeader(const InputStream& istream);

        static uint32_t GetMatSectionOffset(const CndHeader& header);
        static utils::HashMap<Material> ParseSectionMaterials(const CndHeader& header, const InputStream& istream); // Reads materials section. Offset of istream hast to be at beginning of material section.
        static utils::HashMap<Material> ReadMaterials(const InputStream& istream);
        static bool ReplaceMaterial(const Material& mat, const std::string& filename);

        static std::size_t GetKeySectionOffset(const CndHeader& header, const InputStream& istream);
        static utils::HashMap<Animation> ParseSectionKeyframes(const CndHeader& cndHeader, const InputStream& istream); // Reads keyframes section. Offset of istream hast to be at beginning of keyframe section.
        static utils::HashMap<Animation> ReadAnimations(const InputStream& istream);
        static void WriteSectionKeyframes(OutputStream& ostream, const utils::HashMap<Animation>& animations);
    };


}
#endif // LIBIM_CND_H
