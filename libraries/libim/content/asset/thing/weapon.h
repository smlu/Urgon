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
            ImpactSoundFx           = 0x400,    // When weapon has damage class = 0x20 - Crunch (machete) and has this flag set, the impacted world surface produces hit sound effect.
                                                //  e.g. gen_machete_hit_metal.wav if surface is metal, gen_machete_hit_web.wav if surface is spider web etc ...
            AttachToThing           = 0x800,
            CloseProximityTrigger   = 0x1000,
            InstantImpact           = 0x2000,
            DamageDecay             = 0x4000,   // Damage decays with time to minDamage.
            ObjectTrail             = 0x8000,   // Weapon will have trail things created after it.
            Unknown_20000           = 0x20000,  // inf_turnerhunt.cog
            Unknown_40000           = 0x40000,
            RicochetOffSurface      = 0x80000,  // Rebounds/bounces off of surface. Usually should hit COG thing which will produce bounce off sound fx.
            TriggerAiEvent          = 0x200000, // Triggers AI event for hit AI actor. e.g. if AI actor is sleeping it will be woken up & COG msg 0x1000 - EVENT_TARGETED will be sent to actor's cog script.
            ExplodeOnWorldFloorHit  = 0x400000,
            MophiaBomb              = 0x800000  // Marduk weapon when he's in mophia form.
                                                // When player holding mirror and projectile with this flag is set, it is reflected of the mirror by creating new projectile from `+mardukhit` template.
                                                // Also hiting player without holding mirror plays 'aet_mr_hit_indy.wav'.
                                                // i.e. `+mardukhit` template has this flag set
        };
    };
}
#endif // LIBIM_WEAPON_H
