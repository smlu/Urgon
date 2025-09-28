#include "cnd_key_structs.h"
#include "../cnd.h"
#include "../material/cnd_mat_header.h"
#include "../../world_ser_common.h"

#include <libim/content/asset/animation/animation.h>
#include <libim/log/log.h>
#include <libim/utils/utils.h>
#include <libim/types/safe_cast.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <string_view>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;
using namespace std::string_literals;


std::size_t CND::getOffset_Keyframes(const InputStream& istream, const CndHeader& header)
{
    return getOffset_Sprites(istream, header) + header.numSprites * sizeof(CndResourceName);
}

UniqueTable<Animation> CND::parseSection_Keyframes(const InputStream& istream, const CndHeader& header)
{
    try
    {
        UniqueTable<Animation>  animations;

        /* Return if no materials are present in file*/
        if(header.numKeyframes < 1)
        {
            LOG_INFO("CND::parseSection_Keyframes(): No animations found!");
            return animations;
        }

        auto aNumEntries = istream.read<std::array<uint32_t, 3>>();

        // Read key header list, marker list, key node list and key node entry list
        auto headerList    = istream.read<std::vector<CndKeyHeader>>(header.numKeyframes);
        auto markerList    = istream.read<std::vector<KeyMarker>>   (aNumEntries.at(0));
        auto nodeList      = istream.read<std::vector<CndKeyNode>>  (aNumEntries.at(1));
        auto nodeEntryList = istream.read<std::vector<KeyNodeEntry>>(aNumEntries.at(2));

        auto mIt  = markerList.cbegin();
        auto nIt  = nodeList.cbegin();
        auto neIt = nodeEntryList.cbegin();
        animations.reserve(header.numKeyframes);

        for(const auto& header : headerList)
        {
            Animation anim;
            anim.setName(utils::trim(header.name));
            anim.setFlags(header.flags);
            anim.setType(header.type);
            anim.setFrames(header.frames);
            anim.setFps(header.fps);
            anim.setJoints(header.numJoints);

            /* Copy key markers */
            std::vector<KeyMarker> markers;
            mIt = utils::copy(mIt, header.numMarkers, markers);
            anim.setMarkers(std::move(markers));

            /* Copy key nodes and it's entries */
            anim.nodes().resize(header.numNodes);
            for(auto& node : anim.nodes())
            {
                node.meshName = utils::trim(nIt->meshName);
                node.num = nIt->nodeNum;
                neIt = utils::copy(neIt, nIt->numEntries, node.entries); // TODO: check bounds for nIt->numEntries
                nIt++;
            }

            animations.pushBack(anim.name(), std::move(anim));
        }

        world_ser_assert(mIt == markerList.end(), "Not all parsed key markers were used");
        world_ser_assert(nIt == nodeList.end(), "Not all parse key nodes were used");
        world_ser_assert(neIt == nodeEntryList.end(), "Not all parse key node entires were used");
        return animations;
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e)
    {
        throw CNDError("parseSection_Keyframes",
            "An exception was encountered while parsing secion 'Keyframes': "s + e.what()
        );
    }
}

UniqueTable<Animation> CND::readKeyframes(const InputStream& istream)
{
    auto cndHeader = readHeader(istream);

    // Move stream to the beginning of the keyframes section
    auto sectionOffset = getOffset_Keyframes(istream, cndHeader);
    if(sectionOffset == 0) {
        throw CNDError("readKeyframes",
            "Section 'Keyframes' not found in CND stream"
        );
    }

    istream.seek(sectionOffset);
    return parseSection_Keyframes(istream, cndHeader);
}

void CND::writeSection_Keyframes(OutputStream& ostream, const UniqueTable<Animation>& animations)
{
    try
    {
        std::vector<CndKeyHeader> cndHeaders;
        cndHeaders.reserve(animations.size());

        std::vector<KeyMarker> markers;
        std::vector<CndKeyNode> nodes;
        std::vector<KeyNodeEntry> entries;
        for(const auto& anim : animations)
        {
            CndKeyHeader h;
            if(!utils::strcpy(h.name, anim.name())) {
                throw CNDError("writeSection_Keyframes",
                    "Too long animation name to copy to CndKeyHeader.name field"
                );
            }

            h.flags      = anim.flags();
            h.type       = anim.type();
            h.frames     = safe_cast<decltype(h.frames)>(anim.frames());
            h.fps        = anim.fps();
            h.numMarkers = safe_cast<decltype(h.numMarkers)>(anim.markers().size());
            h.numJoints  = safe_cast<decltype(h.numJoints)>(anim.joints());
            h.numNodes   = safe_cast<decltype(h.numNodes)>(anim.nodes().size());
            cndHeaders.push_back(std::move(h));

            /* Copy key markers */
            if(anim.markers().size() > kCndMaxKeyMarkers) {
                throw CNDError("writeSection_Keyframes",
                    "Too many key markers."
                );
            }

            markers.reserve(anim.markers().size());
            std::copy(anim.markers().begin(), anim.markers().end(), std::back_inserter(markers));

            /* Copy key nodes and it's entries */
            nodes.reserve(anim.nodes().size());
            for(auto& node : anim.nodes())
            {
                CndKeyNode n;
                if(!utils::strcpy(n.meshName, node.meshName)) {
                    throw CNDError("writeSection_Keyframes",
                        "Too long mesh name to copy to CndKeyNode.name field"
                    );
                }

                n.nodeNum = node.num;
                n.numEntries = safe_cast<decltype(n.numEntries)>(node.entries.size());
                nodes.push_back(std::move(n));

                entries.reserve(node.entries.size());
                std::copy(node.entries.begin(), node.entries.end(), std::back_inserter(entries));
            }
        }

        std::array<uint32_t, 3> aSizeEntries {
            safe_cast<uint32_t>(markers.size()),
            safe_cast<uint32_t>(nodes.size()),
            safe_cast<uint32_t>(entries.size())
        };

        /* Write sizes of key entries */
        ostream.write(aSizeEntries);

        /* Write key headers */
        ostream.write(cndHeaders);

        /* Write key entries*/
        ostream.write(markers);
        ostream.write(nodes);
        ostream.write(entries);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e)
    {
        throw CNDError("writeSection_Keyframes",
            "An exception was encountered while writing secion 'Keyframes': "s + e.what()
        );
    }
}
