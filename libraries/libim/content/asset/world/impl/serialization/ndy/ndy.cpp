#include "ndy.h"
#include "../world_ser_common.h"
#include <libim/content/text/impl/text_resource_literals.h>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::audio;
using namespace libim::content::audio::impl;
using namespace libim::content::text;
using namespace libim::utils;


static constexpr std::size_t kFileVersion          = 3;
static constexpr float       kDefaultHorizonPixels = 768.f;
static constexpr std::size_t kCopyrightLineWidth   = 32;

static constexpr auto kCeilingSkyOffset = "Ceiling Sky Offset"sv;
static constexpr auto kCeilingSkyZ      = "Ceiling Sky Z"sv;
static constexpr auto kFog              = "Fog"sv;
static constexpr auto kHorizonDistance  = "Horizon Distance"sv;
static constexpr auto kHorizonPixels    = "Horizon Pixels per Rev"sv;
static constexpr auto kHorizonSkyOffset = "Horizon Sky Offset"sv;
static constexpr auto kLODDistances     = "LOD Distances"sv;
static constexpr auto kVersion          = "Version"sv;
static constexpr auto kWorldGravity     = "World Gravity"sv;

// Other sections constants
static constexpr auto kWorldSounds       = "World sounds"sv;
static constexpr auto kWorldMaterials    = "World materials"sv;
static constexpr auto kWorldAIClasses    = "World AIClasses"sv;
static constexpr auto kWorldModels       = "World models"sv;
static constexpr auto kWorldSprites      = "World sprites"sv;
static constexpr auto kWorldKeyframes    = "World keyframes"sv;
static constexpr auto kWorldPuppets      = "World puppets"sv;
static constexpr auto kWorldSoundClasses = "World soundclasses"sv;
static constexpr auto kWorldScripts      = "World scripts"sv;
static constexpr auto kPvsSize           = "PVS size:"sv;


bool NDY::parseSection_Copyright(TextResourceReader& rr)
{
    const std::size_t nLines    = kFileCopyright.size() / kCopyrightLineWidth;
    const std::size_t nReadLen  = kFileCopyright.size() + nLines;
    std::string copyright(rr.getString(nReadLen));

    copyright.erase(std::remove(copyright.begin(), copyright.end(), '\n'), copyright.end());
    return copyright == kFileCopyright;
}

void NDY::writeSection_Copyright(TextResourceWriter& rw)
{
    rw.writeLine("#### Copyright information #####"sv);
    rw.writeSection(kSectionCopyright, /*overline=*/ false);

    for(std::size_t i = 0; i < kFileCopyright.size(); i += kCopyrightLineWidth) {
        rw.writeLine(kFileCopyright.substr(i, kCopyrightLineWidth));
    }

    rw.writeLine("################################"sv);
    rw.writeEol();
    rw.writeEol();
}

CndHeader NDY::parseSection_Header(TextResourceReader& rr)
{
    CndHeader h {};

    h.version = rr.readKey<decltype(h.version)>(kVersion);
    if(h.version != kFileVersion) {
        throw StreamError("Error wrong NDY file version: " + std::to_string(h.version));
    }

    h.worldGravity    = rr.readKey<decltype(h.worldGravity)>(kWorldGravity);
    h.ceilingSky_Z    = rr.readKey<decltype(h.ceilingSky_Z)>(kCeilingSkyZ);
    h.horizonDistance = rr.readKey<decltype(h.horizonDistance)>(kHorizonDistance);

    rr.readKey<decltype(kDefaultHorizonPixels)>(kHorizonPixels);
    h.horizonSkyOffset = rr.readKey<decltype(h.horizonSkyOffset)>(kHorizonSkyOffset);
    h.ceilingSkyOffset = rr.readKey<decltype(h.ceilingSkyOffset)>(kCeilingSkyOffset);
    h.lodDistances     = rr.readKey<decltype(h.lodDistances)>(kLODDistances);

    rr.assertIdentifier(kFog);
    h.fog.enabled    = rr.getNumber<bool>();
    h.fog.color      = rr.readVector<decltype(h.fog.color)>();
    h.fog.startDepth = rr.getNumber<decltype(h.fog.startDepth)>();
    h.fog.endDepth   = rr.getNumber<decltype(h.fog.endDepth)>();

    return h;
}

