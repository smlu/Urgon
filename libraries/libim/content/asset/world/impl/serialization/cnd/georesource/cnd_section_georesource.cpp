#include "../cnd.h"
#include "../material/cnd_mat_header.h"
#include "../../world_ser_common.h"
#include "cnd_adjoin.h"
#include "cnd_surface.h"

#include <libim/types/safe_cast.h>
#include <libim/utils/utils.h>
#include <string>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;
using namespace std::string_literals;


std::size_t CND::getOffset_Georesource(const InputStream& istream, const CndHeader& header)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

   const auto matSectionOffset = getOffset_Materials(istream);
   istream.seek(matSectionOffset);

    const uint32_t nPixelDataSize = istream.read<uint32_t>();
    return istream.tell() + nPixelDataSize + header.numMaterials * sizeof(CndMatHeader);
}

Georesource CND::readGeoresource(const InputStream& istream)
{
    auto cndHeader = readHeader(istream);
    istream.seek(getOffset_Georesource(istream, cndHeader));
    return parseSection_Georesource(istream, cndHeader);
}

Georesource CND::parseSection_Georesource(const InputStream& istream, const CndHeader& cndHeader)
{
    try
    {
        Georesource geores;
        geores.vertices    = istream.read<std::vector<Vector3f>>(cndHeader.numVertices);
        geores.texVertices = istream.read<std::vector<Vector2f>>(cndHeader.numTexVertices);
        auto adjoins       = istream.read<std::vector<CndSurfaceAdjoin>>(cndHeader.numAdjoins);

        geores.adjoins.reserve(adjoins.size());
        for (const auto& a : adjoins)
        {
            geores.adjoins.push_back({
                a.flags,
                makeOptionalIdx(a.mirror),
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
            s.matIdx     = makeOptionalIdx(h.materialIdx);
            s.surflags   = h.surfflags;
            s.flags      = h.faceflags;
            s.geoMode    = h.geoMode;
            s.lightMode  = h.lightMode;
            s.adjoinIdx  = makeOptionalIdx(h.adjoinIdx);
            s.extraLight = h.extraLight;
            s.normal     = h.normal;

            s.vertices.resize(h.numVerts);
            s.vecIntensities.reserve(h.numVerts);
            for (auto& v : s.vertices)
            {
                v.vertIdx = safe_cast<decltype(v.vertIdx)>(itVerts->vertIdx);
                v.uvIdx   = makeOptionalIdx(itVerts->uvIdx);
                s.vecIntensities.push_back(std::move(itVerts->color));
                ++itVerts;
            }

            geores.surfaces.push_back(std::move(s));
        }

        world_ser_assert(itVerts == vecSurfVerts.end(),
            "Not all parsed CndSurfaceVerts were used"
        );

        // Note: After reading the cnd structs from file, Jones3d engine initialite SithSurface struct (see cnd_surface.h)
        //       and sets pointers for the adjoin idx and material idx;

        return geores;
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e)
    {
        throw CNDError("parseSection_Georesource",
            "An exception was encountered while parsing secion 'Georesource': "s + e.what()
        );
    }
}

void CND::writeSection_Georesource(OutputStream& ostream, const Georesource& geores)
{
    try
    {
        // Write verteices and tex vertices
        ostream.write(geores.vertices);
        ostream.write(geores.texVertices);

        // Write adjoins
        std::vector<CndSurfaceAdjoin> cadjons;
        cadjons.reserve(geores.adjoins.size());
        for(const auto& a : geores.adjoins)
        {
            cadjons.push_back({
                a.flags,
                fromOptionalIdx(a.mirrorIdx),
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
            h.materialIdx = fromOptionalIdx(s.matIdx);
            h.surfflags   = s.surflags;
            h.faceflags   = s.flags;
            h.geoMode     = s.geoMode;
            h.lightMode   = s.lightMode;
            h.adjoinIdx   = fromOptionalIdx(s.adjoinIdx);
            h.extraLight  = s.extraLight;
            h.numVerts    = s.vertices.size();
            h.normal      = s.normal;
            surfheaders.push_back(std::move(h));

            vecSurfVerts.reserve(s.vertices.size());
            for(auto[idx, v] : cenumerate(s.vertices))
            {
                vecSurfVerts.push_back({
                    safe_cast<decltype(CndSurfaceVerts::vertIdx)>(v.vertIdx),
                    fromOptionalIdx(v.uvIdx),
                    s.vecIntensities.at(idx)
                });
            }
        }

        ostream.write(surfheaders);
        ostream.write<int32_t>(safe_cast<int32_t>(vecSurfVerts.size()));
        ostream.write(vecSurfVerts);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e)
    {
        throw CNDError("writeSection_Georesource",
            "An exception was encountered while writing secion 'Georesource': "s + e.what()
        );
    }
}
