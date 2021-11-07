#ifndef LIBIM_WEAPON_H
#define LIBIM_WEAPON_H
#include "thing.h"

namespace libim::content::asset {
    class Weapon : Thing
    {
    public:
        enum Flag: uint32_t
        {
            NoDamageToShooter       = 0x01,
            ExplodeOnSurfaceHit     = 0x04,
            ExplodeOnThingHit       = 0x08,
            AttachToWall            = 0x80,
            ExplodeAtTimerTimeout   = 0x100,
            ExplodeWhenDamaged      = 0x200,    // When damaged by an explosion
            Unknown_400             = 0x400,    // Could be: weapon projectile will rebound/ricochet off of certain type of surface
            AttachToThing           = 0x800,
            CloseProximityTrigger   = 0x1000,
            InstantImpact           = 0x2000,
            DemageDecay             = 0x4000,   // Damage decays with time to minDamage.
            ObjectTrail             = 0x8000,   // Weapon will have trailthings created after it.
            Unknown_20000           = 0x20000,  // inf_turnerhunt.cog
            Unknown_40000           = 0x40000,
            RicochetOffWorldSurface = 0x80000,  // Rebounds/bounces off of any world surface
            Unknown_200000          = 0x200000,
            ExplodeOnWorldFloorHit  = 0x400000
        };
    };
}
#endif // LIBIM_WEAPON_H
