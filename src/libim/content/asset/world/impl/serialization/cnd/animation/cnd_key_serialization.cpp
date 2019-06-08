#include "cnd_key_structs.h"
#include "../cnd.h"
#include "../material/cnd_mat_header.h"
#include "../../../../../animation/animation.h"
#include "../../../../../../../log/log.h"
#include "../../../../../../../utils/utils.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <string_view>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;
using namespace std::string_view_literals;


std::size_t FindFirstKey(const InputStream& istream)
{
    std::size_t offs = 0;

    // Find first occurrence of string '.key'
    while(!istream.atEnd())
    {
        std::size_t nRead = 2048;
        if(istream.tell() + nRead >= istream.size()) {
            nRead = istream.size() - istream.tell();
        }

        auto bytes = istream.read<std::vector<char>>(nRead);
        std::string_view bytesView(&bytes[0], bytes.size());

        constexpr auto key = ".key"sv;
        auto nPos = bytesView.find(key);
        if(nPos != std::string::npos) // Found '.key'?
        {
            offs = istream.tell() - (nRead - nPos);
            break;
        }

        // Move ofs key len back.
        istream.seek(istream.tell() - 4);
    }

    if(offs != 0)
    {
        // Move backwards untill
        // non-printable char is found
        char c = '0';
        do
        {
            offs--;
            istream.seek(offs);
            c = istream.read<char>();
        }
        while(std::isprint(c));
        offs++; // move offs to first printable char
    }

    return offs;
}


std::size_t CND::GetKeySectionOffset(const CndHeader& header, const InputStream& istream)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    /* Seek to the beginning of material list */
    istream.seek(GetMatSectionOffset(istream));

    /* Calculate material header list size */
    std::size_t nMatHeaderListSize = sizeof(CndMatHeader) * header.numMaterials;

    /* Read materials pixel data size */
    std::size_t nMatBitmapSize = istream.read<uint32_t>();

    /* Seek to the end of material list */
    std::size_t matEndOffs =  istream.tell() + nMatHeaderListSize + nMatBitmapSize;
    istream.seek(matEndOffs);

    /* Find the first key in the list */
    auto ofs = FindFirstKey(istream);
    return (ofs >= 12) ? ofs - 12 : ofs; // move 12 bytes back to the beginning of the keyframes section
}

HashMap<Animation> CND::ParseSectionKeyframes(const CndHeader& cndHeader, const InputStream& istream)
{
    HashMap<Animation>  animations;

    /* Return if no materials are present in file*/
    if(cndHeader.numKeyframes < 1)
    {
        LOG_INFO("CND: No animations found in CND file stream!");
        return animations;
    }

    auto aNumEntries = istream.read<std::array<uint32_t, 3>>();

    // Read key header list, marker list, key node list and key node entry list
    auto headerList = istream.read<std::vector<CndKeyHeader>>(cndHeader.numKeyframes);
    auto markerList = istream.read<std::vector<KeyMarker>>(aNumEntries.at(0));
    auto nodeList   = istream.read<std::vector<CndKeyNode>>(aNumEntries.at(1));
    auto nodeEntryList = istream.read<std::vector<KeyNodeEntry>>(aNumEntries.at(2));

    auto mIt  = markerList.begin();
    auto nIt  = nodeList.begin();
    auto neIt = nodeEntryList.begin();
    animations.reserve(cndHeader.numKeyframes);

    for(const auto& header : headerList)
    {
        Animation anim;
        anim.setName(utils::trim(header.name));
        anim.setFlags(header.flags);
        anim.setType(header.type);
        anim.setFrames(header.frames);  // TODO: check bounds
        anim.setFps(header.fps);
        anim.setJoints(header.numJoints); // TODO: check bounds

        /* Copy key markers */
        std::vector<KeyMarker> markers;
        mIt = utils::copy(mIt, header.numMarkers, markers);
        anim.setMarkers(std::move(markers));

        /* Copy key nodes and it's entries */
        anim.nodes().resize(header.numNodes); // TODO: check bounds
        for(auto& node : anim.nodes())
        {
            node.meshName = utils::trim(nIt->meshName);
            node.num = nIt->nodeNum;
            neIt = utils::copy(neIt, nIt->numEntries, node.entries); // TODO: check bounds for nIt->numEntries
            nIt++;
        }

        animations.pushBack(anim.name(), std::move(anim));
    }

    assert(mIt == markerList.end());
    assert(nIt == nodeList.end());
    assert(neIt == nodeEntryList.end());

    return animations;
}

HashMap<Animation> CND::ReadAnimations(const InputStream& istream)
{
    auto cndHeader = LoadHeader(istream);

    // Move stream to the beginning of the keyframes section
    auto sectionOffset = GetKeySectionOffset(cndHeader, istream);
    if(sectionOffset == 0) {
        throw StreamError("No keyframes section found in CND file stream");
    }

    istream.seek(sectionOffset);
    return ParseSectionKeyframes(cndHeader, istream);
}

void CND::WriteSectionKeyframes(OutputStream& ostream, const utils::HashMap<Animation>& animations)
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
            throw StreamError("Too long animation name to copy to CndKeyHeader.name field!");
        }

        h.flags      = anim.flages();
        h.type       = anim.type();
        h.frames     = static_cast<uint32_t>(anim.frames());         // TODO: check bounds
        h.fps        = anim.fps();
        h.numMarkers = static_cast<uint32_t>(anim.markers().size()); // TODO: check bounds
        h.numJoints  = static_cast<uint32_t>(anim.joints());         // TODO: check bounds
        h.numNodes   = static_cast<uint32_t>(anim.nodes().size());   // TODO: check bounds
        cndHeaders.push_back(std::move(h));

        /* Copy key markers */
        if(anim.markers().size() > kCndMaxKeyMarkers) {
            throw StreamError("Num key markers > 16!");
        }

        markers.reserve(anim.markers().size());
        std::copy(anim.markers().begin(), anim.markers().end(), std::back_inserter(markers));

        /* Copy key nodes and it's entries */
        nodes.reserve(anim.nodes().size());
        for(auto& node : anim.nodes())
        {
            CndKeyNode n;
            if(!utils::strcpy(n.meshName, node.meshName)) {
                throw StreamError("Too long mesh name to copy to CndKeyNode.name field!");
            }

            n.nodeNum = node.num;
            n.numEntries = static_cast<uint32_t>(node.entries.size()); // TODO: check bounds
            nodes.push_back(std::move(n));

            entries.reserve(node.entries.size());
            std::copy(node.entries.begin(), node.entries.end(), std::back_inserter(entries));
        }
    }

    std::array<uint32_t, 3> aSizeEntries {
        static_cast<uint32_t>(markers.size()),  // TODO: check bounds
        static_cast<uint32_t>(nodes.size()),    // TODO: check bounds
        static_cast<uint32_t>(entries.size())   // TODO: check bounds
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
