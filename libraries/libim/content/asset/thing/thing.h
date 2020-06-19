#ifndef LIBIM_THING_H
#define LIBIM_THING_H
#include <cstdint>

namespace libim::content::asset {
    class Thing
    {
    public:
        enum Type : uint32_t
        {
            Free      = 0,
            Camera    = 1,
            Actor     = 2,
            Weapon    = 3,
            Debris    = 4,
            Item      = 5,
            Explosion = 6,
            Cog       = 7,
            Ghost     = 8,
            Corpse    = 9,
            Player    = 10,
            Particle  = 11,
            Hint      = 12,
            Sprite    = 13,
            Polyline  = 14
        };

        enum class Flag : uint32_t
        {
            None                    = 0x0,
            EmitsLight              = 0x1,       // When set the Light property in sithThing is initialized
            Dead                    = 0x2,
            NoWeaponCollide         = 0x4,
            PartOfWorldGeometery    = 0x8,       // whip climb in shs_whip.cog
            Invisible               = 0x10,      // but touchable unlike hidden
            CanStandOn              = 0x40,      // nub_lightning.cog
            Mountable               = 0x80,      // pyr_fixminecar.cog, shs_barrel.cog
            Unknown_100             = 0x100,     // Possible: something with invinsible player thing
            Killed                  = 0x200,
            CogLinked               = 0x400,
            NoCrush                 = 0x800,
            NotInEasy               = 0x1000,     // unknown
            Wood                    = 0x2000,     // thing is wood object
            HasShadow               = 0x4000,     // shadow on: 11_pyr_kidvsindy_2.cog, teo_introscene.cog, shs_ctladder.cog, pyr_openingcutscene.cog
            NotInMultiPlayer        = 0x8000,     // unknown
            Snow                    = 0x10000,    // thing is snow object
            SendingPulseEvent       = 0x20000,    // sends pulse interval timeout event to its cog
            SendingTimerEvent       = 0x40000,    // sends timer timeout event to its cog
            Hidden                  = 0x80000,    // sol_transformer.cog, sol_redgem.cog, teo_pool.cog, teo_spiderblock.cog, vol_co-op_room.cog, pru_whipspike.cog ...
            Metal                   = 0x400000,   // thing is metal object
            Earth                   = 0x800000,   // thing is earth (dirt) object
            Seen                    = 0x100000,   // mark Thing as seen by player. See: gen_bubblegen.cog, gen_watersplash.cog
            NoSounds                = 0x1000000,
            Underwater              = 0x2000000,  // swimming weap_machete.cog, teo_drops.cog, nub_lightning.cog, pru_caveamb.cog
            ClimbableCreate         = 0x4000000,  // lag_boomrotate.cog
            DestroyedInWater        = 0x8000000,
            DestroyedInAir          = 0x10000000, // class_mine.cog
            GeneratesSplash         = 0x20000000, // sends splash message to cog
            Movable                 = 0x40000000, // pushable/movable
            Whippable               = 0x80000000
        };
    };
}

#endif // LIBIM_THING_H
