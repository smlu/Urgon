#ifndef LIBIM_ACTOR_H
#define LIBIM_ACTOR_H
#include "thing.h"

namespace libim::content::asset {
    class Actor : Thing
    {
    public:
        enum Flag: uint32_t
        {
            None              = 0x00,
            CanRotateHead     = 0x01,        // Actor head joint can be rotated (e.g. through SetActorHeadPYR )
            Unknown_2         = 0x02,
            HasHeadLight      = 0x04,        // Actor light intensity is used.
            Invulnerable      = 0x08,        // Actor won't be harmed by other things/physics.
            HeadIsCentered    = 0x10,
            ExplodeWhenKilled = 0x20,        // Actor explodes when killed.
                                             // The `explode` Thing param must be set with explosion template.

            BreathUnderWater  = 0x40,        // Actor won't drawn under water.
            Invisible         = 0x80,        // Actor/player won't be seen/attacked by other actors.
            Droid             = 0x100,       // Actor is robot/droid. Produces robot hit sound fx.
                                             //   e.g.: the 'small_robot' template in 14_inf.cnd has this flag set.
            Boss              = 0x200,
            Deaf              = 0x400,       // Actor cannot hear other actors.
            Blind             = 0x800,       // Actor cannot see other actors.
            Poisoned          = 0x2000,      // Actor was bitten by snake, spider, scorpion etc. Shows death icon in player's HUD.
            Unknown_4000      = 0x4000,      // Makes puppet based movement animation faster.
            Unknown_8000      = 0x8000,      // Makes puppet based movement animation faster.
            SlideSlope        = 0x10000,     // When set if world surface is tilted in z for  more than 39.53 degrees and less than 45.84 degrees
                                             // then the thing object will slide down the slope. Applies default to player no need to set this flag.
                                             // Example: _cutactor in 00_cyn has this flag set

            DelayFire         = 0x20000,     // Actor_mophia.cog, actor_lavaboss.cog
            Immobile          = 0x40000,     // weap_ailasermophia.cog, vol_commie_elevswitch.cog, actor_stickyspider.cog, nub_robotboss.cog
            NoTarget          = 0x100000,    // Player won't aim at thing with this flag set. actor_floateraet.cog
            Disabled          = 0x200000,
            PlayerKilled      = 0x400000,    // Internal flag set by the engine
            Unknown_800000    = 0x800000,    // Found in engine
            Unknown_1000000   = 0x1000000,   // 13_nub_cinematic_end.cog, 
            Unknown_8000000   = 0x8000000,   // Player has this flag set (00_cyn - shirtplayer template, _cutactor).
            FlayerMove        = 0x10000000,  // actor_mophia.cog
            ElectricWhip      = 0x40000000,
            Arachnid          = 0x80000000   // Spider, scorpion. Flag causes the gen_a4sprite_blood_grn.mat and +vulcansplort to be created when actor is shot.
        };
    };
}

#endif // LIBIM_ACTOR_H
