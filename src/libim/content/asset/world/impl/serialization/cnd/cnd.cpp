#include "cnd.h"
#include "animation/cnd_key_structs.h"
#include "sector/cnd_sector.h"
#include "../world_ser_common.h"
#include "../../../../../../utils/utils.h"

#include <algorithm>
#include <array>
#include <cstdint>

using namespace libim;
using namespace libim::content::asset;

static constexpr uint32_t FileVersion = 3;

std::vector<std::string> ReadResourceList(const InputStream& istream, std::size_t size)
{
    auto rlist = istream.read<std::vector<ResourceName>>(size);

    std::vector<std::string> res;
    res.reserve(rlist.size());

    std::transform(rlist.begin(), rlist.end(), std::back_insert_iterator(res),
                   [](const auto& aName){ return utils::trim(aName); });

    return res;
}

template<template<typename, typename ...> class List, typename T, typename ...Args, typename Lambda>
void WriteResourceList(OutputStream& ostream, const List<T, Args...>& list, Lambda&& nameExtractor)
{
    std::vector<ResourceName> wlist;
    wlist.reserve(list.size());

    std::transform(list.begin(), list.end(), std::back_insert_iterator(wlist),
    [&](const T& e)
    {
        ResourceName aName;
        if(!utils::strcpy(aName, nameExtractor(e))) {
            throw StreamError("Too long resource name to write to CND stream");
        }
        return aName;
    });

    ostream.write(wlist);
}



CndHeader CND::LoadHeader(const InputStream& istream)
{
    istream.seek(0);
    CndHeader cndHeader = istream.read<CndHeader>();

    /* Verify file copyright notice  */
    if( !std::equal(cndHeader.copyright.begin(), cndHeader.copyright.end(), kFileCopyright.begin())) {
        throw StreamError("Error bad CND file copyright");
    }

    /* Verify file version */
    if(cndHeader.version != FileVersion) {
        throw StreamError("Error wrong CND file version: " + std::to_string(cndHeader.version));
    }

    return cndHeader;
}

std::size_t CND::GetOffset_AiClass(const InputStream& istream, const CndHeader& header)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    istream.seek(
        GetOffset_Sectors(istream, header) +
        sizeof(CndSectorHeader) * header.numSectors
    );

    auto vecBuffSize = istream.read<uint32_t>();
    return istream.tell() + vecBuffSize * sizeof(uint32_t);
}

std::vector<std::string> CND::ParseSection_AiClass(const InputStream& istream, const CndHeader& header)
{
    return ReadResourceList(istream, header.numAiClasses);
}

std::vector<std::string> CND::ReadAiClass(const InputStream& istream)
{
    auto header = LoadHeader(istream);
    istream.seek(GetOffset_AiClass(istream, header));
    return ParseSection_AiClass(istream, header);
}

void CND::WriteSection_AiClass(OutputStream& ostream, const std::vector<std::string>& aiclasses)
{
    WriteResourceList(ostream, aiclasses);
}

std::size_t CND::GetOffset_Models(const InputStream& istream, const CndHeader& header)
{
    return GetOffset_AiClass(istream, header) + header.numAiClasses * sizeof(ResourceName);
}

std::vector<std::string> CND::ParseSection_Models(const InputStream& istream, const CndHeader& header)
{
    return ReadResourceList(istream, header.numModels);
}

std::vector<std::string> CND::ReadModels(const InputStream& istream)
{
    auto header = LoadHeader(istream);
    istream.seek(GetOffset_Models(istream, header));
    return ParseSection_Models(istream, header);
}

void CND::WriteSection_Models(OutputStream& ostream, const std::vector<std::string>& models)
{
    WriteResourceList(ostream, models);
}

std::size_t CND::GetOffset_Sprites(const InputStream& istream, const CndHeader& header)
{
    return GetOffset_Models(istream, header) + header.numModels * sizeof(ResourceName);
}

std::vector<std::string> CND::ParseSection_Sprites(const InputStream& istream, const CndHeader& header)
{
    return ReadResourceList(istream, header.numSprites);
}

std::vector<std::string> CND::ReadSprites(const InputStream& istream)
{
    auto header = LoadHeader(istream);
    istream.seek(GetOffset_Sprites(istream, header));
    return ParseSection_Sprites(istream, header);
}

void CND::WriteSection_Sprites(OutputStream& ostream, const std::vector<std::string>& sprites)
{
    WriteResourceList(ostream, sprites);
}

std::size_t CND::GetOffset_AnimClass(const InputStream& istream, const CndHeader& header)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    istream.seek(GetOffset_Keyframes(istream, header));
    auto aSizes = istream.read<std::array<uint32_t, 3>>();

    return istream.tell() +
           sizeof(CndKeyHeader) * header.numKeyframes +
           sizeof(KeyMarker) * aSizes.at(0) +
           sizeof(CndKeyNode) * aSizes.at(1) +
           sizeof(KeyNodeEntry) * aSizes.at(2);
}

std::vector<std::string> CND::ParseSection_AnimClass(const InputStream& istream, const CndHeader& header)
{
    return ReadResourceList(istream, header.numPuppets);
}

std::vector<std::string> CND::ReadAnimClass(const InputStream& istream)
{
    auto header = LoadHeader(istream);
    istream.seek(GetOffset_AnimClass(istream, header));
    return ParseSection_AnimClass(istream, header);
}

void CND::WriteSection_AnimClass(OutputStream& ostream, const std::vector<std::string>& animclasses)
{
    WriteResourceList(ostream, animclasses);
}

std::size_t CND::GetOffset_SoundClass(const InputStream& istream, const CndHeader& header)
{
    return GetOffset_AnimClass(istream, header) + sizeof(ResourceName) * header.numPuppets;
}

std::vector<std::string> CND::ParseSection_SoundClass(const InputStream& istream, const CndHeader& header)
{
    return ReadResourceList(istream, header.numSoundClasses);
}

std::vector<std::string> CND::ReadSoundClass(const InputStream& istream)
{
    auto header = LoadHeader(istream);
    istream.seek(GetOffset_SoundClass(istream, header));
    return ParseSection_SoundClass(istream, header);
}

void CND::WriteSection_SoundClass(OutputStream& ostream, const std::vector<std::string>& sndclasses)
{
    WriteResourceList(ostream, sndclasses);
}

std::size_t CND::GetOffset_CogScripts(const InputStream& istream, const CndHeader& header)
{
    return GetOffset_SoundClass(istream, header) + sizeof(ResourceName) * header.numSoundClasses;
}

std::vector<std::string> CND::ParseSection_CogScripts(const InputStream& istream, const CndHeader& header)
{
    return ReadResourceList(istream, header.numCogScripts);
}

std::vector<std::string> CND::ReadCogScripts(const InputStream& istream)
{
    auto header = LoadHeader(istream);
    istream.seek(GetOffset_CogScripts(istream, header));
    return ParseSection_CogScripts(istream, header);
}

void CND::WriteSection_CogScripts(OutputStream& ostream, const std::vector<std::string>& scripts)
{
    WriteResourceList(ostream, scripts);
}


