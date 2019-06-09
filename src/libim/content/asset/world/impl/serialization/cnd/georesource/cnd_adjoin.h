#ifndef LIBIM_CND_ADJOIN_H
#define LIBIM_CND_ADJOIN_H
#include <cstdint>
#include "../../../../surface_adjoin.h"

namespace libim::content::asset {

    struct CndSurfaceAdjoin
    {
        SurfaceAdjoin::Flag flags;
        int32_t mirror;   // -1 = no mirror
        float distance;
    };

}
#endif // LIBIM_CND_ADJOIN_H
