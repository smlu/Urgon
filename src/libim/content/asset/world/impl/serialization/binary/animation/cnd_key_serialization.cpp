#include "cnd_key_structs.h"
#include "../cnd.h"
#include "../material/cnd_mat_header.h"
#include "../../../../../animation/animation.h"
#include "../../../../../../../log/log.h"
#include "../../../../../../../utils/utils.h"

#include <algorithm>
#include <cctype>
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


std::size_t CND::GetAnimSectionOffset(const CndHeader& header, const InputStream& istream)
{
    /* Seek to the beginning of material list */
    istream.seek(GetMatSectionOffset(header));

    /* Calculate material header list size */
    auto nMatHeaderListSize = sizeof(CndMatHeader) * header.numMaterials;

    /* Read materials pixel data size */
    uint32_t nMatBitmapSize = istream.read<uint32_t>();

    /* Seek to the end of material list */
    auto matEndOffs =  istream.tell() + nMatHeaderListSize + nMatBitmapSize;
    istream.seek(matEndOffs);

    /* Find the first key in the list */
    auto ofs = FindFirstKey(istream);
    return (ofs >= 12) ? ofs - 12 : ofs; // move 12 bytes back to the beginning of the keyframes section
}



HashMap<Animation> CND::ReadAnimations(const InputStream& istream)
{
    HashMap<Animation>  animations;

    auto cndHeader = LoadHeader(istream);

    /* Return if no materials are present in file*/
    if(cndHeader.numKeyframes < 1)
    {
        LOG_INFO("CND: No animations found in CND file stream!");
        return animations;
    }

    // Move stream to the beginning of the animation section
    auto sectionOffset = GetAnimSectionOffset(cndHeader, istream);
    if(sectionOffset == 0) {
        throw StreamError("No keyframes section found in CND file stream");
    }

    istream.seek(sectionOffset);
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
