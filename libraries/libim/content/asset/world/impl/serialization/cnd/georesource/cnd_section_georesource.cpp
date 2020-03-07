#include "../cnd.h"
#include "../material/cnd_mat_header.h"

#include "cnd_adjoin.h"
#include "cnd_surface.h"
#include "../../world_ser_common.h"
#include <libim/types/safe_cast.h>
#include <libim/utils/utils.h>



using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;


std::size_t CND::GetOffset_Georesource(const CndHeader& header, const InputStream& istream)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

   const auto matSectionOffset = GetOffset_Materials(istream);
   istream.seek(matSectionOffset);

    const uint32_t nPixelDataSize = istream.read<uint32_t>();
    return istream.tell() + nPixelDataSize + header.numMaterials * sizeof(CndMatHeader);
}

Georesource CND::ReadGeoresource(const InputStream& istream)
{
    auto cndHeader = ReadHeader(istream);
    istream.seek(GetOffset_Georesource(cndHeader, istream));
    return ParseSection_Georesource(cndHeader, istream);
}

Georesource CND::ParseSection_Georesource(const CndHeader& cndHeader, const InputStream& istream)
{
    Georesource geores;
    geores.verts    = istream.read<std::vector<Vector3f>>(cndHeader.numVertices);
    geores.texVerts = istream.read<std::vector<Vector2f>>(cndHeader.numTexVertices);
    auto adjoints   = istream.read<std::vector<CndSurfaceAdjoin>>(cndHeader.numAdjoins);

    geores.adjoints.reserve(adjoints.size());
    for (const auto& a : adjoints)
    {
        geores.adjoints.push_back({
            a.flags,
            make_optional_idx(a.mirror),
            std::nullopt,
            std::nullopt,
            a.distance
        });
    }

    /* Note: After reading adjoin list, jones3d goes over every cnd adjoin and initializes the list of SurfaceAdjoint structs.
            Besides fields flags and distance it sets a pointer in the SurfaceAdjoint instead of mirror number.
            If mirror number is -1, nullptr is set instead.

            struct SurfaceAdjoin
            {
                int flags;
                int unknown1;
                int unknown2;
                SurfaceAdjoin* pMirror;
                int unknown4;
                int unknown5;
                float distance;
            };

            if ( pWorld->numAdjoins )
            {
                adjoinIdx = 0;
                CndSurfaceAdjoin* pCndAdjoin= &adjoinList[0];
                do
                {
                    SurfaceAdjoin* pAdjoin = &pWorld->aAdjoins[adjoinIdx];
                    pAdjoin->flags = pCndAdjoin->flags;
                    if ( pCndAdjoin->mirror == -1 )
                    {
                        pAdjoin->pAMirror = nullptr;
                    }
                    else
                    {
                        pAdjoin->pMirror = &pWorld->aAdjoins[pCndAdjoin->mirror];
                    }

                    pAdjoin->distance = pCndAdjoin->distance;
                    ++adjoinIdx;
                    ++pCndAdjoin;
                }
                while ( adjoinIdx < pWorld->numAdjoins );
            }
    */



     auto vecSurfHeaders = istream.read<std::vector<CndSurfaceHeader>>(cndHeader.numSurfaces);
     auto numVertsBuff   = istream.read<uint32_t>();
     auto vecSurfVerts   = istream.read<std::vector<CndSurfaceVerts>>(numVertsBuff);

     auto itVerts = vecSurfVerts.begin();
     geores.surfaces.reserve(cndHeader.numSurfaces);
     for(const auto& h : vecSurfHeaders)
     {
         Surface s;
         s.id         = std::size(geores.surfaces);
         s.matIdx     = make_optional_idx(h.materialIdx);
         s.surflags   = h.surfflags;
         s.flags      = h.faceflags;
         s.geoMode    = h.geoMode;
         s.lightMode  = h.lightMode;
         s.adjoinIdx  = make_optional_idx(h.adjoinIdx);
         s.extraLight = h.extraLight;
         s.normal     = h.normal;

         s.verts.resize(h.numVerts);
         s.vecIntensities.reserve(h.numVerts);
         for (auto& v : s.verts)
         {
             v.vertIdx   = safe_cast<decltype(v.vertIdx)>(itVerts->vertIdx);
             v.texIdx    = make_optional_idx(itVerts->texIdx);
             s.vecIntensities.push_back(std::move(itVerts->color));
             ++itVerts;
         }

         geores.surfaces.push_back(std::move(s));
     }

     // Note: After reading the cnd structs from file, Jones3d engine initialite SithSurface struct (see cnd_surface.h)
     //       and sets pointers for the adjoin idx and material idx;

     return geores;
}


void CND::WriteSection_Georesource(OutputStream& ostream, const Georesource& geores)
{
    // Write verteices and tex vertices
    ostream.write(geores.verts);
    ostream.write(geores.texVerts);

    // Write adjoins
    std::vector<CndSurfaceAdjoin> cadjons;
    cadjons.reserve(geores.adjoints.size());
    for(const auto& a : geores.adjoints)
    {
        cadjons.push_back({
            a.flags,
            from_optional_idx(a.mirrorIdx),
            a.distance,
        });
    }

    ostream.write(cadjons);

    // Write surfaces
    std::vector<CndSurfaceHeader> surfheaders;
    surfheaders.reserve(geores.surfaces.size());
    std::vector<CndSurfaceVerts> vecSurfVerts;
    for(const auto& s : geores.surfaces)
    {
        CndSurfaceHeader h;
        h.materialIdx = from_optional_idx(s.matIdx);
        h.surfflags   = s.surflags;
        h.faceflags   = s.flags;
        h.geoMode     = s.geoMode;
        h.lightMode   = s.lightMode;
        h.adjoinIdx   = from_optional_idx(s.adjoinIdx);
        h.extraLight  = s.extraLight;
        h.numVerts    = s.verts.size();
        h.normal      = s.normal;
        surfheaders.push_back(std::move(h));

        vecSurfVerts.reserve(s.verts.size());
        for(auto[idx, v] : cenumerate(s.verts))
        {
            vecSurfVerts.push_back({
                safe_cast<decltype(CndSurfaceVerts::vertIdx)>(v.vertIdx),
                from_optional_idx(v.texIdx),
                s.vecIntensities.at(idx)
            });
        }
    }

    ostream.write(surfheaders);
    ostream.write<int32_t>(safe_cast<int32_t>(vecSurfVerts.size()));
    ostream.write(vecSurfVerts);
}


