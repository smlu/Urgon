#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>

#include "../cnd.h"
#include "../../world_ser_common.h"
#include "cnd_thing.h"

#include <libim/utils/utils.h>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;
using namespace std::string_literals;


// TODO:
//template<typename T>
//struct VectorStream : std::vector<T>
//{
//    using std::vector<T>::vector;
//    virtual ~VectorStream()
//    {
//        if(throwIfNotEnd && cit != end()) {
//            throw std::runtime_error("Not all elements were read from VectorStream")
//        }
//    }
//    T getNext()
//    {
//        T e = std::move(*cit_);
//        std::advance(cit_, 1);
//        return e;
//    }

//    template<typename Container>
//    auto copyNext(std::size_t count, Container& dest)
//    {
//        cit_ = utils::copy(cit_, count, dest);
//        return cit_;
//    }
//};

// Usage:
//    auto vs = istream.read<VectorStream<SomeType>>(numToRead);
//    vs.getNext();



template<typename Lambda>
void parseThingList(const InputStream& istream, std::size_t numThings, Lambda&& insertThing)
{
    static_assert(sizeof(PathFrame) == 24);

    // Read list of thing headers and list of sizes for 11 lists following.
    auto headers        = istream.read<std::vector<CndThingHeader>>(numThings);
    auto sizes          = istream.read<CndThingParamListSizes>();

    // Read list of phyisc infos
    auto physicsInfos   = istream.read<std::vector<CndPhysicsInfo>>(sizes.sizePhysicsInfoList);

    // Read list of number of thing path frames & list of thing paths
    auto numPathFrames  = istream.read<std::vector<uint32_t>>(sizes.sizeNumPathFramesList);
    auto pathFrames     = istream.read<std::vector<PathFrame>>(sizes.sizePathFrameList);

    // Read list of thing infos (ActorInfo, WeaponInfo, ExplosionInfo etc...)
    auto actorInfos     = istream.read<std::vector<CndActorInfo>>(sizes.sizeActorInfoList);
    auto weaponInfos    = istream.read<std::vector<CndWeaponInfo>>(sizes.sizeWeaponInfoList);
    auto explosionInfos = istream.read<std::vector<CndExplosionInfo>>(sizes.sizeExplosionInfoList);
    auto itemInfos      = istream.read<std::vector<CndItemInfo>>(sizes.sizeItemInfoList);
    auto hintUserVals   = istream.read<std::vector<CndHintUserVal>>(sizes.sizeHintUserValueList); // idx for partInfos and Hint list size are switched
    auto partInfos      = istream.read<std::vector<CndParticleInfo>>(sizes.sizeParticleInfoList);

    // AI Thing Control infos
    auto aiControlInfos = istream.read<std::vector<CndAIControlInfoHeader>>(sizes.sizeAIControlInfoList);
    auto aiPathFrames   = istream.read<std::vector<Vector3f>>(sizes.sizeAIPathFrameList);

    auto pit    = physicsInfos.cbegin();
    auto npfit  = numPathFrames.cbegin();
    auto pfit   = pathFrames.cbegin();
    auto ait    = actorInfos.cbegin();
    auto wit    = weaponInfos.cbegin();
    auto eit    = explosionInfos.cbegin();
    auto iit    = itemInfos.cbegin();
    auto huvit  = hintUserVals.cbegin();
    auto pait   = partInfos.cbegin();
    auto aicit  = aiControlInfos.cbegin();
    auto aipfit = aiPathFrames.cbegin();

    for (const auto& h : headers)
    {
        CndThing t;
        memcpy(&t, &h, sizeof(CndThingHeader));

        // Copy thing movement info
        if(t.moveType == CndThingMoveType::Physics)
        {
            t.moveInfo = std::move(*pit);
            ++pit;
        }
        else if(t.moveType == CndThingMoveType::Path)
        {
            if(*npfit > 0)
            {
                PathInfo p;
                pfit = utils::copy(pfit, *npfit, p.pathFrames);
                t.moveInfo = std::move(p);
            }
            std::advance(npfit, 1);
        }

        // Set thing info
        switch(t.type)
        {
            case Thing::Actor:
            case Thing::Player:
            {
                t.thingInfo = std::move(*ait);
                ++ait;
            } break;
            case Thing::Weapon:
            {
                t.thingInfo = std::move(*wit);
                ++wit;
            } break;
            case Thing::Explosion:
            {
                t.thingInfo = std::move(*eit);
                ++eit;
            } break;
            case Thing::Item:
            {
                t.thingInfo = std::move(*iit);
                ++iit;
            } break;
            case Thing::Hint:
            {
                t.thingInfo = std::move(*huvit);
                ++huvit;
            } break;
            case Thing::Particle:
            {
                t.thingInfo = std::move(*pait);
                ++pait;
            } break;
            default:
                break;
        }

        if(t.controlType == CndThingControlType::AI)
        {
            CndAIControlInfo ai;
            ai.aiFileName = aicit->aiFileName;
            if(aicit->numPathFrames > 0) {
                aipfit = utils::copy(aipfit, aicit->numPathFrames, ai.pathFrames);
            }
            t.controlInfo = std::move(ai);
            std::advance(aicit, 1);
        }

        // Push new thing to return list
        insertThing(std::move(t));
    }

    world_ser_assert(pit == physicsInfos.end()    , "Not all parsed PhysicsInfos were used");
    world_ser_assert(npfit == numPathFrames.end() , "Not all parsed path frames were used");
    world_ser_assert(pfit == pathFrames.end()     , "Not all parsed path frames were used");
    world_ser_assert(ait == actorInfos.end()      , "Not all parsed ActorInfos were used");
    world_ser_assert(wit == weaponInfos.end()     , "Not all parsed WeaponInfos were used");
    world_ser_assert(eit == explosionInfos.end()  , "Not all parsed ExplosionInfos were used");
    world_ser_assert(iit == itemInfos.end()       , "Not all parsed ItemInfos were used");
    world_ser_assert(pait == partInfos.end()      , "Not all parsed ParticleInfos were used");
    world_ser_assert(aicit == aiControlInfos.end(), "Not all parsed AIControlInfos were used");
    world_ser_assert(aipfit == aiPathFrames.end() , "Not all parsed AIPathFrames were used");
}

