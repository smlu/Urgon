#ifndef LIBIM_SURFACE_ADJOIN_H
#define LIBIM_SURFACE_ADJOIN_H
#include <cstdint>
#include <string>
#include <optional>

#include <libim/math/math.h>
#include <libim/types/flags.h>


namespace libim::content::asset {

    struct SurfaceAdjoin final
    {
        enum  Flag
        {
            Visible            = 0x1,  // Renders through the adjoin surface to the mirroved adjoin surface to adjacent sector.
            AllowMovement      = 0x2,
            AllowSound         = 0x4,
            AllowPlayerOnly    = 0x8,
            NoPlayerMove       = 0x10, // gen_slashweb.cog
            AdjoinSetBySector  = 0x20  // flag is in connection with sector flag 0x80. 
                                       //  This flag is set when cog fuction SetSectorAdjoins(sector, visible=false) is called and unset with SetSectorAdjoins(sector, visible=true).
        };

        Flags<Flag> flags;
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
