#ifndef LIBIM_LIGHT_MODE_H
#define LIBIM_LIGHT_MODE_H
#include <cstdint>

namespace libim::content::asset {
    enum class LightMode : uint32_t
    {
        Unlit     = 0,
        Lit       = 1,
        Diffuse   = 2,
        Gouraud   = 3,
        Gouraud2  = 4,
        Gouraud3  = 5
    };
}
#endif // LIBIM_LIGHT_MODE_H
