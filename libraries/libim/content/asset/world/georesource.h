#ifndef LIBIM_CND_GEORESOURCE_H
#define LIBIM_CND_GEORESOURCE_H
#include <cstdint>
#include <vector>

#include "surface_adjoin.h"
#include "surface.h"

#include <libim/math/vector2.h>
#include <libim/math/vector3.h>

namespace libim::content::asset {
    struct Georesource
    {
        std::vector<Vector3f> vertices;
        std::vector<Vector2f> texVertices;
        std::vector<SurfaceAdjoin> adjoins;
        std::vector<Surface> surfaces;
    };

    inline bool operator == (const Georesource& lhs, const Georesource& rhs)
    {
        return lhs.vertices    == rhs.vertices    &&
               lhs.texVertices == rhs.texVertices &&
               lhs.adjoins     == rhs.adjoins     &&
               lhs.surfaces    == rhs.surfaces;
    }

    inline bool operator != (const Georesource& lhs, const Georesource& rhs) {
        return !(lhs == rhs);
    }
}
#endif // LIBIM_CND_GEORESOURCE_H
