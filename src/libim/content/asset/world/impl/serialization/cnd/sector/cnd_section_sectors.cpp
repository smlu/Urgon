#include "../cnd.h"
#include "cnd_sector.h"
#include "../georesource/cnd_adjoin.h"
#include "../georesource/cnd_surface.h"
#include "../../../../../../../utils/utils.h"

using namespace libim;
using namespace libim::content::asset;

std::size_t CND::GetOffset_Sectors(const InputStream& istream, const CndHeader& header)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    istream.seek(
        GetOffset_Georesource(header, istream) +
        sizeof(Vector3f) * header.numVertices +
        sizeof(Vector2f) * header.numTexVertices +
        sizeof(CndSurfaceAdjoin) * header.numAdjoins +
        sizeof(CndSurfaceHeader) * header.numSurfaces
    );

    auto numVertsBuff   = istream.read<uint32_t>();
    auto offs = istream.tell() + sizeof(CndSurfaceVerts) * numVertsBuff;
    return offs;
}

std::vector<Sector> CND::ParseSection_Sectors(const InputStream& istream, const CndHeader& header)
{
    auto headers = istream.read<std::vector<CndSectorHeader>>(header.numSectors);
    std::size_t numVertIdxs = istream.read<uint32_t>();
    auto vertIdxs = istream.read<std::vector<uint32_t>>(numVertIdxs);

    // Construct result
    std::vector<Sector> sectors;
    sectors.reserve(headers.size());
    auto vidxIt = vertIdxs.begin();

    for(const auto& h : headers)
    {
        Sector s;
        s.id    = std::size(sectors);
        s.flags = h.flags;
        s.tint  = h.tint;

        s.center = h.center;
        s.radius = h.radius;
        s.thrust = h.thrust;

        s.ambientLight    = h.ambientLight;   //TODO: Alpha is probably 0, set it to 1.f
        s.extraLight      = h.extraLight;     //TODO: Alpha is probably 0, set it to 1.f
        s.avgLightPos     = h.avgLightPos;
        s.avgLightInt     = h.avgLightInt;    //TODO: Alpha is probably 0, set it to 1.f
        s.avgLightFalloff = h.avgLightFalloff;

        s.boundBox   = h.boundBox;
        s.collideBox = h.collideBox;

        if( h.ambientSound.at(0) )
        {
            s.ambientSound = {
                utils::trim(h.ambientSound),  // TODO: get actual sound object ref from content manager
                h.ambientSoundVolume
            };
        }

        s.surfaces.firstIdx = h.firstSurfaceIdx; //TODO: safecast
        s.surfaces.count    = h.surfacesCount;   //TODO: safecast

        // Copy vertex idxs
        s.vertIdxs.resize(h.verticesCount); //TODO: safecast
        auto vidxItEnd = std::next(vidxIt, h.verticesCount);
        std::copy(vidxIt, vidxItEnd, s.vertIdxs.begin());
        vidxIt = vidxItEnd;

        sectors.push_back(std::move(s));
    }


    if(vertIdxs.end() != vidxIt)
    {
        LOG_WARNING("CND::ParseSection_Sectors(): Not all vertex idxs were copied from the buffer!");
        assert(false);
    }

    return sectors;
}

void CND::WriteSection_Sectors(OutputStream& ostream, const std::vector<Sector>& sectors)
{
    std::vector<CndSectorHeader> cndsectors;
    cndsectors.reserve(sectors.size());

    auto vertIdxs = std::vector<uint32_t>();
    vertIdxs.reserve(1000);

    for(const auto& s : sectors)
    {
        CndSectorHeader h;
        h.flags        = s.flags;
        h.tint         = s.tint;

        h.center = s.center;
        h.radius = s.radius;

        h.ambientLight = s.ambientLight;
        h.extraLight   = s.extraLight;
        h.avgLightPos     = s.avgLightPos;
        h.avgLightInt     = s.avgLightInt;
        h.avgLightFalloff = s.avgLightFalloff;

        h.collideBox = s.collideBox;
        h.boundBox   = s.boundBox;

        if(s.ambientSound)
        {
            if(!utils::strcpy(h.ambientSound, s.ambientSound->sound)) {
                throw StreamError("Too long sound name to copy to CndSectorHeader.ambientSound field");
            }

            h.ambientSoundVolume = s.ambientSound->volume;
        }

        h.firstSurfaceIdx = s.surfaces.firstIdx; // TODO: safecast
        h.surfacesCount   = s.surfaces.count;    // TODO: safecast
        h.verticesCount   = s.vertIdxs.size();   // TODO: safecast
        h.thrust = s.thrust;

        if(vertIdxs.capacity() < s.vertIdxs.size()) {
            vertIdxs.reserve(1000);
        }
        std::copy(s.vertIdxs.begin(), s.vertIdxs.end(), std::back_inserter(vertIdxs));

        cndsectors.push_back(std::move(h));
    }

    ostream.write(cndsectors);
    ostream.write<uint32_t>(vertIdxs.size()); // TODO: safecast
    ostream.write(vertIdxs);
}

std::vector<Sector> CND::ReadSectors(const InputStream& istream)
{
    auto cndHeader = LoadHeader(istream);
    istream.seek(GetOffset_Sectors(istream, cndHeader));
    return ParseSection_Sectors(istream, cndHeader);
}
