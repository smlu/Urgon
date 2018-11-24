#ifndef LIBIM_KEYFRAME_H
#define LIBIM_KEYFRAME_H
#include <cstdint>

#include "../../../math/vector3.h"
#include "../../../math/rotator.h"

namespace libim::content::asset {

    struct Keyframe
    {
        // Ref: https://www.massassi.net/jkspecs/
        //     See also sw jk2 atwalk.key
        enum Flag
        {
            NoChange       = 0,
            PositionChange = 1,
            RotationChange = 2
        };

        uint32_t number;
        Flag     flags;
        FVector3 pos;
        FVector3 dpos;
        FRotator rot;
        FRotator drot;
    };
}
#endif // LIBIM_KEYFRAME_H
