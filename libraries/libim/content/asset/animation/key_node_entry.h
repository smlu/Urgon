#ifndef LIBIM_KEY_NODE_ENTRY_H
#define LIBIM_KEY_NODE_ENTRY_H
#include <cstdint>

#include <libim/math/math.h>
#include <libim/math/rotator.h>
#include <libim/math/vector3.h>
#include <libim/types/flags.h>

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
        Flags<Flag> flags;

        Vector3f position;
        FRotator rot;

        Vector3f dpos;
        FRotator drot;
    };

    inline constexpr bool operator == (const KeyNodeEntry& kf1, const KeyNodeEntry& kf2)
    {
        return libim::cmpf(kf1.frame, kf2.frame) &&
               kf1.flags    == kf2.flags         &&
               kf1.position == kf2.position      &&
               kf1.dpos     == kf2.dpos          &&
               kf1.rot      == kf2.rot           &&
               kf1.drot     == kf2.drot;
    }
}
#endif // LIBIM_KEY_NODE_ENTRY_H
