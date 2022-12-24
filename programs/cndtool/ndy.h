#ifndef CNDTOOL_NDY_H
#define CNDTOOL_NDY_H
#include "resource.h"

#include <cmdutils/cmdutils.h>

#include <libim/common.h>
#include <libim/content/asset/world/georesource.h>
#include <libim/content/asset/world/sector.h>
#include <libim/content/asset/world/impl/serialization/cnd/thing/cnd_thing.h>
#include <libim/content/asset/world/impl/serialization/ndy/ndy.h>
#include <libim/content/asset/world/impl/serialization/ndy/thing/ndy_thing_oser.h>
#include <libim/content/audio/soundbank.h>
#include <libim/content/text/text_resource_writer.h>
#include <libim/content/text/text_resource_reader.h>
#include <libim/content/text/impl/text_resource_literals.h>
#include <libim/io/filestream.h>
#include <libim/io/vfs.h>
#include <libim/log/log.h>
#include <libim/utils/utils.h>

#include <exception>
#include <filesystem>
#include <vector>

namespace cndtool {
    namespace fs = std::filesystem;
    using namespace libim;
    using namespace libim::content::asset;
    using namespace libim::content::audio;
    using namespace libim::content::text;
    using namespace libim::utils;

    struct NdyWorld
    {
        CndHeader header;
        std::pair<std::size_t, std::vector<std::string>> sounds;
        std::pair<std::size_t, std::vector<std::string>> materials;
        Georesource georesource;
        std::vector<Sector> sectors;
        std::pair<std::size_t, std::vector<std::string>> aiClasses;
        std::pair<std::size_t, std::vector<std::string>> models;
        std::pair<std::size_t, std::vector<std::string>> sprites;
        std::pair<std::size_t, std::vector<std::string>> keyframes;
        std::pair<std::size_t, std::vector<std::string>> animClasses;
        std::pair<std::size_t, std::vector<std::string>> soundClasses;
        std::pair<std::size_t, std::vector<std::string>> cogScripts;
        std::pair<std::size_t, std::vector<SharedRef<Cog>>> cogs;
        std::pair<std::size_t, IndexMap<CndThing>> templates;
        std::vector<CndThing> things;
        ByteArray pvs;
    };

