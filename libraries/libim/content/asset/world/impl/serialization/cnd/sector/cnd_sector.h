#ifndef LIBIM_CND_SECTOR_H
#define LIBIM_CND_SECTOR_H
#include "../cnd.h"
#include <cstdint>

#include <libim/content/asset/world/sector.h>
#include <libim/types/flags.h>

namespace libim::content::asset {
    struct CndSectorHeader final
    {
        Flags<Sector::Flag> flags;
        LinearColor    ambientLight;
        LinearColor    extraLight;
        LinearColorRgb tint; // tint rgb color
        Vector3f avgLightPos;       // point light position
        LinearColor    avgLightInt; // point light intensity
        Vector2f avgLightFalloff;
        Box3f    collideBox;
        Box3f    boundBox;
        CndResourceName ambientSound; // The name of sound file
        float    ambientSoundVolume;
        Vector3f center;
        float    radius;
        int32_t  firstSurfaceIdx;
        int32_t  surfacesCount;
        int32_t  verticesCount;
        int32_t  pvsIdx;
        Vector3f thrust;
    };
    static_assert(sizeof(CndSectorHeader) == 244);

}
#endif // LIBIM_CND_SECTOR_H
