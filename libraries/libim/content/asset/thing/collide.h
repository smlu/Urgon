#ifndef LIBIM_COLLIDE_H
#define LIBIM_COLLIDE_H
#include <cstdint>

namespace libim::content::asset {
    struct Collide
    {
        enum Type : uint32_t
        {
            None   = 0x0,
            Sphere = 0x1,
            Face   = 0x3,
        };

        Collide::Type type;
        double movesize;
        double size;
        double collideWidth;
        double collideHeight;
        double unknown4;
        double unknown5;
    };
}
#endif // LIBIM_COLLIDE_H
