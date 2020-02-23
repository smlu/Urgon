#ifndef LIBIM_SURFACE_H
#define LIBIM_SURFACE_H
#include <cstdint>
#include <string>

#include "../primitives/face.h"

namespace libim::content::asset {

    struct Surface final : public Face
    {
        enum SurfaceFlag : uint32_t
        {                           // 0x0 - can got through the surface
            Floor                    = 0x1,
            CogLinked                = 0x2,
            Impassable               = 0x4,        // Maybe unable for thing(player) to walk on this surface.
            AiCannotWalkOnFloor      = 0x8,        // Ai won't walk on this surface
            DoubletextureScale       = 0x10,
            HalftextureScale         = 0x20,
            EighthtextureScale       = 0x40,
            NoFallingDamage          = 0x80,
            HorizonSky               = 0x200,
            CeilingSky               = 0x400,
            Scrolling                = 0x800,
            KillFloor                = 0x1000,     // Player dies if it touches the floor. In cyn_killfloor.cog
            Climbable                = 0x2000,
            MineCarTrack             = 0x4000,     // Mine car track: sol_ramp.cog, sol_ctrllever5.cog, sol_ctrllever4.cog, sol_ctrllever3.cog, sol_ctrllever2.cog, sol_ctrllever1.cog, sol_comfalls.cog, sol_comentrance.cog
            Metal                    = 0x10000,
            DeepWater                = 0x20000,
            ShallowWater             = 0x40000,
            Dirt                     = 0x80000,
            Web                      = 0x100000,   // Jones engine
            Lava                     = 0x200000,
            Unknown_400000           = 0x400000,
            Hangable                 = 0x1000000,  // Player can hang on the ledge of this surface. In: tem_bossflood.cog, vol_block_ledgecontrol.cog
            WaterClimbOutLedge       = 0x2000000,
            UnknownCouldBeWhipSurf   = 0x10000000, // vol_lekk_door.cog
            Wood                     = 0x40800000,
            Vine                     = 0x80480000
            //missing flag for stone
        };
        
        using Id = std::size_t;
        
        Id id;
        SurfaceFlag surflags;
        std::optional<std::size_t> adjoinIdx;
        std::vector<Color> vecIntensities;     // verticies color. Color of each vertex is applied over surface's texture and
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
