#include "cnd.h"
#include "animation/cnd_key_structs.h"
#include "sector/cnd_sector.h"
#include "thing/cnd_thing.h"
#include "../world_ser_common.h"

#include <libim/utils/utils.h>
#include <libim/types/safe_cast.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <variant>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;
using namespace std::string_literals;

static constexpr uint32_t kFileVersion = 3;


std::vector<std::string> readResourceList(const InputStream& istream, std::size_t size)
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
void writeResourceList(OutputStream& ostream, const List<T, Args...>& list, Lambda&& nameExtractor)
{
    std::vector<CndResourceName> wlist;
    wlist.reserve(list.size());

    std::transform(list.begin(), list.end(), std::back_insert_iterator(wlist),
    [&](const T& e)
    {
        CndResourceName aName;
        if(!utils::strcpy(aName, nameExtractor(e))) {
            throw std::runtime_error("Too long resource name to write to CND stream");
        }
        return aName;
    });

    ostream.write(wlist);
}

template<template<typename, typename ...> class List, typename ...Args>
void writeResourceList(OutputStream& ostream, const List<std::string, Args...>& list)
{
     writeResourceList(ostream, list, [](const std::string& e){ return e; });
}



CndHeader CND::readHeader(const InputStream& istream)
{
    istream.seekBegin();
    CndHeader cndHeader = istream.read<CndHeader>();

    /* Verify file copyright notice  */
    if (!std::equal(cndHeader.copyright.begin(), cndHeader.copyright.end(), kFileCopyright.begin())) {
        throw CNDError("readHeader", "Error bad CND file copyright");
    }

    /* Verify file version */
    if (cndHeader.version != kFileVersion) {
        throw CNDError("readHeader", "Error wrong CND file version: " + std::to_string(cndHeader.version));
    }

    return cndHeader;
}

std::size_t CND::getOffset_AIClass(const InputStream& istream, const CndHeader& header)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    istream.seek(
        getOffset_Sectors(istream, header) +
        sizeof(CndSectorHeader) * header.numSectors
    );

    auto vecBuffSize = istream.read<uint32_t>();
    return istream.tell() + vecBuffSize * sizeof(uint32_t);
}

std::vector<std::string> CND::parseSection_AIClass(const InputStream& istream, const CndHeader& header)
{
    try {
        return readResourceList(istream, header.numAIClasses);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_AIClass",
            "An exception was encountered while parsing section 'AIClass': "s + e.what()
        );
    }
}

std::vector<std::string> CND::readAIClass(const InputStream& istream)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_AIClass(istream, header));
    return parseSection_AIClass(istream, header);
}

void CND::writeSection_AIClass(OutputStream& ostream, const std::vector<std::string>& aiclasses)
{
    try {
        writeResourceList(ostream, aiclasses);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_AIClass",
            "An exception was encountered while writing section 'AIClass': "s + e.what()
        );
    }
}

std::size_t CND::getOffset_Models(const InputStream& istream, const CndHeader& header)
{
    return getOffset_AIClass(istream, header) + header.numAIClasses * sizeof(CndResourceName);
}

std::vector<std::string> CND::parseSection_Models(const InputStream& istream, const CndHeader& header)
{
    try {
        return readResourceList(istream, header.numModels);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_Models",
            "An exception was encountered while parsing section 'Models': "s + e.what()
        );
    }
}

std::vector<std::string> CND::readModels(const InputStream& istream)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_Models(istream, header));
    return parseSection_Models(istream, header);
}

void CND::writeSection_Models(OutputStream& ostream, const std::vector<std::string>& models)
{
    try {
        writeResourceList(ostream, models);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_Models",
            "An exception was encountered while writing section 'Models': "s + e.what()
        );
    }
}

std::size_t CND::getOffset_Sprites(const InputStream& istream, const CndHeader& header)
{
    return getOffset_Models(istream, header) + header.numModels * sizeof(CndResourceName);
}

