#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>

#include "../cnd.h"
#include "../../world_ser_common.h"
#include "cnd_thing.h"

#include <libim/types/safe_cast.h>
#include <libim/utils/utils.h>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;
using namespace std::string_literals;
using namespace std::string_view_literals;


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


[[nodiscard]] inline const CndThing* getBaseTemplate(std::string_view name, const UniqueTable<CndThing>& templates)
{
    if (name.empty() || iequal(name, "none"sv)) {
        return nullptr;
    }
    auto it = templates.find(name);
    world_ser_assert(it != templates.end(),
        utils::format("Failed to find base template '%'", std::string_view{ name })
    );
    return &(*it);
}

template<typename VariantMemberT, typename T, typename VariantT, typename ExtractorT>
inline bool getVariantValue(T& value, const VariantT& var, ExtractorT&& extractor)
{
    if (const auto* pvm = std::get_if<VariantMemberT>(&var)) {
        value = extractor(*pvm);
        return true;
    }
    return false;
}

template<typename VariantMemberT, typename VariantT, typename SetterT>
inline bool setVariantValue(VariantT& var, SetterT&& setter)
{
    if (auto* pvm = std::get_if<VariantMemberT>(&var)) {
        setter(*pvm);
        return true;
    }
    return false;
}

template<typename ThingInfoType, typename SetterT>
void thingAndBaseInfo(CndThing& thing, const CndThing* base, SetterT&& setter)
{
    if (base) {
        if (const auto* pbi = std::get_if<ThingInfoType>(&base->thingInfo)) {
            if (auto* pti = std::get_if<ThingInfoType>(&thing.thingInfo)) {
                setter(*pti, *pbi);
            }
        }
    }
}


