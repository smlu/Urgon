#ifndef LIBIM_CND_ADJOIN_H
#define LIBIM_CND_ADJOIN_H
#include <cstdint>
#include <libim/content/asset/world/surface_adjoin.h>
#include <libim/types/flags.h>

namespace libim::content::asset {

    struct CndSurfaceAdjoin
    {
        Flags<SurfaceAdjoin::Flag> flags;
        int32_t mirror;   // -1 = no mirror
        float distance;
    };
    static_assert(sizeof(CndSurfaceAdjoin) == 12);
}
#endif // LIBIM_CND_ADJOIN_H