std::vector<std::string> CND::parseSection_Sprites(const InputStream& istream, const CndHeader& header)
{
    try{
        return readResourceList(istream, header.numSprites);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_Sprites",
            "An exception was encountered while parsing section 'Sprites': "s + e.what()
        );
    }
}

std::vector<std::string> CND::readSprites(const InputStream& istream)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_Sprites(istream, header));
    return parseSection_Sprites(istream, header);
}

void CND::WriteSection_Sprites(OutputStream& ostream, const std::vector<std::string>& sprites)
{
    try {
        writeResourceList(ostream, sprites);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_Sprites",
            "An exception was encountered while writing section 'Sprites': "s + e.what()
        );
    }
}

std::size_t CND::getOffset_AnimClass(const InputStream& istream, const CndHeader& header)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    istream.seek(getOffset_Keyframes(istream, header));
    auto aSizes = istream.read<std::array<uint32_t, 3>>();

    return istream.tell() +
           sizeof(CndKeyHeader) * header.numKeyframes +
           sizeof(KeyMarker) * aSizes.at(0) +
           sizeof(CndKeyNode) * aSizes.at(1) +
           sizeof(KeyNodeEntry) * aSizes.at(2);
}

std::vector<std::string> CND::parseSection_AnimClass(const InputStream& istream, const CndHeader& header)
{
    try {
        return readResourceList(istream, header.numPuppets);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_AnimClass",
            "An exception was encountered while parsing section 'AnimClass': "s + e.what()
        );
    }
}

std::vector<std::string> CND::readAnimClass(const InputStream& istream)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_AnimClass(istream, header));
    return parseSection_AnimClass(istream, header);
}

void CND::writeSection_AnimClass(OutputStream& ostream, const std::vector<std::string>& animclasses)
{
    try {
        writeResourceList(ostream, animclasses);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_AnimClass",
            "An exception was encountered while writing section 'AnimClass': "s + e.what()
        );
    }
}

std::size_t CND::getOffset_SoundClass(const InputStream& istream, const CndHeader& header)
{
    return getOffset_AnimClass(istream, header) + sizeof(CndResourceName) * header.numPuppets;
}

std::vector<std::string> CND::parseSection_SoundClass(const InputStream& istream, const CndHeader& header)
{
    try {
        return readResourceList(istream, header.numSoundClasses);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_SoundClass",
            "An exception was encountered while parsing section 'SoundClass': "s + e.what()
        );
    }
}

std::vector<std::string> CND::readSoundClass(const InputStream& istream)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_SoundClass(istream, header));
    return parseSection_SoundClass(istream, header);
}

void CND::writeSection_SoundClass(OutputStream& ostream, const std::vector<std::string>& sndclasses)
{
    try {
        writeResourceList(ostream, sndclasses);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_SoundClass",
            "An exception was encountered while writing section 'SoundClass': "s + e.what()
        );
    }
}

std::size_t CND::getOffset_CogScripts(const InputStream& istream, const CndHeader& header)
{
    return getOffset_SoundClass(istream, header) + sizeof(CndResourceName) * header.numSoundClasses;
}

std::vector<std::string> CND::parseSection_CogScripts(const InputStream& istream, const CndHeader& header)
{
    try {
        return readResourceList(istream, header.numCogScripts);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_CogScripts",
            "An exception was encountered while parsing section 'CogScripts': "s + e.what()
        );
    }
}

std::vector<std::string> CND::readCogScripts(const InputStream& istream)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_CogScripts(istream, header));
    return parseSection_CogScripts(istream, header);
}

void CND::writeSection_CogScripts(OutputStream& ostream, const std::vector<std::string>& scripts)
{
    try {
        writeResourceList(ostream, scripts);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_CogScripts",
            "An exception was encountered while writing section 'CogScripts': "s + e.what()
        );
    }
}


