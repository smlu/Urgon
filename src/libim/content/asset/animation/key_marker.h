#ifndef KEY_MARKER_H
#define KEY_MARKER_H
#include <cstdint>


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
}
#endif // KEY_MARKER_H
