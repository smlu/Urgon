#include "cnd_key_structs.h"
#include "../cnd.h"
#include "../material/cnd_mat_header.h"
#include "../../../../../animation/animation.h"
#include "../../../../../../../log/log.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

using namespace libim;
using namespace libim::content::asset;
using namespace std::string_view_literals;

template<std::size_t N>
std::string GetTrimmedName(const char (&str)[N])
{
    std::size_t end = 0;
    while(end++ < N) {
        if(str[end] == '\0') {
            break;
        }
    }

    return std::string(str, end);
}

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

    /* Find the first key in list*/
    return FindFirstKey(istream);
}

std::vector<Animation> CND::ReadAnimations(const InputStream& istream)
{
    std::vector<Animation> animations;

    auto cndHeader = LoadHeader(istream);

    /* Return if no materials are present in file*/
    if(cndHeader.numKeyframes < 1)
    {
        LOG_INFO("CND: No animations found in CND file!");
        return animations;
    }

    // Move stream to the beginning of the animation section
    auto sectionOffset = GetAnimSectionOffset(cndHeader, istream);
    if(sectionOffset == 0) {
        throw StreamError("No animation section found in CND file stream");
    }

    istream.seek(sectionOffset);

    // Read anim header list
    animations.resize(cndHeader.numKeyframes);
    auto headerList = istream.read<std::vector<CndKeyHeader>>(cndHeader.numKeyframes);

    // Read key markers
    for(std::size_t i = 0; i < headerList.size(); i++)
    {
        const auto& header = headerList.at(i);
        auto markers = istream.read<std::vector<KeyMarker>>(header.numMarkers);
        auto& anim = animations.at(i);
        anim.setName(GetTrimmedName(header.name));
        anim.setFlags(header.flags);
        anim.setType(header.type);
        anim.setFrames(header.frames);  // TODO: check bounds
        anim.setFps(header.fps);
        anim.setJoints(header.numJoints); // TODO: check bounds
        anim.setMarkers(std::move(markers));

        anim.nodes().resize(header.numNodes); // TODO: check bounds
    }

    // Read node header list
    for(auto& anim : animations)
    {
        std::vector<CndKeyNode> nodes = istream.read<std::vector<CndKeyNode>>(anim.nodes().size());
        for(std::size_t i = 0; i < anim.nodes().size(); i++)
        {
            auto& node = anim.nodes().at(i);
            node.meshName = GetTrimmedName(nodes.at(i).meshName);
            node.num = nodes.at(i).nodeNum; // TODO: check bounds
            node.entries.resize(nodes.at(i).numEntries); // TODO: check bounds
        }
    }

    // Read keyframes
    for(auto& anim : animations)
    {
        for(auto& node : anim.nodes())
        {
            std::size_t nToRead = sizeof(KeyNodeEntry) * node.entries.size();
            auto nRead = istream.read(reinterpret_cast<byte_t*>(&node.entries[0]), nToRead);
            if(nToRead != nRead) {
                throw StreamError("Could not read keyframe from CND file stream");
            }
        }
    }

    return animations;
}
