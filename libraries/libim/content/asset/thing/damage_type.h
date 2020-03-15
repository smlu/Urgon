#ifndef LIBIM_DAMAGE_TYPE_H
#define LIBIM_DAMAGE_TYPE_H
#include <cstdint>

namespace libim::content::asset {
    enum class DamageType : uint32_t
    {
        None             = 0x00,
        Impact           = 0x01,       // actor_robotboss.cog
        Energy           = 0x02,       // actor_robotsc.cog
        Fire             = 0x04,       // actor_robotsc.cog
        Fists            = 0x08,       // or punch - actor_robotguard.cog
        Whip             = 0x10,       // actor_robotguard.cog
        Crunch           = 0x20,       // actor_boomthingwood.cog
        Drown            = 0x40,       // engine
        Crush            = 0x80,       // actor_floateraet.cog
        Poison           = 0x100,      // actor_indy.cog
        Lava             = 0x200,      // tem_lavaboss.cog
        ElectricWhip     = 0x800,      // actor_mophia.cog
        Imp1             = 0x1000,     // Infernal Machine Part 1 - shw_iceboss.cog
        Imp4             = 0x4000,     // Infernal Machine Part 4
        Imp5             = 0x5000,     // Infernal Machine Part 5
        Lightning        = 0x100000,   // actor_robotboss.cog
        Spike            = 0x400000,   // olv_quetzalcoatl.cog
        Unknown_800000   = 0x800000,
        Quetz            = 0x1000000,  // olv_quetzalcoatl.cog
        Vehicle          = 0x2000000,  // jeep, mine car
        Bonk             = 0x4000000,  // actor_indymine.cog
        Debris           = 0x8000000,  // 00_cny debris template's weapon info
        Unknown_10000000 = 0x10000000, // game engine
        IceMonster       = 0x20000000, // actor_iceboss.cog
        ColdWater        = 0x40000000, // actor_indy.cog
        Chicken          = 0x80000000  // actor_lavaguy.cog
    };
}

#endif // LIBIM_DAMAGE_TYPE_H
