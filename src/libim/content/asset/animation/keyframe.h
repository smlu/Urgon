#ifndef LIBIM_KEYFRAME_H
#define LIBIM_KEYFRAME_H
#include <cstdint>

#include "../../../math/math.h"
#include "../../../math/rotator.h"
#include "../../../math/vector3.h"

namespace libim::content::asset {

    struct Keyframe final
    {
        // Ref: https://www.massassi.net/jkspecs/
        //     See also sw jk2 atwalk.key
        enum Flag
        {
            NoChange       = 0,
            PositionChange = 1,
            RotationChange = 2
        };

        float number;
        Flag  flags;

        FVector3 pos;
        FVector3 dpos;

        FRotator rot;
        FRotator drot;
    };

    inline constexpr bool operator == (const Keyframe& kf1, const Keyframe& kf2)
    {
        return cmpf(kf1.number, kf2.number) &&
               kf1.flags == kf2.flags &&
               kf1.pos   == kf2.pos   &&
               kf1.dpos  == kf2.dpos  &&
               kf1.rot   == kf2.rot   &&
               kf1.drot  == kf2.drot;
    }
}
#endif // LIBIM_KEYFRAME_H
