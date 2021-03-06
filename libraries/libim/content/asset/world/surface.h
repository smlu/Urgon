#ifndef LIBIM_SURFACE_H
#define LIBIM_SURFACE_H
#include <cstdint>
#include <string>

#include "../primitives/face.h"
#include <libim/types/flags.h>

namespace libim::content::asset {

    struct Surface final : public Face
    {
        enum SurfaceFlag : uint32_t
        {
            None                     = 0x0,
            Floor                    = 0x1,        // Surface is ground floor. Could be face collision if Collision is set.
            CogLinked                = 0x2,        // Surface is linked to cog script.
            Collision                = 0x4,        // Surface has collision.
            AiCannotWalkOnFloor      = 0x8,        // Ai won't walk on this surface
            DoubleSurfaceScale       = 0x10,       // When set, it affects SlideWall() cog function.
                                                   //   By default the animating surface matrix is resized to 320x320, this flag resizes it to 640x640

            HalfSurfaceScale         = 0x20,       // When set, it affects SlideWall() cog function.
                                                   //   By default the animating surface matrix is resized to 320x320, this flag resizes it to 160x160

            EighthSurfaceScale       = 0x40,       // When set, it affects SlideWall() cog function.
                                                   //   By default the animating surface matrix is resized to 320x320, this flag resizes it to 40x40

            Aetherium                = 0x80,       // Aetherium surface.
                                                   //   If surface is hit or walk on by a thing it produces "aetherium" enviornment effect sound.
                                                   //   e.g.: fol_in_lrunaet.wav, fol_in_rrunaet.wav

            HorizonSky               = 0x200,      // The surface is part of sky box. e.g.: defined in 02_riv
            CeilingSky               = 0x400,      // The surface is part of ceiling sky. e.g.: defined in 00_cyn
            Scrolling                = 0x800,
            KillFloor                = 0x1000,     // Player dies if it touches the surface. e.g.: cyn_killfloor.cog
            Climbable                = 0x2000,
            MineCarTrack             = 0x4000,     // Mine car track.
                                                   //  e.g.: sol_ramp.cog, sol_ctrllever5.cog, sol_ctrllever4.cog, sol_ctrllever3.cog,
                                                   //        sol_ctrllever2.cog, sol_ctrllever1.cog, sol_comfalls.cog, sol_comentrance.cog

            SurfaceChanged           = 0x8000,     // Set by engine to mark the rendering surface should be updated.
            Metal                    = 0x10000,
            Water                    = 0x20000,    // Water surface.
            ShallowWater             = 0x40000,
            Earth                    = 0x80000,    // Dirt surface.
            Web                      = 0x100000,   // Spider web surface.
            Lava                     = 0x200000,   // Lava or frozen water surface. In 02_riv freezing water has this flag set.
            Snow                     = 0x400000,   // Snow surface.
            Wood                     = 0x800000,   // Wood surface.
            Hangable                 = 0x1000000,  // Player can hang on the ledge of this surface. In: tem_bossflood.cog, vol_block_ledgecontrol.cog
            WaterClimbOutLedge       = 0x2000000,
            QuaterSurfaceScale       = 0x4000000,  // When set, it affects SlideWall() cog function.
                                                   //   By default the animating surface matrix is resized to 320x320, this flag resizes it to 80x80

            QuadrupleSurfaceScale    = 0x8000000,  // When set, it affects SlideWall() cog function.
                                                   //   By default the animating surface matrix is resized to 320x320, this flag resizes it to 1280x1280.
                                                   //   e.g.: in 00_cyn.ndy water surfaces

            WhipAim                  = 0x10000000, // Player starts aiming at whippable object on this surface.
                                                   //   e.g.: vol_lekk_door.cog, 00_cyn surface no. 1460

            Echo                     = 0x20000000, // Indoor surface with echo effect when players move on this surface (eg jumphardecho sound class mode)
            WoodEcho                 = 0x40000000, // Indoor wood surface with echo effect when players move on this surface (e.g.: jumpwood sound class mode)
            EarthEcho                = 0x80000000  // Indoor earth surface with echo effect when players move on this surface - vine surface has this flag set.
                                                   //   e.g.: jumpearthecho sound class mode
        };

        using Id = std::size_t;

        Id id;
        Flags<SurfaceFlag> surflags;
        std::optional<std::size_t> adjoinIdx;
        std::vector<LinearColor> vecIntensities;     // verticies color. Color of each vertex is applied over surface's texture and
                                               // can give surface an additional ambient color e.g. underwater blue color.
    };


    inline bool operator == (const Surface& lhs, const Surface& rhs)
    {
        return dynamic_cast<const Face&>(lhs)  == rhs &&
               lhs.id             == rhs.id           &&
               lhs.surflags       == rhs.surflags     &&
               lhs.adjoinIdx      == rhs.adjoinIdx    &&
               lhs.vecIntensities == rhs.vecIntensities;
    }

    inline bool operator != (const Surface& lhs, const Surface& rhs) {
        return !(lhs == rhs);
    }

    /* Jones3d surface struct

       struct Surface
        {
          int unknown0;
          int unknown1;
          SurfaceAdjoin *pAdjoin;
          int surfflags;
          RdFace face;
          RGBA *aIntensities; // vertices color
          int unknown23;
        };

        struct RdFace
        {
          int num;
          int type; // cnd faceflags
          int geoMode;
          int lightMode;
          int nVerts;
          int *aVertIdxs;
          int *aTexIdxs;
          SithMaterial *pMaterial;
          int unknown8;
          int unknown9;
          int unknown10;
          RGBA color;
          Vector3f normal;
        };

        struct SurfaceAdjoin
        {
          int flags;
          int unknown1;
          Surface *pSurface;
          SurfaceAdjoin *pMirror;
          int unknown4;
          int unknown5;
          float distance;
        };

        struct MatRecord
        {
          char unknown[148];
        };


        struct SithMaterial
        {
          char aName[64];
          int id;
          int colorMode;
          int Width;
          int Height;
          int unknown21;
          int MatRecordCount;
          MatRecord *aRecords;
        };
   */
}
#endif // LIBIM_SURFACE_H
