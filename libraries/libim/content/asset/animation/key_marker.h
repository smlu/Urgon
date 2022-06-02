#ifndef KEY_MARKER_H
#define KEY_MARKER_H
#include <cstdint>
#include <libim/math/math.h>

namespace libim::content::asset {

    struct KeyMarker final
    {
        enum Type : uint32_t
        {
            Marker               = 0,
            LeftFoot             = 1,
            RightFoot            = 2,
            Attack               = 3,
            Swing                = 4,
            SwingFinish          = 5,
            SwimLeft             = 6,
            RunLeftFoot          = 8,
            RunRightFoot         = 9,
            Died                 = 10,
            Jump                 = 11,
            SwimRight            = 13,
            Duck                 = 14,
            Climb                = 15,
            Activate             = 16,
            Crawl                = 17,
            RunJumpLand          = 18,
            ActivateRightArm     = 19,
            ActivateRightArmRest = 20,
            PlaceRightArm        = 21,
            PlaceRightArmRest    = 22,
            ReachRightArm        = 23,
            ReachRightArmRest    = 24,
            Pickup               = 25,
            Drop                 = 26,
            Move                 = 27,
            InventoryPull        = 28,
            InventoryPut         = 29,
            AttackFinish         = 30,
            TurnOff              = 31,
            Unknown_32           = 32,
            Unknown_33           = 33,
            LeftSide             = 34,
            RightSide            = 35
        };

        float frame;
        Type type;
    };


    constexpr bool operator == (const KeyMarker& k1, const KeyMarker& k2){
        return cmpf(k1.frame, k2.frame) && k1.type == k2.type;
    }
}
#endif // KEY_MARKER_H