void NDY::writeSection_Header(TextResourceWriter& rw, const CndHeader& header)
{
    auto indentWidth = [&](const auto& k) {
        return kHorizonPixels.size() + 3 - k.size();
    };

    rw.writeLine("###### Header information ######"sv);
    rw.writeSection(kSectionHeader, /*overline=*/ false);

    rw.writeLine("# version and global constant settings"sv);
    rw.writeKeyValue(kVersion         , kFileVersion           , indentWidth(kVersion)             );
    rw.writeKeyValue(kWorldGravity    , header.worldGravity    , indentWidth(kWorldGravity)        );
    rw.writeKeyValue(kCeilingSkyZ     , header.ceilingSky_Z    , indentWidth(kCeilingSkyZ)         );
    rw.writeKeyValue(kHorizonDistance , header.horizonDistance , indentWidth(kHorizonDistance)     );
    rw.writeKeyValue(kHorizonPixels   , kDefaultHorizonPixels  , indentWidth(kHorizonPixels)       );
    rw.writeKeyValue(kHorizonSkyOffset, header.horizonSkyOffset, indentWidth(kHorizonSkyOffset) - 1);
    rw.writeKeyValue(kCeilingSkyOffset, header.ceilingSkyOffset, indentWidth(kCeilingSkyOffset) - 1);
    rw.writeKeyValue(kLODDistances    , header.lodDistances    , indentWidth(kLODDistances)     - 1);


    rw.write(kFog);

    rw.indent(indentWidth(kFog));
    rw.writeNumber(header.fog.enabled);

    rw.indent(2);
    rw.writeVector(header.fog.color, /*width=*/1);

    rw.indent(3);
    rw.writeNumber<10,8>(header.fog.startDepth);

    rw.indent(3);
    rw.writeNumber<10, 8>(header.fog.endDepth);
    rw.writeEol();

    rw.writeLine("################################"sv);
    rw.writeEol();
    rw.writeEol();
}

std::pair<std::size_t, std::vector<std::string>>
NDY::parseSection_Sounds(TextResourceReader& rr)
{
    return parseResourceSection<false>(rr, kWorldSounds);
}

void NDY::writeSection_Sounds(TextResourceWriter& rw, std::size_t maxWorldSounds, const HashMap<Sound>& track)
{
    writeResourceSection<false>(rw,
        "#### Sound information  #####"sv,
        kSectionSounds,
        kWorldSounds,
        maxWorldSounds,
        track,
        [](const auto& v) { return v.name(); }
    );
}

std::pair<std::size_t, std::vector<std::string>>
NDY::parseSection_Materials(TextResourceReader& rr)
{
    return parseResourceSection<true>(rr, kWorldMaterials);
}

void NDY::writeSection_Materials(TextResourceWriter& rw, const HashMap<Material>& materials)
{
    writeResourceSection<true>(rw,
        "##### Material information #####"sv,
        kSectionMaterials,
        kWorldMaterials,
        materials.size() + 64, // game engine allows max 64 additional materials to be loaded later (through cog script).
        materials,
        [](const auto& v) { return v.name(); }
    );
}

std::pair<std::size_t, std::vector<std::string>>
NDY::parseSection_AIClass(TextResourceReader& rr)
{
    return parseResourceSection<true>(rr, kWorldAIClasses);
}

void NDY::writeSection_AIClass(TextResourceWriter& rw, std::size_t maxWorldAIClasses, const std::vector<std::string>& aiclasses)
{
    writeResourceSection<true>(rw,
        "######### AI Classes ###########"sv,
        kSectionAIClass,
        kWorldAIClasses,
        maxWorldAIClasses,
        aiclasses,
        [](const auto& v) { return v; }
    );
}

std::pair<std::size_t, std::vector<std::string>>
NDY::parseSection_Models(TextResourceReader& rr)
{
    return parseResourceSection<true>(rr, kWorldModels);
}

void NDY::writeSection_Models(TextResourceWriter& rw, std::size_t maxWorldModels, const std::vector<std::string>& models)
{
    writeResourceSection<true>(rw,
        "###### Models information ######"sv,
        kSectionModels,
        kWorldModels,
        maxWorldModels,
        models,
        [](const auto& v) { return v; }
    );
}