     /**
     * Verifies that the normal world contains all required data. Throws exception if not.
     * Note, function expects StaticResourceNames to be already initialized when the function is called.
     */
    void checkNdyWorld(const NdyWorld& world, const StaticResourceNames& staticResources)
    {
        using namespace libim::utils;
        check(world.sounds.second.size()       <= kStaticResourceIndexMask , "Too many sounds in the list, max=%!"        , kStaticResourceIndexMask);
        check(world.materials.second.size()    <= kStaticResourceIndexMask , "Too many materials in the list, max=%!"     , kStaticResourceIndexMask);
        check(world.aiClasses.second.size()    <= kStaticResourceIndexMask , "Too many ai classes in the list, max=%!"    , kStaticResourceIndexMask);
        check(world.models.second.size()       <= kStaticResourceIndexMask , "Too many models in the list, max=%!"        , kStaticResourceIndexMask);
        check(world.sprites.second.size()      <= kStaticResourceIndexMask , "Too many sprites in the list, max=%!"       , kStaticResourceIndexMask);
        check(world.keyframes.second.size()    <= kStaticResourceIndexMask , "Too many keyframes in the list, max=%!"     , kStaticResourceIndexMask);
        check(world.animClasses.second.size()  <= kStaticResourceIndexMask , "Too many anim classes in the list, max=%!"  , kStaticResourceIndexMask);
        check(world.soundClasses.second.size() <= kStaticResourceIndexMask , "Too many sound classes in the list, max=%!" , kStaticResourceIndexMask);
        check(world.cogScripts.second.size()   <= kStaticResourceIndexMask , "Too many cog scripts in the list, max=%!"   , kStaticResourceIndexMask);
        check(world.cogs.second.size()         <= kStaticResourceIndexMask , "Too many cogs in the list, max=%!"          , kStaticResourceIndexMask);
        check(world.templates.second.size()    <= kMaxTemplates            , "Too many templates in the list, max=%!"     , kMaxTemplates);
        check(world.things.size()              <= kMaxThings               , "Too many things in the list, max=%!"        , kMaxThings);

        check(world.georesource.adjoins.size()     > 0 , "Georesource section is missing adjoins!");
        check(world.georesource.surfaces.size()    > 0 , "Georesource section is missing surfaces!");
        check(world.georesource.vertices.size()    > 0 , "Georesource section is missing vertices!");
        check(world.georesource.texVertices.size() > 0 , "Georesource section is missing texture vertices!");

        check(world.sectors.size() > 0 , "Sector section is empty!");
        check(world.things.size()  > 0 , "Thing section is empty!");

        // Check surface indices are in bounds
        for (std::size_t i = 0; i < world.georesource.surfaces.size(); i++)
        {
            const Surface& surf = world.georesource.surfaces[i];
            if (int32_t matIdx = surf.matIdx.value_or(-1); matIdx > -1)
            {
                // Check mat index is in bounds
                if (isStaticResource(matIdx)) {
                    check(getStaticResourceIdx(matIdx) < staticResources.materials.size(),
                        "The static material index % of surface % is out of bounds (size=%)!",
                        getStaticResourceIdx(matIdx), i, staticResources.materials.size()
                    );
                }
                else
                {
                    check(matIdx< world.materials.second.size(),
                        "The material index % of surface % is out of bounds (size=%)!",
                        matIdx, i, world.materials.second.size()
                    );
                }
            }

            // Check surface adjoin index
            if (surf.adjoinIdx.has_value())
            {
                check(surf.adjoinIdx.value() < world.georesource.adjoins.size(),
                    "The adjoin index % of surface % is out of bounds (size=%)!",
                    surf.adjoinIdx.value(), i, world.georesource.adjoins.size()
                );
            }

            // Check surface vertices
            check(surf.vecIntensities.size() == surf.vertices.size(),
                "The number of vertex intensities % of surface % does not match the number of surface vertices %!",
                surf.vecIntensities.size(), i, surf.vertices.size()
            );

            for (const auto& vert : surf.vertices)
            {
                // Check vertex indices are in bounds
                check(vert.vertIdx < world.georesource.vertices.size(),
                    "The vertex index % of surface % is out of bounds (size=%)!",
                    vert.vertIdx, i, world.georesource.vertices.size()
                );

                // Check texture vertex indices are in bounds
                check(!vert.uvIdx.has_value() || vert.uvIdx.value() < world.georesource.texVertices.size(),
                    "The texture vertex index % of surface % is out of bounds (size=%)!",
                    vert.uvIdx.value(), i, world.georesource.texVertices.size()
                );
            }
        }
    }