template<typename Lambda>
void parseThingList(const InputStream& istream, std::size_t numThings, const UniqueTable<CndThing>& templates, Lambda&& insertThing)
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

        const auto* pBaseTemplate = getBaseTemplate(h.baseName, templates);
        if (pBaseTemplate) {
            t = *pBaseTemplate;
        }

        // Copy header data
        t.baseName         = h.baseName;
        t.name             = h.name;
        t.position         = h.position;
        t.pyrOrient        = h.pyrOrient;
        t.unknown          = h.unknown;
        t.sectorNum        = h.sectorNum;
        t.type             = h.type;
        t.flags            = h.flags;
        t.moveType         = h.moveType;
        t.controlType      = h.controlType;
        t.msecLifeLeft     = h.msecLifeLeft;
        t.performanceLevel = h.performanceLevel;
        t.sndFilename      = h.sndFilename;
        t.rdThingType      = h.rdThingType;
        t.createThingTemplateName = h.createThingTemplateName;

        if (t.flags & Thing::Flag::Seen) {
            t.flags -= Thing::Flag::Seen;
        }

        memcpy(&t.light, &h.light, sizeof(CndThingLight));
        memcpy(&t.collide, &h.collide, sizeof(CndCollide));

        // Copy resource data if present
        if (!h.rdThingFilename.isEmpty()) {
            t.rdThingFilename = h.rdThingFilename;
        }

        if (!h.pupFilename.isEmpty()) {
            t.pupFilename = h.pupFilename;
        }

        if (!h.cogScriptFilename.isEmpty()) {
            t.cogScriptFilename = h.cogScriptFilename;
        }

        // Copy thing movement info
        if (t.moveType == CndThingMoveType::Physics)
        {
            t.moveInfo = std::move(*pit);
            ++pit;
        }
        else if (t.moveType == CndThingMoveType::Path)
        {
            if (*npfit > 0)
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
                // Copy actor info
                t.thingInfo = std::move(*ait);
                ++ait;

                // Reset empty resource data
                thingAndBaseInfo<CndActorInfo>(t, pBaseTemplate, [](auto& info, const auto& baseInfo) {
                    if (info.weaponTemplateName.isEmpty()) {
                        info.weaponTemplateName = baseInfo.weaponTemplateName;
                    }
                    if (info.explodeTemplateName.isEmpty()) {
                        info.explodeTemplateName = baseInfo.explodeTemplateName;
                    }
                });
            } break;
            case Thing::Weapon:
            {
                // Copy weapon info
                t.thingInfo = std::move(*wit);
                ++wit;

                // Reset empty resource data
                thingAndBaseInfo<CndWeaponInfo>(t, pBaseTemplate, [](auto& info, const auto& baseInfo) {
                    if (info.explosionTemplateName.isEmpty()) {
                        info.explosionTemplateName = baseInfo.explosionTemplateName;
                    }
                });
            } break;
            case Thing::Explosion:
            {
                // Copy explosion info
                t.thingInfo = std::move(*eit);
                ++eit;

                // Reset empty resource data
                thingAndBaseInfo<CndExplosionInfo>(t, pBaseTemplate, [](auto& info, const auto& baseInfo) {
                    if (info.spriteTemplateName.isEmpty()) {
                        info.spriteTemplateName = baseInfo.spriteTemplateName;
                    }
                });
            } break;
            case Thing::Item:
            {
                // Copy item info
                t.thingInfo = std::move(*iit);
                ++iit;
            } break;
            case Thing::Hint:
            {
                // Copy hint info
                t.thingInfo = std::move(*huvit);
                ++huvit;
            } break;
            case Thing::Particle:
            {
                // Copy particle info
                t.thingInfo = std::move(*pait);
                ++pait;

                // Reset empty resource data
                thingAndBaseInfo<CndParticleInfo>(t, pBaseTemplate, [](auto& info, const auto& baseInfo) {
                    if (info.materialFilename.isEmpty()) {
                        info.materialFilename = baseInfo.materialFilename;
                    }
                });
            } break;
            default:
                break;
        }

        if (t.controlType == CndThingControlType::AI)
        {
            if (!std::holds_alternative<CndAIControlInfo>(t.controlInfo)) {
                t.controlInfo = CndAIControlInfo{};
            }

            CndAIControlInfo& ai = std::get<CndAIControlInfo>(t.controlInfo);
            if (!aicit->aiFileName.isEmpty()) {
                ai.aiFileName = aicit->aiFileName;
            }

            if (aicit->numPathFrames > 0) {
                aipfit = utils::copy(aipfit, safe_cast<std::size_t>(aicit->numPathFrames), ai.pathFrames);
            }

            // Advance to next AIControlInfo
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
void writeThingList(OutputStream& ostream, const Container& c, const UniqueTable<CndThing>& templates)
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
        // Make sure free thing has no sector
        if (hit->type == Thing::Free) {
            hit->sectorNum = -1;
        }
        if (hit->flags & Thing::Flag::Seen) {
            hit->flags -= Thing::Flag::Seen;
        }

        const CndThing* pTemplate = getBaseTemplate(hit->baseName, templates);

        // Remove polyline rdname
        if (hit->rdThingType == CndRdThingType::RdPolyline) {
            hit->rdThingFilename = CndResourceName{}; // Empty string
        }

        // Remove 3DO model if it's the same as the template
        if (hit->rdThingType == CndRdThingType::RdModel) {
            if (pTemplate && pTemplate->rdThingFilename == hit->rdThingFilename) {
                hit->rdThingFilename = CndResourceName{}; // Empty string
            }
        }

        // Remove puppet filename if it's the same as the template
        if (pTemplate && pTemplate->pupFilename == hit->pupFilename) {
            hit->pupFilename = CndResourceName{}; // Empty string
        }

        // Remove COG filename if it's the same as the template
        if (pTemplate && pTemplate->cogScriptFilename == hit->cogScriptFilename) {
            hit->cogScriptFilename = CndResourceName{}; // Empty string
        }

        // Advance header iterator
        std::advance(hit, 1);

        // Copy thing movement info
        if (t.moveType == CndThingMoveType::Physics)
        {
            reserve(physicsInfos);
            physicsInfos.push_back(
                std::get<CndPhysicsInfo>(t.moveInfo) // Should throw an exception if object is missing
            );
        }
        else if (t.moveType == CndThingMoveType::Path)
        {
            int32_t numFrames = 0;
            if (auto pi = std::get_if<PathInfo>(&t.moveInfo))
            {
                const auto&frames = pi->pathFrames;
                numFrames = safe_cast<decltype(numPathFrames)::value_type>(
                    frames.size()
                );

                if (!frames.empty())
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

                // Remove explosion template name if it's the same as the base template
                if (pTemplate)
                {
                    if (const auto* pBWi = std::get_if<CndWeaponInfo>(&pTemplate->thingInfo)) {
                        auto& wi = weaponInfos.back();
                        if (pBWi->explosionTemplateName == wi.explosionTemplateName) {
                            wi.explosionTemplateName = CndResourceName{}; // Empty string
                        }
                    }
                }
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

        if (t.controlType == CndThingControlType::AI)
        {
            const auto& ai = std::get<CndAIControlInfo>(t.controlInfo); // Should throw an exception if object is missing

            CndAIControlInfoHeader h{};
            if (!ai.aiFileName.isEmpty()) {
                h.aiFileName = ai.aiFileName;
            }

            h.numPathFrames = safe_cast<decltype(h.numPathFrames)>(
                ai.pathFrames.size()
            );

            if (!ai.pathFrames.empty())
            {

                reserve(aiPathFrames);
                utils::copy(ai.pathFrames.begin(), ai.pathFrames.size(), aiPathFrames);
            }

            reserve(aiControlInfos);
            aiControlInfos.push_back(std::move(h));
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

UniqueTable<CndThing> CND::parseSection_Templates(const InputStream& istream, const CndHeader& header)
{
    try
    {
        UniqueTable<CndThing> templates;
        templates.reserve(header.numThingTemplates);
        parseThingList(istream, header.numThingTemplates, templates, [&](CndThing&& t){
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

UniqueTable<CndThing> CND::readTemplates(const InputStream& istream)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_Templates(istream, header));
    return parseSection_Templates(istream, header);
}

void CND::writeSection_Templates(OutputStream& ostream, const UniqueTable<CndThing>& templates)
{
    try {
        writeThingList(ostream, templates, templates);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_Templates",
            format("An exception was encountered while writing section 'Templates': %", e.what())
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

std::vector<CndThing> CND::parseSection_Things(const InputStream& istream, const CndHeader& header, const UniqueTable<CndThing>& templates)
{
    try
    {
        std::vector<CndThing> things;
        things.reserve(header.numThings);
        parseThingList(istream, header.numThings, templates, [&](CndThing&& t) {
            things.push_back(std::move(t));
        });
        return things;
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("parseSection_Things",
            format("An exception was encountered while writing section 'Things': %", e.what())
        );
    }
}

std::vector<CndThing> CND::readThings(const InputStream& istream, const UniqueTable<CndThing>& templates)
{
    auto header = readHeader(istream);
    istream.seek(getOffset_Things(istream, header));
    return parseSection_Things(istream, header, templates);
}

void CND::writeSection_Things(OutputStream& ostream, const std::vector<CndThing>& things, const UniqueTable<CndThing>& templates)
{
    try {
        writeThingList(ostream, things, templates);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_Things",
            "An exception was encountered while writing section 'Things': "s + e.what()
        );
    }
}
