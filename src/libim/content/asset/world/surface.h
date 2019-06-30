#ifndef LIBIM_SURFACE_H
#define LIBIM_SURFACE_H
#include <cstdint>
#include <string>

#include "../primitives/face.h"

namespace libim::content::asset {

    struct Surface final : public Face
    {
        enum SurfaceFlag : uint32_t
        {
            Floor                    = 0x1,
            CogLinked                = 0x2,
            Impassable               = 0x4,
            AiCannotWalkOnFloor      = 0x8,
            DoubletextureScale       = 0x10,
            HalftextureScale         = 0x20,
            EighthtextureScale       = 0x40,
            NoFallingDamage          = 0x80,
            HorizonSky               = 0x200,
            CeilingSky               = 0x400,
            Scrolling                = 0x800,
            Icy                      = 0x1000,
            VeryIcy                  = 0x2000,
            MagSealed                = 0x4000,
            Metal                    = 0x10000,
            DeepWater                = 0x20000,
            ShallowWater             = 0x40000,
            Dirt                     = 0x80000,
            VeryDeepWater            = 0x100000
        };
        using Id = std::size_t;
        Id id;
        SurfaceFlag surflags;
        std::optional<std::size_t> adjoinIdx;
        std::vector<Color> vecIntensities;
    };


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