std::size_t CND::getOffset_Cogs(const InputStream& istream, const CndHeader& header)
{
    return getOffset_CogScripts(istream, header) + sizeof(CndResourceName) * header.numCogScripts;
}

std::vector<SharedRef<Cog>> CND::parseSection_Cogs(const InputStream& istream, const CndHeader& header, const IndexMap<SharedRef<CogScript>>& scripts)
{
    try
    {
        auto aSizes = istream.read<std::array<uint32_t, 2>>();
        auto snames = readResourceList(istream, aSizes.at(0));
        auto values = readResourceList(istream, aSizes.at(1));

        std::vector<SharedRef<Cog>> cogs;
        cogs.reserve(snames.size());

        auto vit = values.begin();
        for(const auto& sname : snames)
        {
            auto it = scripts.find(sname);
            if(it == scripts.end()) {
                throw CNDError("parseSection_Cogs",
                    "Can't parse COG, can't find cog script: " + sname
                );
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
            throw CNDError("parseSection_Cogs",
                "Incomplete initialization of COGs while parsing COG section"
            );
        }

        return cogs;
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_Cogs",
            "An exception was encountered while parsing section 'COGs': "s + e.what()
        );
    }
}

std::vector<SharedRef<Cog>> CND::readCogs(const InputStream& istream, const IndexMap<SharedRef<CogScript>>& scripts)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_Cogs(istream, header));
    return parseSection_Cogs(istream, header, scripts);
}

void CND::writeSection_Cogs(OutputStream& ostream, const std::vector<SharedRef<Cog>>& cogs)
{
    try
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

        writeResourceList(ostream, cogs, [](const SharedRef<Cog>& e) { return e->name(); });
        writeResourceList(ostream, cogvals);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_Cogs",
            "An exception was encountered while writing section 'Cogs': "s + e.what()
        );
    }
}


std::size_t CND::getOffset_PVS(const InputStream& istream, const CndHeader& header)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    // PVS section is at the end of thing section
    istream.seek(getOffset_Things(istream, header));
    istream.advance(sizeof(CndThingHeader) * header.numThings);

    auto sizes = istream.read<CndThingParamListSizes>();
    istream.advance(
        sizeof(CndPhysicsInfo)         * sizes.sizePhysicsInfoList    +
        sizeof(uint32_t)               * sizes.sizeNumPathFramesList  +
        sizeof(PathFrame)              * sizes.sizePathFrameList      +
        sizeof(CndActorInfo)           * sizes.sizeActorInfoList      +
        sizeof(CndWeaponInfo)          * sizes.sizeWeaponInfoList     +
        sizeof(CndExplosionInfo)       * sizes.sizeExplosionInfoList  +
        sizeof(CndItemInfo)            * sizes.sizeItemInfoList       +
        sizeof(CndHintUserVal)         * sizes.sizeHintUserValueList  +
        sizeof(CndParticleInfo)        * sizes.sizeParticleInfoList   +
        sizeof(CndAIControlInfoHeader) * sizes.sizeAIControlInfoList  +
        sizeof(Vector3f)               * sizes.sizeAIPathFrameList
    );

    return istream.tell();
}

ByteArray CND::parseSection_PVS(const InputStream& istream)
{
    try {
        return istream.read<ByteArray>(istream.read<uint32_t>());
    }
    catch(const std::exception& e) {
        throw CNDError("parseSection_PVS",
            "An exception was encountered while parsing section 'PVS': "s + e.what()
        );
    }
}

ByteArray CND::readPVS(const InputStream& istream)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_PVS(istream, header));
    return parseSection_PVS(istream);
}

void CND::writeSection_PVS(OutputStream& ostream, const ByteArray& pvs)
{
    try {
        ostream.write<int32_t>(safe_cast<int32_t>(pvs.size()));
        ostream.write(pvs);
    }
    catch(const std::exception& e) {
        throw CNDError("writeSection_PVS",
            "An exception was encountered while writing section 'PVS': "s + e.what()
        );
    }
}
