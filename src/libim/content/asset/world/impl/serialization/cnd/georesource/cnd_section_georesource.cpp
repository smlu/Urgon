#include "../cnd.h"
#include "../material/cnd_mat_header.h"

#include "cnd_adjoin.h"
#include "cnd_surface.h"
#include "../../world_ser_common.h"



using namespace libim;
using namespace libim::content::asset;


std::size_t CND::GetOffset_Georesource(const CndHeader& header, const InputStream& istream)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

   const auto matSectionOffset = GetMatSectionOffset(istream);
   istream.seek(matSectionOffset);

    const uint32_t nPixelDataSize = istream.read<uint32_t>();
    return istream.tell() + nPixelDataSize + header.numMaterials * sizeof(CndMatHeader);
}

Georesource CND::ReadGeoresource(const InputStream& istream)
{
    auto cndHeader = LoadHeader(istream);
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
         s.matIdx    = make_optional_idx(h.materialIdx);
         s.surflags  = h.surfflags;
         s.flags     = h.faceflags;
         s.geoMode   = h.geoMode;
         s.lightMode = h.lightMode;
         s.adjoinIdx = make_optional_idx(h.adjoinIdx);
         s.color     = h.color;
         s.normal    = h.normal;

         s.verts.resize(h.numVerts);
         for(auto& v : s.verts)
         {
             v.vertIdx   = itVerts->vertIdx; // TODO: safe cast
             v.texIdx    = make_optional_idx(itVerts->texIdx);
             v.intensity = std::move(itVerts->color);
             ++itVerts;
         }

         geores.surfaces.push_back(std::move(s));
     }

     // Note: After reading the cnd structs from file, Jones3d engine initialite SithSurface struct (see cnd_surface.h)
     //       and sets pointers for the adjoin idx and material idx;

     return geores;
}