std::pair<std::size_t, std::vector<std::string>>
NDY::parseSection_Sprites(TextResourceReader& rr)
{
    return parseResourceSection<true>(rr, kWorldSprites);
}

void NDY::writeSection_Sprites(TextResourceWriter &rw,  std::size_t maxWorldSprites, const std::vector<std::string>& sprites)
{
    writeResourceSection<true>(rw,
        "###### Sprite information ######"sv,
        kSectionSprites,
        kWorldSprites,
        maxWorldSprites,
        sprites,
        [](const auto& v) { return v; }
    );
}

std::pair<std::size_t, std::vector<std::string>>
NDY::parseSection_Keyframes(TextResourceReader& rr)
{
    return parseResourceSection<true>(rr, kWorldKeyframes);
}

void NDY::writeSection_Keyframes(TextResourceWriter &rw, std::size_t maxWorldKeyframes, const HashMap<Animation>& keyframes)
{
    writeResourceSection<true>(rw,
        "##### Keyframe information #####"sv,
        kSectionKeyframes,
        kWorldKeyframes,
        maxWorldKeyframes,
        keyframes,
        [](const auto& v) { return v.name(); }
    );
}

std::pair<std::size_t, std::vector<std::string>>
NDY::parseSection_AnimClass(TextResourceReader& rr)
{
    return parseResourceSection<true>(rr, kWorldPuppets);
}

void NDY::writeSection_AnimClass(TextResourceWriter &rw, std::size_t maxWorldPuppets, const std::vector<std::string>& puppets)
{
    writeResourceSection<true>(rw,
        "###### Animation Classes #######"sv,
        kSectionAnimClass,
        kWorldPuppets,
        maxWorldPuppets,
        puppets,
        [](const auto& v) { return v; }
    );
}

std::pair<std::size_t, std::vector<std::string>>
NDY::parseSection_SoundClass(TextResourceReader& rr)
{
    return parseResourceSection<true>(rr, kWorldSoundClasses);
}

void NDY::writeSection_SoundClass(TextResourceWriter &rw, std::size_t maxWorldSndClasses, const std::vector<std::string>& sndclasses)
{
    writeResourceSection<true>(rw,
        "######### Sound Classes ########"sv,
        kSectionSoundClass,
        kWorldSoundClasses,
        maxWorldSndClasses,
        sndclasses,
        [](const auto& v) { return v; }
    );
}

std::pair<std::size_t, std::vector<std::string>>
NDY::parseSection_CogScripts(TextResourceReader& rr)
{
    return parseResourceSection<true>(rr, kWorldScripts);
}

void NDY::writeSection_CogScripts(TextResourceWriter &rw, std::size_t maxWorldCogScripts, const std::vector<std::string>& scripts)
{
    writeResourceSection<true>(rw,
        "########## COG scripts #########"sv,
        kSectionCogScripts,
        kWorldScripts,
        maxWorldCogScripts,
        scripts,
        [](const auto& v) { return v; }
    );
}


void NDY::writeSection_PVS(TextResourceWriter& rw, const ByteArray& pvs, const std::vector<Sector>& sectors)
{
    if (pvs.empty()) return; // Don't write empty section

    rw.writeLine("########### PVS Info ###########"sv);
    rw.writeSection(kSectionPVS, /*overline=*/ false);
    rw.writeEol();
    rw.writeKeyValue(kPvsSize, pvs.size());

    // Write PVS bytes as 32bit integers
    for (size_t i = 0; i < pvs.size(); i += 4)
    {
        (i % 64 == 0) ? rw.writeEol() : rw.indent(1); // Write 64 bytes per line
        uint32_t pves = 0;
        if (pvs.size() - i < sizeof(pves)) {
            std::copy(&pvs.at(i), &pvs.at(i) + (pvs.size() - i), reinterpret_cast<byte_t*>(&pves));
        }
        else {
            pves = reinterpret_cast<const uint32_t&>(pvs.at(i));
        }
        rw.writeNumber</*base=*/16, /*precision=*/8>(pves);
    }
    rw.writeEol();
    rw.writeEol();

    //Write pvsIdx of each sector
    rw.writeCommentLine("--- Sectors PVS idx ---"sv);
    for(const auto& s : sectors)
    {
        rw.writeNumber</*base=*/16>(s.pvsIdx);
        rw.writeEol();
    }
    rw.writeLine("################################"sv);
}
