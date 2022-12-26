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
            Destroyed               = 0x2,       // Thing is destroyed and will be removed from the game.
            NoWeaponCollide         = 0x4,
            WhipClimbable           = 0x8,       // On the WhipAim surface player can mount whip on thing and climb up the whip.
                                                 //   e.g.: shs_whip.cog  Note: if set, thing can't have flag WhipSwingable

            Invisible               = 0x10,      // Thing is invisible but touchable unlike hidden.
            CanStandOn              = 0x40,      // nub_lightning.cog
            Mountable               = 0x80,      // pyr_fixminecar.cog, shs_barrel.cog
            Unknown_100             = 0x100,     // Possible: disables sending cog messages to linked cog script. In corelation with 0x400 flag
            ToBeRemoved             = 0x200,     // Used in combination with 'LIFELEFT' Thing param. Usually used by the engine marking the Object has been killed and to be removed from the game.
            CogLinked               = 0x400,     // Thing is used in COG.
            NoCrush                 = 0x800,
            Unknown_1000            = 0x1000,
            Wood                    = 0x2000,     // Thing is wood object
            HasShadow               = 0x4000,     // shadow on: 11_pyr_kidvsindy_2.cog, teo_introscene.cog, shs_ctladder.cog, pyr_openingcutscene.cog
            Unknown_8000            = 0x8000,
            Snow                    = 0x10000,    // Thing is snow object
            SendingPulseEvent       = 0x20000,    // Sends pulse interval timeout event to its COG
            SendingTimerEvent       = 0x40000,    // Sends timer timeout event to its COG
            Hidden                  = 0x80000,    // sol_transformer.cog, sol_redgem.cog, teo_pool.cog, teo_spiderblock.cog, vol_co-op_room.cog, pru_whipspike.cog ...
            Seen                    = 0x100000,   // Mark Thing as seen by player. See: gen_bubblegen.cog, gen_watersplash.cog
            Unknown_200000          = 0x200000,   // Internal engine flag
            Metal                   = 0x400000,   // Thing is metal object
            Earth                   = 0x800000,   // Thing is earth (dirt) object
            NoSound                 = 0x1000000,  // Thing makes no sound
            Underwater              = 0x2000000,  // Swimming.
                                                  //   e.g.: weap_machete.cog, teo_drops.cog, nub_lightning.cog, pru_caveamb.cog
            ClimbableCreate         = 0x4000000,  // Player can clib onto Thing. e.g.: lag_boomrotate.cog
            DestroyedInWater        = 0x8000000,  // Thing is destroyed when enters the water sector
            DestroyedInAir          = 0x10000000, // Thing is destroyed when exits water sector. class_mine.cog
            GeneratesSplash         = 0x20000000, // Sends splash message to COG
            Movable                 = 0x40000000, // Thing is pushable/movable.
            WhipSwingable           = 0x80000000  // On the WhipAim surface player can whip swing using thing as whip mount point.
                                                  //   e.g.: In 00_cyn thing whip_branch
        };
    };
}

#endif // LIBIM_THING_H