template<typename Container>
void writeThingList(OutputStream& ostream, const Container& c)
{
    std::vector<CndThingHeader> headers(std::size(c));
    auto hit = headers.begin();

    auto physicsInfos   = std::vector<CndPhysicsInfo>();
    auto numPathFrames  = std::vector<int32_t>();
    auto pathFrames     = std::vector<PathFrame>();
    auto actorInfos     = std::vector<CndActorInfo>();
    auto weaponInfos    = std::vector<CndWeaponInfo>();
    auto explosionInfos = std::vector<CndExplosionInfo>();
    auto itemInfos      = std::vector<CndItemInfo>();
    auto hintUserVals   = std::vector<CndHintUserVal>();
    auto particleInfos  = std::vector<CndParticleInfo>();
    auto aiControlInfos = std::vector<CndAIControlInfoHeader>();
    auto aiPathFrames   = std::vector<Vector3f>();
    auto reserve = [](auto& c) {
        if(c.capacity() == 0) {
            c.reserve(20);
        }
    };

    // Fill headers and other infos vectors
    for (const CndThing& t : c)
    {
        memcpy(&(*hit), &t, sizeof(CndThingHeader));
        std::advance(hit, 1);

        // Copy thing movement info
        if(t.moveType == CndThingMoveType::Physics)
        {
            reserve(physicsInfos);
            physicsInfos.push_back(
                std::get<CndPhysicsInfo>(t.moveInfo) // Should throw an exception if object is missing
            );
        }
        else if(t.moveType == CndThingMoveType::Path)
        {
            int32_t numFrames = 0;
            if(auto pi = std::get_if<PathInfo>(&t.moveInfo))
            {
                const auto&frames = pi->pathFrames;
                numFrames = safe_cast<decltype(numPathFrames)::value_type>(
                    frames.size()
                );

                if(!frames.empty())
                {
                    reserve(pathFrames);
                    utils::copy(frames.begin(), frames.size(), pathFrames);
                }
            }

            reserve(numPathFrames);
            numPathFrames.push_back(numFrames);
        }

        // Set thing info
        switch(t.type)
        {
            case Thing::Actor:
            case Thing::Player:
            {
                reserve(actorInfos);
                actorInfos.push_back(
                    std::get<CndActorInfo>(t.thingInfo) // Should throw an exception if object is missing
                );
            } break;
            case Thing::Weapon:
            {
                reserve(weaponInfos);
                weaponInfos.push_back(
                    std::get<CndWeaponInfo>(t.thingInfo) // Should throw an exception if object is missing
                );
            } break;
            case Thing::Explosion:
            {
                reserve(explosionInfos);
                explosionInfos.push_back(
                    std::get<CndExplosionInfo>(t.thingInfo) // Should throw an exception if object is missing
                );
            } break;
            case Thing::Item:
            {
                reserve(itemInfos);
                itemInfos.push_back(
                    std::get<CndItemInfo>(t.thingInfo) // Should throw an exception if object is missing
                );
            } break;
            case Thing::Hint:
            {
                reserve(hintUserVals);
                hintUserVals.push_back(
                    std::get<CndHintUserVal>(t.thingInfo) // Should throw an exception if object is missing
                );
            } break;
            case Thing::Particle:
            {
                reserve(particleInfos);
                particleInfos.push_back(
                    std::get<CndParticleInfo>(t.thingInfo) // Should throw an exception if object is missing
                );
            } break;
            default:
                break;
        }

        if(t.controlType == CndThingControlType::AI)
        {
            auto& ai = std::get<CndAIControlInfo>(t.controlInfo); // Should throw an exception if object is missing

            CndAIControlInfoHeader h;
            h.aiFileName = ai.aiFileName;
            h.numPathFrames = safe_cast<decltype(h.numPathFrames)>(
                ai.pathFrames.size()
            );

            reserve(aiControlInfos);
            aiControlInfos.push_back(std::move(h));

            if(!ai.pathFrames.empty())
            {
                reserve(aiPathFrames);
                utils::copy(ai.pathFrames.begin(), ai.pathFrames.size(), aiPathFrames);
            }
        }
    }

    // Construct list sizes struct
    CndThingParamListSizes sizes;
    sizes.sizePhysicsInfoList   = safe_cast<uint32_t>(physicsInfos.size());
    sizes.sizeNumPathFramesList = safe_cast<uint32_t>(numPathFrames.size());
    sizes.sizePathFrameList     = safe_cast<uint32_t>(pathFrames.size());
    sizes.sizeActorInfoList     = safe_cast<uint32_t>(actorInfos.size());
    sizes.sizeWeaponInfoList    = safe_cast<uint32_t>(weaponInfos.size());
    sizes.sizeExplosionInfoList = safe_cast<uint32_t>(explosionInfos.size());
    sizes.sizeItemInfoList      = safe_cast<uint32_t>(itemInfos.size());
    sizes.sizeHintUserValueList = safe_cast<uint32_t>(hintUserVals.size());
    sizes.sizeParticleInfoList  = safe_cast<uint32_t>(particleInfos.size());
    sizes.sizeAIControlInfoList = safe_cast<uint32_t>(aiControlInfos.size());
    sizes.sizeAIPathFrameList   = safe_cast<uint32_t>(aiPathFrames.size());

    // Write lists to stream
    ostream << headers;
    ostream << sizes;
    ostream << physicsInfos;
    ostream << numPathFrames;
    ostream << pathFrames;
    ostream << actorInfos;
    ostream << weaponInfos;
    ostream << explosionInfos;
    ostream << itemInfos;
    ostream << hintUserVals;
    ostream << particleInfos;
    ostream << aiControlInfos;
    ostream << aiPathFrames;
}

