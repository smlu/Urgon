#ifndef KEY_MARKER_H
#define KEY_MARKER_H
#include <cstdint>
#include <libim/math/math.h>

namespace libim::content::asset {

    struct KeyMarker final
    {
        //Ref https://github.com/jdmclark/gorc/blob/develop/src/libs/libold/content/flags/key_marker_type.hpp
        enum Type : uint32_t
        {
            LeftFootstep      = 1,
            RightFootstep     = 2,
            ActivateBin       = 3,
            SaberUnknown1     = 4,
            SaberUnknown2     = 5,
            SwimLeft          = 6,
            LeftRunFootstep   = 8,
            RightRunFootstep  = 9,
            Death             = 10,
            SwimRight         = 13
        };

        float frame;
        Type type;
    };


    constexpr bool operator == (const KeyMarker& k1, const KeyMarker& k2){
        return cmpf(k1.frame, k2.frame) && k1.type == k2.type;
    }
}
#endif // KEY_MARKER_H
