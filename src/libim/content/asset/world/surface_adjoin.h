#ifndef LIBIM_SURFACE_ADJOIN_H
#define LIBIM_SURFACE_ADJOIN_H
#include <cstdint>
#include <string>
#include <optional>

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
