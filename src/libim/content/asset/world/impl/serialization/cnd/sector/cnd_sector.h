#ifndef LIBIM_CND_SECTOR_H
#define LIBIM_CND_SECTOR_H
#include <cstdint>
#include "../../../../sector.h"

namespace libim::content::asset {

    struct CndSectorHeader
    {
        Sector::Flag flags;
        Color    ambientLight;
        Color    extraLight;
        ColorRgb tint; // tint rgb color
        Vector3f avgLightPos;
        Color    avgLightInt; // average light intensity
        Vector2f avgLightFalloff;
        Box3f    collideBox;
        Box3f    boundBox;
        std::array<char, 64> ambientSound = {0};
        float    ambientSoundVolume;
        Vector3f center;
        float    radius;
        int32_t  firstSurfaceIdx;
        int32_t  surfacesCount;
        int32_t  verticesCount;
        int32_t  unknown;
        Vector3f thrust;
    };
}
#endif // LIBIM_CND_SECTOR_H
