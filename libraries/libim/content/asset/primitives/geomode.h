#ifndef LIBIM_GEOMODE_H
#define LIBIM_GEOMODE_H
#include <cstdint>

namespace libim::content::asset {
    enum class GeoMode : uint32_t
    {
        NotDrawn    = 0,
        Points      = 1,
        Wireframe   = 2,
        Solid       = 3,
        Textured    = 4,
        Full        = 5
    };
}
#endif // LIBIM_GEOMODE_H
