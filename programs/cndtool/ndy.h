#ifndef CNDTOOL_NDY_H
#define CNDTOOL_NDY_H
#include "resource.h"

#include <cmdutils/cmdutils.h>

#include <libim/content/asset/world/impl/serialization/ndy/ndy.h>
#include <libim/content/asset/world/impl/serialization/ndy/thing/ndy_thing_oser.h>
#include <libim/content/audio/soundbank.h>
#include <libim/io/filestream.h>
#include <libim/io/vfs.h>
#include <libim/log/log.h>

#include <exception>
#include <filesystem>

namespace cndtool {
    namespace fs = std::filesystem;

    bool convertToNdy(const fs::path& cndPath, const libim::VirtualFileSystem& vfs, const fs::path& outDir, bool verbose)
    {
        using namespace cmdutils;
        using namespace libim;
        using namespace libim::content::asset;
        using namespace libim::content::audio;

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
}

#endif // CNDTOOL_NDY_H