#include "cnd.h"
#include "animation/cnd_key_structs.h"
#include "sector/cnd_sector.h"
#include "../world_ser_common.h"
#include "../../../../../../utils/utils.h"
#include <libim/types/safe_cast.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <variant>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;

static constexpr uint32_t kFileVersion = 3;

std::vector<std::string> ReadResourceList(const InputStream& istream, std::size_t size)
{
    auto rlist = istream.read<std::vector<CndResourceName>>(size);

    std::vector<std::string> res;
    res.reserve(rlist.size());

    std::transform(rlist.begin(), rlist.end(), std::back_insert_iterator(res),
        [](const auto& aName){ return utils::trim(aName); }
    );

    return res;
}

template<template<typename, typename ...> class List, typename T, typename ...Args, typename Lambda>
void WriteResourceList(OutputStream& ostream, const List<T, Args...>& list, Lambda&& nameExtractor)
{
    std::vector<CndResourceName> wlist;
    wlist.reserve(list.size());

    std::transform(list.begin(), list.end(), std::back_insert_iterator(wlist),
    [&](const T& e)
    {
        CndResourceName aName;
        if(!utils::strcpy(aName, nameExtractor(e))) {
            throw StreamError("Too long resource name to write to CND stream");
        }
        return aName;
    });

    ostream.write(wlist);
}

template<template<typename, typename ...> class List, typename ...Args>
void WriteResourceList(OutputStream& ostream, const List<std::string, Args...>& list)
{
     WriteResourceList(ostream, list, [](const std::string& e){ return e; });
}



