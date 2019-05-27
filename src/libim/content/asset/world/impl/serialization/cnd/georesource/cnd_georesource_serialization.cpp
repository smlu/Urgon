#include "../cnd.h"
#include "../material/cnd_mat_header.h"
#include "cnd_adjoin.h"
#include "cnd_georesource.h"
#include "cnd_surface.h"

using namespace libim;
using namespace libim::content::asset;


std::size_t CND::GetOffset_Georesource(const CndHeader& header, const InputStream& istream)
{
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
    geores.adjoints = istream.read<std::vector<CndSurfaceAdjoin>>(cndHeader.numAdjoins);

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
     for(auto& h : vecSurfHeaders)
     {
         CndSurface s{h};
         s.verts.resize(s.numVerts);
         for(auto& v : s.verts)
         {
             std::get<0>(v) = itVerts->vertIdx;
             std::get<1>(v) = itVerts->texIdx;
             std::get<2>(v) = std::move(itVerts->color);
             ++itVerts;
         }

         geores.surfaces.push_back(std::move(s));
     }

     // Note: After reading the cnd structs from file, Jones3d engine initialite SithSurface struct (see cnd_surface.h)
     //       and sets pointers for the adjoin idx and material idx;

     return geores;
}



