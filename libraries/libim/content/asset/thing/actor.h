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
            Unknown_1         = 0x01,
            Invulnerable      = 0x08,        // Actor won't be harmed by other things/physics.
            HeadIsCentered    = 0x10,
            ExplodeWhenKilled = 0x20,        // Actor explodes when killed.
                                             // The `explode` Thing param must be set with explosion template.

            BreathUnderWater  = 0x40,        // Actor won't drawn under water.
            Invinsible        = 0x80,        // Actor/player won't be seen/attacked by other actors.
            Boss              = 0x200,
            Deaf              = 0x400,       // Actor cannot hear other actors.
            Blind             = 0x800,       // Actor cannot see other actors.
            Poisoned          = 0x2000,      // Actor was biten by snake, spider, scorpion etc. Shows death icon in player's HUD.
            Unknown_10000     = 0x10000,     // _cutactor in 00_cyn has this flag set
            DelayFire         = 0x20000,     // Actor_mophia.cog, actor_lavaboss.cog
            Immobile          = 0x40000,     // weap_ailasermophia.cog, vol_commie_elevswitch.cog, actor_stickyspider.cog, nub_robotboss.cog
            NoTarget          = 0x100000,    // Player won't aim at thing with this flag set. actor_floateraet.cog
            Disabled          = 0x200000,
            PlayerKilled      = 0x400000,    // Set in game engine.
            Unknown_1000000   = 0x1000000,   // 13_nub_cinematic_end.cog
            Unknown_8000000   = 0x8000000,   // Player has this flag set (00_cyn - shirtplayer template, _cutactor).
            FlayerMove        = 0x10000000,  // actor_mophia.cog
            ElectricWhip      = 0x40000000
        };
    };
}

#endif // LIBIM_ACTOR_H