CndHeader CND::ReadHeader(const InputStream& istream)
{
    istream.seekBegin();
    CndHeader cndHeader = istream.read<CndHeader>();

    /* Verify file copyright notice  */
    if (!std::equal(cndHeader.copyright.begin(), cndHeader.copyright.end(), kFileCopyright.begin())) {
        throw StreamError("Error bad CND file copyright");
    }

    /* Verify file version */
    if (cndHeader.version != kFileVersion) {
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
    auto header = ReadHeader(istream);
    istream.seek(GetOffset_AiClass(istream, header));
    return ParseSection_AiClass(istream, header);
}

void CND::WriteSection_AiClass(OutputStream& ostream, const std::vector<std::string>& aiclasses)
{
    WriteResourceList(ostream, aiclasses);
}

std::size_t CND::GetOffset_Models(const InputStream& istream, const CndHeader& header)
{
    return GetOffset_AiClass(istream, header) + header.numAiClasses * sizeof(CndResourceName);
}

std::vector<std::string> CND::ParseSection_Models(const InputStream& istream, const CndHeader& header)
{
    return ReadResourceList(istream, header.numModels);
}

std::vector<std::string> CND::ReadModels(const InputStream& istream)
{
    auto header = ReadHeader(istream);
    istream.seek(GetOffset_Models(istream, header));
    return ParseSection_Models(istream, header);
}

void CND::WriteSection_Models(OutputStream& ostream, const std::vector<std::string>& models)
{
    WriteResourceList(ostream, models);
}

std::size_t CND::GetOffset_Sprites(const InputStream& istream, const CndHeader& header)
{
    return GetOffset_Models(istream, header) + header.numModels * sizeof(CndResourceName);
}

std::vector<std::string> CND::ParseSection_Sprites(const InputStream& istream, const CndHeader& header)
{
    return ReadResourceList(istream, header.numSprites);
}

std::vector<std::string> CND::ReadSprites(const InputStream& istream)
{
    auto header = ReadHeader(istream);
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
    auto header = ReadHeader(istream);
    istream.seek(GetOffset_AnimClass(istream, header));
    return ParseSection_AnimClass(istream, header);
}

void CND::WriteSection_AnimClass(OutputStream& ostream, const std::vector<std::string>& animclasses)
{
    WriteResourceList(ostream, animclasses);
}

std::size_t CND::GetOffset_SoundClass(const InputStream& istream, const CndHeader& header)
{
    return GetOffset_AnimClass(istream, header) + sizeof(CndResourceName) * header.numPuppets;
}

std::vector<std::string> CND::ParseSection_SoundClass(const InputStream& istream, const CndHeader& header)
{
    return ReadResourceList(istream, header.numSoundClasses);
}

std::vector<std::string> CND::ReadSoundClass(const InputStream& istream)
{
    auto header = ReadHeader(istream);
    istream.seek(GetOffset_SoundClass(istream, header));
    return ParseSection_SoundClass(istream, header);
}

void CND::WriteSection_SoundClass(OutputStream& ostream, const std::vector<std::string>& sndclasses)
{
    WriteResourceList(ostream, sndclasses);
}

std::size_t CND::GetOffset_CogScripts(const InputStream& istream, const CndHeader& header)
{
    return GetOffset_SoundClass(istream, header) + sizeof(CndResourceName) * header.numSoundClasses;
}

std::vector<std::string> CND::ParseSection_CogScripts(const InputStream& istream, const CndHeader& header)
{
    return ReadResourceList(istream, header.numCogScripts);
}

std::vector<std::string> CND::ReadCogScripts(const InputStream& istream)
{
    auto header = ReadHeader(istream);
    istream.seek(GetOffset_CogScripts(istream, header));
    return ParseSection_CogScripts(istream, header);
}

void CND::WriteSection_CogScripts(OutputStream& ostream, const std::vector<std::string>& scripts)
{
    WriteResourceList(ostream, scripts);
}


std::size_t CND::GetOffset_Cogs(const InputStream& istream, const CndHeader& header)
{
    return GetOffset_CogScripts(istream, header) + sizeof(CndResourceName) * header.numCogScripts;
}

std::vector<SharedRef<Cog>> CND::ParseSection_Cogs(const InputStream& istream, const CndHeader& header, const HashMap<SharedRef<CogScript>>& scripts)
{
    auto aSizes = istream.read<std::array<uint32_t, 2>>();
    auto snames = ReadResourceList(istream, aSizes.at(0));
    auto values = ReadResourceList(istream, aSizes.at(1));

    std::vector<SharedRef<Cog>> cogs;
    cogs.reserve(snames.size());

    auto vit = values.begin();
    for(const auto& sname : snames)
    {
        auto it = scripts.find(sname);
        if(it == scripts.end())
        {
            LOG_ERROR("CND::ParseSection_Cogs(): Can't find cog script '%'", sname);
            throw StreamError("Can't make Cog, CogScript not found");
        }

        Cog c;
        c.id     = std::size(cogs);
        c.script = *it;
        c.flags  = c.script->flags;
        c.vtid   = c.script->getNextVTableId();

        for(auto& s : c.script->symbols)
        {
            if(s.isLocal ||
               s.type == CogSymbol::Message) {
                continue;
            }

            s.vtable[c.vtid] = std::move(*vit);
            // TODO: initialize value with valid type
            ++vit;
        }

        cogs.push_back(std::move(c));
    }

    if(vit != values.end()) {
        throw StreamError("Incomplete initialization of COGs while parsing COG section");
    }

    return cogs;
}

std::vector<SharedRef<Cog>> CND::ReadCogs(const InputStream& istream, const HashMap<SharedRef<CogScript>>& scripts)
{
    auto header = ReadHeader(istream);
    istream.seek(GetOffset_Cogs(istream, header));
    return ParseSection_Cogs(istream, header, scripts);
}

void CND::WriteSection_Cogs(OutputStream& ostream, const std::vector<SharedRef<Cog>>& cogs)
{
    std::vector<std::string> cogvals;
    cogvals.reserve(cogs.size());

    for(const auto& c : cogs)
    {
        if(cogvals.capacity() < c->script->symbols.size()) {
            cogvals.reserve(c->script->symbols.size());
        }

        for(const CogSymbol& s : c->script->symbols)
        {
            if(s.isLocal ||
               !s.vtable.contains(c->vtid)) {
                continue;
            }

            cogvalue_visitor([&](auto&& s) {
                using ST = decltype(s);
                if constexpr(std::is_same_v<std::string_view, std::decay_t<ST>>) {
                    cogvals.emplace_back(s);
                }
                else {
                    cogvals.push_back(std::forward<ST>(s));
                }
            }, s.vtable.at(c->vtid));
        }
    }

    ostream.write(std::array<uint32_t, 2>{
        safe_cast<uint32_t>(cogs.size()),
        safe_cast<uint32_t>(cogvals.size())
    });

    WriteResourceList(ostream, cogs, [](const SharedRef<Cog>& e) { return e->name(); });
    WriteResourceList(ostream, cogvals);
}
