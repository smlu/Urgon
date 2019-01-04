#ifndef LIBIM_KEY_NODE_ENTRY_H
#define LIBIM_KEY_NODE_ENTRY_H
#include <cstdint>

#include "../../../math/math.h"
#include "../../../math/rotator.h"
#include "../../../math/vector3.h"

namespace libim::content::asset {

    struct KeyNodeEntry final
    {
        // Ref: https://www.massassi.net/jkspecs/
        //     See also sw jk2 atwalk.key
        enum Flag
        {
            NoChange       = 0,
            PositionChange = 1,
            RotationChange = 2
        };

        float frame;
        Flag  flags;

        Vector3f pos;
        FRotator rot;

        Vector3f dpos;
        FRotator drot;
    };

    inline constexpr bool operator == (const KeyNodeEntry& kf1, const KeyNodeEntry& kf2)
    {
        return libim::cmpf(kf1.frame, kf2.frame) &&
               kf1.flags == kf2.flags &&
               kf1.pos   == kf2.pos   &&
               kf1.dpos  == kf2.dpos  &&
               kf1.rot   == kf2.rot   &&
               kf1.drot  == kf2.drot;
    }
}
#endif // LIBIM_KEY_NODE_ENTRY_H
