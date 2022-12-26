#include "../cnd.h"
#include "../georesource/cnd_adjoin.h"
#include "../georesource/cnd_surface.h"
#include "cnd_sector.h"

#include <libim/types/safe_cast.h>
#include <libim/utils/utils.h>
#include <string>

using namespace libim;
using namespace libim::content::asset;
using namespace std::string_literals;


std::size_t CND::getOffset_Sectors(const InputStream& istream, const CndHeader& header)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    istream.seek(
        getOffset_Georesource(istream, header)           +
        sizeof(Vector3f)         * header.numVertices    +
        sizeof(Vector2f)         * header.numTexVertices +
        sizeof(CndSurfaceAdjoin) * header.numAdjoins     +
        sizeof(CndSurfaceHeader) * header.numSurfaces
    );

    auto numVertsBuff   = istream.read<uint32_t>();
    auto offs = istream.tell() + sizeof(CndSurfaceVerts) * numVertsBuff;
    return offs;
}

std::vector<Sector> CND::parseSection_Sectors(const InputStream& istream, const CndHeader& header)
{
    try
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
            s.pvsIdx = h.pvsIdx;

            s.ambientLight    = h.ambientLight;   //TODO: Alpha is probably 0, set it to 1.f
            s.extraLight      = h.extraLight;     //TODO: Alpha is probably 0, set it to 1.f
            s.avgLight        = h.avgLight;
            // s.avgLightPos     = h.avgLightPos;
            // s.avgLightInt     = h.avgLightInt;    //TODO: Alpha is probably 0, set it to 1.f
            // s.avgLightFalloff = h.avgLightFalloff;

            s.boundBox   = h.boundBox;
            s.collideBox = h.collideBox;

            if( h.ambientSound.at(0) )
            {
                s.ambientSound = {
                    utils::trim(h.ambientSound),  // TODO: get actual sound object ref from content manager
                    h.ambientSoundVolume
                };
            }

            s.surfaces.firstIdx = safe_cast<decltype(s.surfaces.firstIdx)>(h.firstSurfaceIdx);
            s.surfaces.count    = safe_cast<decltype(s.surfaces.count)>(h.surfacesCount);

            // Copy vertex idxs
            s.vertIdxs.resize(safe_cast<std::size_t>(h.verticesCount));
            auto vidxItEnd = std::next(vidxIt, h.verticesCount);
            std::copy(vidxIt, vidxItEnd, s.vertIdxs.begin());
            vidxIt = vidxItEnd;

            sectors.push_back(std::move(s));
        }


        if(vertIdxs.end() != vidxIt){
            throw CNDError("parseSection_Sectors", "Not all vertex idxs were copied from the buffer");
        }

        return sectors;
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_Sectors",
            "An exception was encountered while parsing section 'Sectors': "s + e.what()
        );
    }
}

void CND::writeSection_Sectors(OutputStream& ostream, const std::vector<Sector>& sectors)
{
    try
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
            h.avgLight     = s.avgLight;
            // h.avgLightPos     = s.avgLightPos;
            // h.avgLightInt     = s.avgLightInt;
            // h.avgLightFalloff = s.avgLightFalloff;

            h.collideBox = s.collideBox;
            h.boundBox   = s.boundBox;

            if(s.ambientSound)
            {
                if(!utils::strcpy(h.ambientSound, s.ambientSound->sound)) {
                    throw CNDError("writeSection_Sectors",
                        "Too long sound name to copy it to CndSectorHeader.ambientSound field"
                    );
                }

                h.ambientSoundVolume = s.ambientSound->volume;
            }

            h.firstSurfaceIdx = safe_cast<decltype(h.firstSurfaceIdx)>(s.surfaces.firstIdx);
            h.surfacesCount   = safe_cast<decltype(h.surfacesCount)>(s.surfaces.count);
            h.verticesCount   = safe_cast<decltype(h.verticesCount)>(s.vertIdxs.size());

            h.pvsIdx = s.pvsIdx;
            h.thrust = s.thrust;

            if(vertIdxs.capacity() < s.vertIdxs.size()) {
                vertIdxs.reserve(1000);
            }
            std::copy(s.vertIdxs.begin(), s.vertIdxs.end(), std::back_inserter(vertIdxs));

            cndsectors.push_back(std::move(h));
        }

        ostream.write(cndsectors);
        ostream.write<uint32_t>(safe_cast<uint32_t>(vertIdxs.size()));
        ostream.write(vertIdxs);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_Sectors",
            "An exception was encountered while writing section 'Sectors': "s + e.what()
        );
    }
}

std::vector<Sector> CND::readSectors(const InputStream& istream)
{
    auto cndHeader = readHeader(istream);
    istream.seek(getOffset_Sectors(istream, cndHeader));
    return parseSection_Sectors(istream, cndHeader);
}
