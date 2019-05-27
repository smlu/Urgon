#ifndef LIBIM_CND_ADJOIN_H
#define LIBIM_CND_ADJOIN_H
#include <cstdint>

namespace libim::content::asset {

    enum class SurfaceAdjoinFlags
    {
        Unknown = 3,

    };

    struct CndSurfaceAdjoin
    {
        SurfaceAdjoinFlags flags;
        int32_t mirror;   // -1 = no mirror
        float distance;
    };

    /* Jones3D engine's adjoin struct
        struct SurfaceAdjoin
        {
          int flags;
          int unknown1;
          Surface *pSurface;
          SurfaceAdjoin *pMirror;
          int unknown4;
          int unknown5;
          float distance;
        };
    */

}
#endif // LIBIM_CND_ADJOIN_H
