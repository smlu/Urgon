#ifndef LIBIM_EXPLOSION_H
#define LIBIM_EXPLOSION_H
#include "thing.h"
#include <cstdint>

namespace libim::content::asset {

    // Explosion represents weapons explosion effects.
    // This can represent fist hit, pistol projectile hit, pistol smoke when fired, explosion itself etc..
    class ExplosionThing : Thing
    {
    public:
        enum Flag: uint32_t
        {
            None                 = 0x00,
            AnimatedSprite       = 0x01,   // If set, fields spriteThing, posSpriteStart and posSpriteEnd are used
            HasBlastPhase        = 0x02,   // If set, msBlastTime field is used
            DamageInBlastRadius  = 0x04,   // If set, range field and damage field are used
            HasChildExplosion    = 0x08,   // If set, the msBabyTime field is used
            VariableLight        = 0x10,   // If set, the maxLight field is used (could be 0)
            Unknown_20           = 0x20,
            NoDamageToShooter    = 0x40,
            RandomDebris         = 0x80,   // If set, the aDebrisThings field might be used
            FlashBlindsThings    = 0x100,
            HasDebris            = 0x200,  // If set, the aDebrisThings field is used
            DamageSet            = 0x400,  // If set, the damage field is used.
            ExpandTimeSet        = 0x800,
            FadeTimeSet          = 0x1000
        };
    };
}

#endif // LIBIM_EXPLOSION_H