/**********************/
/* Section Templates  */
/**********************/
std::size_t CND::getOffset_Templates(const InputStream& istream, const CndHeader& header)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    istream.seek(getOffset_Cogs(istream, header));
    auto aSizes = istream.read<std::array<uint32_t, 2>>();
    return istream.tell() +
           aSizes.at(0) * sizeof(CndResourceName) +
           aSizes.at(1) * sizeof(CndResourceName);
}

HashMap<CndThing> CND::parseSection_Templates(const InputStream& istream, const CndHeader& header)
{
    try
    {
        HashMap<CndThing> templates;
        templates.reserve(header.numThingTemplates);
        parseThingList(istream, header.numThingTemplates, [&](CndThing&& t){
            world_ser_assert(templates.pushBack(t.name, std::move(t)).second,
                "Found duplicated template '" + t.name.toStdString() + "'"
            );
        });
        return templates;
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_Templates",
            "An exception was encountered while parsing section 'Templates': "s + e.what()
        );
    }
}

HashMap<CndThing> CND::readTemplates(const InputStream& istream)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_Templates(istream, header));
    return parseSection_Templates(istream, header);
}

void CND::writeSection_Templates(OutputStream& ostream, const HashMap<CndThing>& templates)
{
    try {
        writeThingList(ostream, templates);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_Templates",
            "An exception was encountered while writing section 'Templates': "s + e.what()
        );
    }
}


/**********************/
/* Section Things     */
/**********************/
std::size_t CND::getOffset_Things(const InputStream& istream, const CndHeader& header)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    istream.seek(getOffset_Templates(istream, header));
    istream.advance(sizeof(CndThingHeader) * header.numThingTemplates);

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

std::vector<CndThing> CND::parseSection_Things(const InputStream& istream, const CndHeader& header)
{
    try
    {
        std::vector<CndThing> things;
        things.reserve(header.numThings);
        parseThingList(istream, header.numThings, [&](CndThing&& t) {
            things.push_back(std::move(t));
        });
        return things;
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_Things",
            "An exception was encountered while parsing section 'Things': "s + e.what()
        );
    }
}

std::vector<CndThing> CND::readThings(const InputStream& istream)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_Things(istream, header));
    return parseSection_Things(istream, header);
}

void CND::writeSection_Things(OutputStream& ostream, const std::vector<CndThing>& things)
{
    try {
        writeThingList(ostream, things);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_Things",
            "An exception was encountered while writing section 'Things': "s + e.what()
        );
    }
}
