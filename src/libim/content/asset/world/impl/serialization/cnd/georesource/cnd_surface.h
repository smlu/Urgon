#ifndef LIBIM_CND_SURFACE_H
#define LIBIM_CND_SURFACE_H
#include <cstdint>
#include <vector>
#include <tuple>
#include "../../../../surface.h"
#include "../../../../../../../math/color.h"
#include "../../../../../../../math/vector3.h"

namespace libim::content::asset {

    struct CndSurfaceHeader
    {
        int32_t materialIdx; // -1, if no surface material
        Surface::SurfaceFlag surfflags;
        Face::Flag faceflags;
        GeoMode    geoMode;
        LightMode  lightMode;
        int32_t    adjoinIdx;  // -1, if no surface adjoin
        Color      color;      // surface color
        Vector3f   normal;
        uint32_t   numVerts;
    };

    struct CndSurfaceVerts
    {
        int32_t vertIdx;
        int32_t texIdx;
        Color color; // vertices color
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

#endif // LIBIM_CND_SURFACE_H
