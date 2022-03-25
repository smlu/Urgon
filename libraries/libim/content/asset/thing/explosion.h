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
        enum Flag : uint32_t
        {
            None                          = 0x00,
            AnimatedSprite                = 0x01,   // If set, sprite material is animated (spriteThing or sprite)
            HasBlastPhase                 = 0x02,   // If set, msBlastTime field is used
            DamageInBlastRadius           = 0x04,   // If set, range field and damage field are used
            HasChildExplosion             = 0x08,   // If set, the msBabyTime field is used
            VariableLight                 = 0x10,   // If set, the maxLight field is used (could be 0)
            RandomSpriteThingOrientation  = 0x20,   // If set, spriteThing is randomly orientated
            NoDamageToShooter             = 0x40,
            RandomDebris                  = 0x80,   // When set the aDebrisThings field might be used
            FlashBlindsThings             = 0x100,
            AnimateDebrisMaterial         = 0x200,  // When set the textures of 3D model of debris are animated
            UpdateDebrisMaterial          = 0x400,  // When set all 3D models of debris have their textures changes to internally cached material. Overrides 0x200.
                                                    //    In case explosion is created as part of a weapon explosion the material of the impacted surface is used.
            ExpandTimeSet                 = 0x800,
            FadeTimeSet                   = 0x1000
        };
    };
}

#endif // LIBIM_EXPLOSION_H
