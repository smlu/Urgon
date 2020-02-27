#ifndef LIBIM_SURFACE_ADJOIN_H
#define LIBIM_SURFACE_ADJOIN_H
#include <cstdint>
#include <string>
#include <optional>
#include "../../../math/math.h"


namespace libim::content::asset {

    struct SurfaceAdjoin final
    {
        enum  Flag
        {
            Visible            = 0x1,
            AllowMovement      = 0x2,
            AllowSound         = 0x4,
            AllowPlayerOnly    = 0x8,
            AllowAiOnly        = 0x10
        };

        Flag flags;
        std::optional<std::size_t> mirrorIdx; // mirror adjoin
        std::optional<std::size_t> surfaceIdx;
        std::optional<std::size_t> sectorIdx;
        float distance;
    };

    inline bool operator == (const SurfaceAdjoin& lhs, const SurfaceAdjoin& rhs)
    {
        return lhs.flags      == rhs.flags      &&
               lhs.mirrorIdx  == rhs.mirrorIdx  &&
               lhs.surfaceIdx == rhs.surfaceIdx &&
               lhs.sectorIdx  == rhs.sectorIdx  &&
               cmpf(lhs.distance, rhs.distance);
    }

    inline bool operator != (const SurfaceAdjoin& lhs, const SurfaceAdjoin& rhs) {
        return !(lhs == rhs);
    }


    /* Jones3D engine's adjoin struct
        struct SurfaceAdjoin
        {
          int flags;
          SithSector* pAdjoinSector;
          Surface *pAdjoinSurface;
          SurfaceAdjoin *pMirrorAdjoin;
          int unknown4;
          int unknown5;
          float distance;
        };
    */
}

#endif // LIBIM_SURFACE_ADJOIN_H