    /**
     * Loads CND file from path and converts it to NDY file format
     */
    bool convertCndToNdy(const fs::path& cndPath, const VirtualFileSystem& vfs, const fs::path& outDir, bool verbose)
    {
        using namespace cmdutils;
        fs::path ndyPath;
        try
        {
            constexpr std::size_t total = 31;
            constexpr auto progressTitle = "Converting to NDY ... "sv;
            std::size_t progress = 0;
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Opening file stream and reading CND header of file %", cndPath);
            InputFileStream icnds(cndPath);
            auto header = CND::readHeader(icnds);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'Sounds' at offset: %", utils::to_string<16>(icnds.tell()));
            SoundBank sb(1);
            CND::parseSection_Sounds(icnds, sb, 0);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'Materials' at offset: %", utils::to_string<16>(icnds.tell()));
            auto materials = CND::parseSection_Materials(icnds, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'GeoResource' at offset: %", utils::to_string<16>(icnds.tell()));
            auto geores = CND::parseSection_Georesource(icnds, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'Sectors' at offset: %", utils::to_string<16>(icnds.tell()));
            auto sectors = CND::parseSection_Sectors(icnds, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'AIClasses' at offset: %", utils::to_string<16>(icnds.tell()));
            auto aiclasses = CND::parseSection_AIClasses(icnds, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'Models' at offset: %", utils::to_string<16>(icnds.tell()));
            auto modelNames = CND::parseSection_Models(icnds, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'Sprites' at offset: %", utils::to_string<16>(icnds.tell()));
            auto sprites = CND::parseSection_Sprites(icnds, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'Keyframes' at offset: %", utils::to_string<16>(icnds.tell()));
            auto keyframes = CND::parseSection_Keyframes(icnds, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'AnimClass' at offset: %", utils::to_string<16>(icnds.tell()));
            auto pupNames = CND::parseSection_AnimClasses(icnds, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'SoundClass' at offset: %", utils::to_string<16>(icnds.tell()));
            auto sndNames = CND::parseSection_SoundClasses(icnds, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'COGScripts' at offset: %", utils::to_string<16>(icnds.tell()));
            auto cogScriptNames = CND::parseSection_CogScripts(icnds, header);

            LOG_DEBUG("Loading % cog scripts from VFS ...", utils::to_string(cogScriptNames.size()));
            auto scripts = loadCogScripts(vfs, cogScriptNames, /*bFixCogScripts=*/true);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'COGs' at offset: %", utils::to_string<16>(icnds.tell()));
            auto cogs = CND::parseSection_Cogs(icnds, header, scripts);

            LOG_DEBUG("Verifying init symbol values for % COGs ...", utils::to_string(cogs.size()));
            verifyCogs(cogs);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'Templates' at offset: %", utils::to_string<16>(icnds.tell()));
            auto cndTemplates = CND::parseSection_Templates(icnds, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'Things' at offset: %", utils::to_string<16>(icnds.tell()));
            auto cndThings = CND::parseSection_Things(icnds, header, cndTemplates);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Parsing CND section 'PVS' at offset: %", utils::to_string<16>(icnds.tell()));
            auto pvs = CND::parseSection_PVS(icnds);
            if (!verbose) printProgress(progressTitle, progress++, total);

            //////////////////////////////////////////////////////////////////////////////////////////
            // Write NDY file
            //////////////////////////////////////////////////////////////////////////////////////////
            ndyPath = outDir / "ndy" / cndPath.filename().replace_extension("ndy");
            LOG_DEBUG("Creating output NDY file path %", ndyPath);
            makePath(ndyPath);

            LOG_DEBUG("Opening output NDY file %", ndyPath);
            OutputFileStream osndy(ndyPath, /*truncate=*/true);
            TextResourceWriter ndytw(osndy);

            LOG_DEBUG("Writing NDY section 'Copyright' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Copyright(ndytw);
            LOG_DEBUG("Writing NDY section 'Header' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Header(ndytw, header);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'Sounds' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Sounds(ndytw, header.numSounds, sb.getTrack(0));
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'Materials' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Materials(ndytw, materials);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'GeoResource' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Georesource(ndytw, geores);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'Sectors' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Sectors(ndytw, sectors);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'AIClass' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_AIClasses(ndytw, header.sizeAIClasses, aiclasses);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'Models' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Models(ndytw, header.sizeModels, modelNames);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'Sprites' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Sprites(ndytw, header.sizeSprites, sprites);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'Keyframes' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Keyframes(ndytw, header.sizeKeyframes, keyframes);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'AnimClass' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_AnimClasses(ndytw, header.sizePuppets, pupNames);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'SoundClass' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_SoundClasses(ndytw, header.sizeSoundClasses, sndNames);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'COGScripts' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_CogScripts(ndytw, header.sizeCogScripts, cogScriptNames);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'COGs' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Cogs(ndytw, header.sizeCogs, cogs);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'Templates' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Templates(ndytw, header.sizeThingTemplates, cndTemplates);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'Things' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_Things(ndytw, cndThings, cndTemplates);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing NDY section 'PVS' at offset: %", utils::to_string<16>(osndy.tell()));
            NDY::writeSection_PVS(ndytw, pvs, sectors);
            if (!verbose) std::cout << "\r" << progressTitle << kSuccess << std::endl;
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << std::endl;
            printError(e.what());
            deleteFile(ndyPath);
            return false;
        }
    }

    void ndyParseSection(std::string_view section, TextResourceReader& rr, const VirtualFileSystem& vfs, NdyWorld& world)
    {
        try
        {
            LOG_DEBUG("Parsing NDY section: %", section);
            if (iequal(section, NDY::kSectionCopyright))
            {
                if (!NDY::parseSection_Copyright(rr)) {
                    throw std::runtime_error("Invalid NDY file copyright!");
                }
            }
            else if (iequal(section, NDY::kSectionHeader)) {
                world.header = NDY::parseSection_Header(rr);
            }
            else if (iequal(section, NDY::kSectionSounds)) {
                world.sounds = NDY::parseSection_Sounds(rr);
            }
            else if (iequal(section,NDY::kSectionMaterials)) {
                world.materials = NDY::parseSection_Materials(rr);
            }
            else if (iequal(section, NDY::kSectionGeoresource)) {
                world.georesource = NDY::parseSection_Georesource(rr);
            }
            else if (iequal(section, NDY::kSectionSectors)) {
                world.sectors = NDY::parseSection_Sectors(rr);
            }
            else if (iequal(section, NDY::kSectionAIClass)) {
                world.aiClasses = NDY::parseSection_AIClasses(rr);
            }
            else if (iequal(section, NDY::kSectionModels)) {
                world.models = NDY::parseSection_Models(rr);
            }
            else if (iequal(section, NDY::kSectionSprites)) {
                world.sprites = NDY::parseSection_Sprites(rr);
            }
            else if (iequal(section, NDY::kSectionKeyframes)) {
                world.keyframes = NDY::parseSection_Keyframes(rr);
            }
            else if (iequal(section, NDY::kSectionAnimClass)) {
                world.animClasses = NDY::parseSection_AnimClasses(rr);
            }
            else if (iequal(section, NDY::kSectionSoundClass)) {
                world.soundClasses = NDY::parseSection_SoundClasses(rr);
            }
            else if (iequal(section,  NDY::kSectionCogScripts)) {
                world.cogScripts = NDY::parseSection_CogScripts(rr);
            }
            else if (iequal(section, NDY::kSectionCogs))
            {
                // Load scripts from files
                auto scripts = loadCogScripts(vfs, world.cogScripts.second, /*bFixCogScripts=*/true);
                world.cogs = NDY::parseSection_Cogs(rr, scripts);
                verifyCogs(world.cogs.second);
            }
            else if (iequal(section, NDY::kSectionTemplates)) {
                world.templates = NDY::parseSection_Templates(rr);
            }
            else if (iequal(section, NDY::kSectionThings)) {
                world.things = NDY::parseSection_Things(rr, world.templates.second);
            }
            else if (iequal(section, NDY::kSectionPVS)) {
                world.pvs = NDY::parseSection_PVS(rr, world.sectors);
            }
            else
            {
                auto loc = rr.currentLocation();
                LOG_INFO("%:%:%: Skipping unknown section '%'", loc.filename, loc.firstLine, loc.firstColumn, section);
            }
        }
        catch (const SyntaxError& e)
        {
            auto loc = e.location();
            throw std::runtime_error(
                utils::format("%:%:%: Syntax error encountered while parsing NDY section '%': %", loc.filename, loc.firstLine, loc.firstColumn, section, e.what())
            );
        }
        catch (const std::exception& e)
        {
            auto loc = rr.currentLocation();
            throw std::runtime_error(
                utils::format("%:%:%: An exception encountered while parsing NDY section '%': %", loc.filename, loc.firstLine, loc.firstColumn, section, e.what())
            );
        }
    }

    NdyWorld ndyReadFile(const fs::path& ndyPath, const VirtualFileSystem& vfs)
    {
        InputFileStream ndyStream(ndyPath);
        TextResourceReader ndytrr(ndyStream);

        NdyWorld world{};

        AT_SCOPE_EXIT([&ndytrr, reol = ndytrr.reportEol()]{
            ndytrr.setReportEol(reol);
        });
        ndytrr.setReportEol(false);

        Token tkn;
        std::string section; // having this here avoids deallocation of the string on each iteration
        while (ndytrr.peekNextToken(tkn))
        {
            if (!iequal(tkn.value(), kResName_Section))
            {
                ndytrr.skipNextToken();
                continue;
            }

            section = std::string(ndytrr.readSection());
            ndyParseSection(section, ndytrr, vfs, world);
        }

        return world;
    }
}

#endif // CNDTOOL_NDY_H