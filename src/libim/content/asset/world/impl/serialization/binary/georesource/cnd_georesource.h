#ifndef LIBIM_CND_GEORESOURCE_H
#define LIBIM_CND_GEORESOURCE_H
#include <cstdint>
#include <vector>

#include "cnd_adjoin.h"
#include "cnd_surface.h"
#include "../../../../../../../math/vector2.h"
#include "../../../../../../../math/vector3.h"

namespace libim::content::asset {
    struct Georesource
    {
        std::vector<Vector3f> verts;
        std::vector<Vector2f> texVerts;
        std::vector<CndSurfaceAdjoin> adjoints;
        std::vector<CndSurface> surfaces;
    };
}
#endif // LIBIM_CND_GEORESOURCE_H
