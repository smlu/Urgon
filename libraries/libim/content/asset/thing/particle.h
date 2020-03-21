#ifndef LIBIM_PARTICLE_H
#define LIBIM_PARTICLE_H
#include "thing.h"
#include <cstdint>

namespace libim::content::asset {
    class ParticleThing final : Thing
    {
    public:
        enum Flag: uint32_t
        {
            None              = 0x00,
            OutwardExpanding  = 0x01,    // If set, the growthSpeed field is used
            AnimateCel        = 0x02,
            RandomStartCel    = 0x04,
            FadeoutOverTime   = 0x08,    // If set, the timeoutRate field is used
            EmitLight         = 0x10,
            Flipped           = 0x20,
            Unknown_40        = 0x40
        };
    };
}
#endif // LIBIM_PARTICLE_H
