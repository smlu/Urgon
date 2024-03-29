#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

#include <cmdutils/cmdutils.h>
#include <matool/utils.h>

#include <imfixes/cogscript_fixes.h>

#include <libim/common.h>
#include <libim/content/asset/animation/animation.h>
#include <libim/content/asset/cog/cog.h>
#include <libim/content/asset/cog/cogscript.h>
#include <libim/content/asset/cog/impl/grammer/parse_utils.h>
#include <libim/content/asset/material/material.h>
#include <libim/content/asset/material/texture.h>
#include <libim/content/asset/material/texture_view.h>
#include <libim/content/asset/material/texutils.h>
#include <libim/content/asset/world/impl/serialization/cnd/cnd.h>
#include <libim/content/asset/world/impl/serialization/ndy/ndy.h>
#include <libim/content/asset/world/impl/serialization/ndy/thing/ndy_thing_oser.h>
#include <libim/content/audio/soundbank.h>
#include <libim/content/text/text_resource_writer.h>
#include <libim/io/filestream.h>
#include <libim/io/vfs.h>
#include <libim/log/log.h>
#include <libim/types/safe_cast.h>

#include "config.h"
#include "cnd.h"
#include "cndtoolargs.h"
#include "ndy.h"
#include "obj.h"

using namespace cmdutils;
using namespace cndtool;
using namespace libim;
using namespace libim::content::audio;
using namespace libim::content::asset;
using namespace libim::content::text;

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace fs = std::filesystem;

static constexpr auto kTmplFilename = "ijim.tpl"sv;

static constexpr auto kExtCnd = ".cnd"sv;
static constexpr auto kExtKey = ".key"sv;
static constexpr auto kExtMat = ".mat"sv;
static constexpr auto kExtNdy = ".ndy"sv;

constexpr static auto cmdAdd     = "add"sv;
constexpr static auto cmdConvert = "convert"sv;
constexpr static auto cmdExtract = "extract"sv;
constexpr static auto cmdList    = "list"sv;
constexpr static auto cmdRemove  = "remove"sv;
constexpr static auto cmdHelp    = "help"sv;

constexpr static auto scmdAnimation = "animation"sv;
constexpr static auto scmdMaterial  = "material"sv;
constexpr static auto scmdCnd       = "cnd"sv;
constexpr static auto scmdNdy       = "ndy"sv;
constexpr static auto scmdObj       = "obj"sv;

constexpr static auto optAnimations            = "--key"sv;
constexpr static auto optExtractAsBmp          = "--mat-bmp"sv;
constexpr static auto optExtractAsBmpShort     = "-b"sv;
constexpr static auto optExtractLod            = "--mat-mipmap"sv;
constexpr static auto optMaxTex                = "--mat-max-tex"sv;
constexpr static auto optMaterials             = "--mat"sv;
constexpr static auto optNoAnimations          = "--no-key"sv;
constexpr static auto optNoCleanup             = "--no-cleanup"sv;
constexpr static auto optNoMaterials           = "--no-mat"sv;
constexpr static auto optNoSounds              = "--no-sound"sv;
constexpr static auto optNoTemplates           = "--no-template"sv;
constexpr static auto optOutputDir             = "--output-dir"sv;
constexpr static auto optOutputDirShort        = "-o"sv;
constexpr static auto optConvertToPng          = "--mat-png"sv;
constexpr static auto optConvertToPngShort     = "-p"sv;
constexpr static auto optReplace               = "--replace"sv;
constexpr static auto optReplaceShort          = "-r"sv;
constexpr static auto optSounds                = "--sound"sv;
constexpr static auto optSoundStartHandle      = "--sound-handle"sv;
constexpr static auto optSoundStartHandleShort = "-h"sv;
constexpr static auto optStatic                = "--static"sv;
constexpr static auto optStrict                = "--strict"sv;
constexpr static auto optVerbose               = "--verbose"sv;
constexpr static auto optVerboseShort          = "-v"sv;
constexpr static auto optConvertToWav          = "--sound-wav"sv;
constexpr static auto optConvertToWavShort     = "-w"sv;
constexpr static auto optExportSoundbank       = "--soundbank"sv;
constexpr static auto optOverwriteTemplates    = "--template-overwrite"sv;

struct ExtractOptions final
{
    bool verboseOutput = false;
    struct {
        bool extract = false;
    } key;

    struct
    {
        bool extract      = false;
        bool convertToBmp = false;
        bool convertToPng = false;
        std::optional<uint64_t> maxTex;
        bool convertMipMap = false;
    } mat;

    struct
    {
        bool extract         = false;
        bool convertToWav    = false;
        bool exportSoundbank = false;
    } sound;

    struct
    {
        bool extract = false;
        bool overwrite = false;
    } templates;
};

bool hasOptVerbose(const CndToolArgs& args)
{
    return args.hasArg(optVerbose) || args.hasArg(optVerboseShort);
}

bool hasOptReplace(const CndToolArgs& args)
{
    return args.hasArg(optReplace) || args.hasArg(optReplaceShort);
}

fs::path getOptOutputDir(const CndToolArgs& args, std::optional<fs::path> optPath = std::nullopt)
{
    if (args.hasArg(optOutputDirShort)){
        return args.arg(optOutputDirShort);
    }
    else if (args.hasArg(optOutputDir)){
        return args.arg(optOutputDir);
    }
    return optPath.value_or(fs::path());
}

std::vector<fs::path> getFilesFromPath(const fs::path path, std::string_view ext)
{
    std::vector<fs::path> files;
    files.reserve(17);
    if (isFilePath(path)) {
        files.push_back(path);
    }
    else // Scan folder for files
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            if (entry.is_regular_file())
            {
                const auto& path = entry.path();
                if (fileExtMatch(path, ext)) {
                    files.push_back(path);
                }
            }
        }
    }
    return files;
}

void printHelp(std::string_view cmd = "sv", std::string_view subcmd = ""sv)
{
    if (cmd == cmdAdd)
    {
        if (subcmd == scmdAnimation)
        {
            std::cout << "Add or replace existing animation assets in a CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add animation [options] <cnd-file-path> <animation-files>" << std::endl << std::endl;
            printOptionHeader();
            printOption( optReplace, optReplaceShort, "Replace existing animation asset" );
            printOption( optVerbose, optVerboseShort, "Verbose printout to the console"  );
        }
        else if (subcmd == scmdMaterial)
        {
            std::cout << "Add or replace existing material assets in a CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add material [options] <cnd-file-path> <material-files>" << std::endl << std::endl;
            printOptionHeader();
            printOption( optReplace, optReplaceShort, "Replace existing material asset" );
            printOption( optVerbose, optVerboseShort, "Verbose printout to the console" );
        }
        else
        {
            std::cout << "Add or replace existing assets in a CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add <sub-command> " << std::endl << std::endl;
            printSubCommandHeader();
            printSubCommand( scmdAnimation, "Add animation assets" );
            printSubCommand( scmdMaterial , "Add material assets"  );
        }
    }
    else if (cmd == cmdConvert)
    {
        if (subcmd == scmdCnd)
        {
            std::cout << "Convert NDY level file(s) to CND file format." << std::endl << std::endl;
            std::cout << "  Usage: cndtool convert cnd [options] <ndy-file-path|ndy-folder> <game-assets-folder>" << std::endl << std::endl;
            printOptionHeader();
            printOption( optSoundStartHandle , optSoundStartHandleShort , "Start sound handle."                                                       );
            printOption( ""                  , ""                       , "By default 349 for normal and 0 for static CND file.\n"                    );

            printOption( optNoCleanup        , ""                       , "Don't remove static game assets from jones3dstatic."                       );
            printOption( ""                  , ""                       , utils::format("Has no effect if % is set.\n", optStatic)                    );

            printOption( optStatic           , ""                       , "NDY file is static file, i.e. game assets container (jones3dstatic.cnd)."  );
            printOption( ""                  , ""                       , "This option is not supported for batch mode.\n"                            );

            printOption( optStrict           , ""                       , "Verify all required sections are set and valid.\n"                         );

            printOption( optOutputDir        , optOutputDirShort        , "Output folder"                                                             );
            printOption( optVerbose          , optVerboseShort          , "Verbose printout to the console"                                           );
        }
        else if (subcmd == scmdNdy)
        {
            std::cout << "Convert CND level file(s) to NDY file format." << std::endl << std::endl;
            std::cout << "  Usage: cndtool convert ndy [options] <cnd-file-path|cnd-folder> <cog-scripts-folder>" << std::endl << std::endl;
            printOptionHeader();
            printOption( optNoAnimations , ""               , "Don't extract animation assets"  );
            printOption( optNoMaterials  , ""               , "Don't extract material assets"   );
            printOption( optNoSounds     , ""               , "Don't extract sound assets"      );
            printOption( optOutputDir    , optOutputDirShort, "Output folder"                   );
            printOption( optVerbose      , optVerboseShort  , "Verbose printout to the console" );
        }
        else if (subcmd == scmdObj)
        {
            std::cout << "Extract level geometry from CND file and convert to Wavefront OBJ file format." << std::endl << std::endl;
            std::cout << "  Usage: cndtool convert obj [options] <cnd-file-path>" << std::endl << std::endl;
            printOptionHeader();
            printOption( optNoMaterials, ""               , "Don't extract material assets"   );
            printOption( optOutputDir  , optOutputDirShort, "Output folder"                   );
            printOption( optVerbose    , optVerboseShort  , "Verbose printout to the console" );
        }
        else
        {
            std::cout << "Convert CND level file to another format." << std::endl << std::endl;
            std::cout << "  Usage: cndtool convert <sub-command> " << std::endl << std::endl;
            printSubCommandHeader();
            printSubCommand( scmdCnd, "Convert NDY to CND file format." );
            printSubCommand( scmdNdy, "Convert CND to NDY file format." );
            printSubCommand( scmdObj, "Extract level geometry and convert to Wavefront OBJ file format." );
        }
    }
    else if (cmd == cmdExtract)
    {
        std::cout << "Extract animation [KEY], material [MAT], sound [IndyWV] and Thing template assets from CND level file(s)." << std::endl << std::endl;
        std::cout << "  Usage: cndtool extract [options] <cnd-file-path|cnd-folder>" << std::endl << std::endl;
        printOptionHeader();
        printOption( optExtractAsBmp       , optExtractAsBmpShort , "Convert extracted material assets to BMP format."                            );
        printOption( optConvertToPng       , optConvertToPngShort , "Convert extracted material assets to PNG format."                            );
        printOption( optMaxTex             , ""                   , "Max number of images to convert from each material file."                    );
        printOption( ""                    , ""                   , "By default all are converted."                                               );
        printOption( optExtractLod         , ""                   , "Extract also MipMap LOD images when converting material file.\n"             );

        printOption( optConvertToWav       , optConvertToWavShort , "If extracted sound is in IndyWV compressed format convert it to WAV format." );
        printOption( optExportSoundbank    , ""                   , "Export soundbank track to file.\n"                                            );

        printOption( optOverwriteTemplates , ""                   , "Overwrite any existing Thing templates."                                     );
        printOption( ""                    , ""                   , "By default only new templates are written if ijim.tpl file exists.\n"        );

        printOption( optNoAnimations       , ""                   , "Don't extract animation assets."                                             );
        printOption( optNoMaterials        , ""                   , "Don't extract material assets."                                              );
        printOption( optNoSounds           , ""                   , "Don't extract sound assets."                                                 );
        printOption( optNoTemplates        , ""                   , "Don't extract Thing templates.\n"                                            );

        printOption( optOutputDir          , optOutputDirShort    , "Output folder."                                                              );
        printOption( optVerbose            , optVerboseShort      , "Verbose printout to the console."                                            );
    }
    else if (cmd == cmdList)
    {
        std::cout << "List assets in a CND level file." << std::endl << std::endl;
        std::cout << "  Usage: cndtool list [options] <cnd-file-path>" << std::endl << std::endl;
        printOptionHeader();
        printOption( optAnimations, "", "List only animation assets" );
        printOption( optMaterials , "", "List only material assets"  );
        printOption( optSounds    , "", "List only sound assets"     );
    }
    else if (cmd == cmdRemove)
    {
        if (subcmd == scmdAnimation)
        {
            std::cout << "Remove animation assets from CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool remove animation [options] <cnd-file-path> <animation_name>.key ..." << std::endl << std::endl;
            printOptionHeader();
            printOption( optVerbose, optVerboseShort, "Verbose printout to the console" );
        }
        else if (subcmd == scmdMaterial)
        {
            std::cout << "Remove material assets from a CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool remove material [options] <cnd-file-path> <animation_name>.mat ..." << std::endl << std::endl;
            printOptionHeader();
            printOption( optVerbose, optVerboseShort, "Verbose printout to the console" );
        }
        else
        {
            std::cout << "Remove assets from CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool remove <sub-command> " << std::endl << std::endl;
            printSubCommandHeader();
            printSubCommand( scmdAnimation, "Remove animation assets" );
            printSubCommand( scmdMaterial , "Remove material assets"  );
        }
    }
    else
    {
        std::cout << "Command-line interface tool to extract and modify\ngame assets stored in a CND level file.\n\n";
        std::cout << "  Usage: cndtool <command> [sub-command] [options]" << std::endl << std::endl;
        printCommandHeader();
        printCommand( cmdAdd    , "Add or replace game assets"                       );
        printCommand( cmdConvert, "Convert CND file"                                 );
        printCommand( cmdExtract, "Extract game assets"                              );
        printCommand( cmdList   , "Print to the console stored game assets"          );
        printCommand( cmdRemove , "Remove one or more game assets"                   );
        printCommand( cmdHelp   , "Show this message or help for a specific command" );
    }
}

void printErrorInvalidCnd(const fs::path cndPath, std::string_view cmd, std::string_view subcmd = ""sv)
{
    if (cndPath.empty()) {
        printError("A valid CND file path required!\n");
    } else {
        printError("CND file % doesn't exist!\n", cndPath);
    }
    printHelp(cmd, subcmd);
}

template<typename AssetT, typename ResLoadF, typename CndReadF, typename CndWriteF>
bool execCmdAddAssets(const CndToolArgs& args, std::string_view assetFileExt, ResLoadF&& loadAsset, CndReadF&& cndReadAssets, CndWriteF&& cndWriteAssets)
{
    const bool bVerbose = hasOptVerbose(args);
    const fs::path cndFile = args.cndFile();
    if (!fileExists(cndFile))
    {
        printErrorInvalidCnd(cndFile, cmdAdd, args.subcmd());
        return false;
    }

    std::cout << "Patching CND file... "  << std::flush;

    std::vector<AssetT> newAssets;
    newAssets.reserve(args.positionalArgs().size());
    for (const auto& assetFile : args.positionalArgs())
    {
        try
        {
            const bool bValidFile  = fileExtMatch(assetFile, assetFileExt);
            const bool bFileExists = bValidFile && fileExists(assetFile);
            if (!bValidFile)
            {
                printError("Invalid file '%', expected file with an extension '%'!\n", assetFile, assetFileExt);
                printHelp(cmdAdd, args.subcmd());
                return false;
            }
            else if (!bFileExists)
            {
                printError("File '%' doesn't exist!\n", assetFile);
                printHelp(cmdAdd, args.subcmd());
                return false;
            }

            InputFileStream as(assetFile);
            newAssets.push_back(loadAsset(as));
        }
        catch (const std::exception& e)
        {
            printError("Failed to load asset file: '%'!\n", assetFile);
            if (bVerbose) {
                std::cerr << "Reason: " << e.what() << std::endl;
            }
        }
    }

    if (newAssets.empty())
    {
        printError("A valid '%' file path required!", assetFileExt);
        printHelp(cmdAdd, args.subcmd());
        return false;
    }

    try
    {
        InputFileStream ifstream(cndFile);
        auto mapAssets = cndReadAssets(ifstream);
        ifstream.close();

        for(auto&& asset : newAssets)
        {
            if (mapAssets.contains(asset.name()) && !hasOptReplace(args))
            {
                std::cerr << std::endl;
                printError("Asset '%' already exists in CND file and no '--replace' option was provided.\n       CND file was not modified!\n", asset.name());
                return false;
            }

            mapAssets[asset.name()] = std::move(asset);
        }

        const bool success = cndWriteAssets(cndFile, mapAssets);
        if (success) {
            std::cout << kSuccess << std::endl;
        }
        return success;
    }
    catch (const std::exception& e)
    {
        printError("Failed to patch CND file!\n       Reason: %\n", e.what());
        printHelp(cmdAdd, args.subcmd());
        return false;
    }
}

int execCmdAddAnimation(const CndToolArgs& args)
{
    bool success = execCmdAddAssets<Animation>(args, kExtKey,
        [](const auto& istream){
            return keyLoad(TextResourceReader(istream));
        },
        [](const auto& istream){
            return CND::readKeyframes(istream);
        },
        [](const auto& cndFile, const auto& animations){
            return patchCndAnimations(cndFile, animations);
        }
    );

    return int(!success);
}

int execCmdAddMaterial(const CndToolArgs& args)
{
    bool success = execCmdAddAssets<Material>(args, kExtMat,
        [](const auto& istream){
            return matLoad(istream);
        },
        [](const auto& istream){
            return CND::readMaterials(istream);
        },
        [](const auto& cndFile, const auto& materials){
            return patchCndMaterials(cndFile, materials);
        }
    );

    return int(!success);
}

int execCmdAdd(std::string_view scmd, const CndToolArgs& args)
{
    if (scmd == scmdAnimation) {
        return execCmdAddAnimation(args);
    }
    else if (scmd == scmdMaterial) {
        return execCmdAddMaterial(args);
    }

    if (scmd.empty()) {
        printError("Subcommand required!\n");
    }
    else {
        printError("Unknown subcommand '%'\n", scmd);
    }

    printHelp(cmdAdd);
    return 1;
}

void writeAnimations(const IndexMap<Animation>& animations, const fs::path& outDir, const std::string& cndName, const ExtractOptions& opt)
{
    if (animations.isEmpty()) return;

    auto keyDir = outDir / "key";
    makePath(keyDir);

    /* Save extracted animations to file */
    const std::vector<std::string> headerComments = [&]() {
        std::vector<std::string> cmt;
        cmt.emplace_back("Extracted from CND file '"s + cndName + "' with " +
            std::string(kProgramName) + " v" + std::string(kVersion)
        );
        cmt.emplace_back(kProgramUrl);
        return cmt;
    }();

    for(const auto[idx, anim] : enumerate(animations))
    {
        if (opt.verboseOutput){
            std::cout << "Extracting animation: " << anim.name() << std::endl;
        }
        else {
            printProgress("Extracting animations... ", idx, animations.size());
        }

        fs::path keyFilePath = keyDir / anim.name();
        OutputFileStream ofs(std::move(keyFilePath), /*truncate=*/true);
        keyWrite(anim, TextResourceWriter(ofs), headerComments);
    }

    if (!opt.verboseOutput) std::cout << "\rExtracting animations... " << kSuccess << std::endl;
}

void writeMaterials(const IndexMap<Material>& materials, const fs::path& outDir, const ExtractOptions& opt)
{
    if (materials.isEmpty()) return;

    const fs::path matDir = outDir / "mat";
    makePath(matDir);

    fs::path bmpDir;
    if (opt.mat.convertToBmp)
    {
        bmpDir = outDir / "bmp";
        makePath(bmpDir);
    }

    fs::path pngDir;
    if (opt.mat.convertToPng)
    {
        pngDir = outDir / "png";
        makePath(pngDir);
    }

    /* Save extracted materials to persistent storage */
    const bool convert = opt.mat.convertToBmp || opt.mat.convertToPng;
    if (convert && opt.mat.maxTex && *opt.mat.maxTex == 0) {
        std::cout << "Warning: Materials won't be converted because option '" << optMaxTex << "' is 0!\n";
    }

    for (const auto[idx, mat] : enumerate(materials))
    {
        if (opt.verboseOutput)
        {
            std::cout << "\nExtracting material: " <<  mat.name() << std::endl;
            matool::matPrintInfo(mat);
        }
        else {
            printProgress("Extracting materials... ", idx + 1, materials.size());
        }

        /* Save MAT file to disk */
        const auto outMatFile = matDir / mat.name();
        matWrite(mat, OutputFileStream(outMatFile, /*truncate=*/true));

        /* Extract images from MAT file and save them to disk */
        if (convert)
        {
            if (opt.mat.convertToBmp) {
                matool::matExtractImages(mat, bmpDir, opt.mat.maxTex, opt.mat.convertMipMap, /*extractAsBmp*/true);
            }
            if (opt.mat.convertToPng) {
                matool::matExtractImages(mat, pngDir, opt.mat.maxTex, opt.mat.convertMipMap);
            }
        }
    }

    if (!opt.verboseOutput) std::cout << "\rExtracting materials... " << kSuccess << std::endl;
}

void writeSounds(const IndexMap<Sound>& sounds, const fs::path& outDir, const ExtractOptions& opt)
{
    if (sounds.isEmpty()) return;
    if (!opt.verboseOutput) printProgress("Extracting sounds... ", 0, 1);

    fs::path outPath = outDir / "sound";
    makePath(outPath);

    fs::path wavDir;
    if (opt.sound.convertToWav)
    {
        wavDir = outDir / "wav";
        makePath(wavDir);
    }

    for (const auto[idx, s] : enumerate(sounds))
    {
        if (opt.verboseOutput) {
            std::cout << "\rExtracting sound: " << s.name() << std::endl;
        }
        else {
            printProgress("Extracting sounds... ", idx + 1, sounds.size());
        }

        if (s.isCompressed())
        {
            OutputFileStream ofs(outPath.append(s.name()), /*truncate=*/true);
            wvWrite(ofs, s);
            outPath = outPath.parent_path();
        }

        /* Save in WAV format */
        if (!s.isCompressed() || opt.sound.convertToWav)
        {
            OutputFileStream ofs(wavDir.append(s.name()), /*truncate=*/true);
            wavWrite(ofs, s);
            wavDir = wavDir.parent_path();
        }
    }

    if (!opt.verboseOutput) std::cout << "\rExtracting sounds... " << kSuccess << std::endl;
}

std::size_t writeTemplates(const IndexMap<CndThing>& templates, const fs::path& outDir, const ExtractOptions& opt)
{
    if (templates.isEmpty()) return 0;

    makePath(outDir);
    const auto outPath = outDir / kTmplFilename;

    // Read any existing template file
    IndexMap<CndThing> outTemplates;
    if (fileExists(outPath))
    {
        LOG_DEBUG("Found existing file: % , reading existing template(s) ...", outPath);
        InputFileStream ifs(outPath);
        outTemplates = NDY::parseTemplateList(TextResourceReader(ifs));
        LOG_DEBUG("Read % existing templates.", outTemplates.size());
        LOG_DEBUG("Merging existing template(s) with new template(s). overwrite=%", opt.templates.overwrite);
    }

    std::size_t newTemplates = 0;
    for (const auto& [key, tmpl] : templates.container())
    {
        if (!outTemplates.contains(key) || opt.templates.overwrite)
         {
            outTemplates[key] = tmpl;
            newTemplates++;
        }
    }

    if (newTemplates == 0)
    {
        LOG_DEBUG("No new templates found. Skipping writing of template file.");
        return newTemplates;
    }

    // Write templates to file
    LOG_DEBUG("Writing % template(s) to file. New or overwritten template(s): %", outTemplates.size(), newTemplates);
    OutputFileStream ofs(outPath, /*truncate=*/true);
    writeTemplateList</*writeEnd=*/false>(TextResourceWriter(ofs), outTemplates, /*writeHeader=*/false);
    return newTemplates;
}

std::size_t extractAnimations(const InputStream& istream, const fs::path& outDir, const ExtractOptions& opt)
{
    if (!opt.key.extract) {
        return 0;
    }

    if (!opt.verboseOutput) printProgress("Extracting animations... ", 1, 0);
    auto animations = CND::readKeyframes(istream);
    if (!animations.isEmpty())
    {
        if (opt.verboseOutput) {
            std::cout << "\nFound: " << animations.size() << std::endl;
        }
        writeAnimations(animations, outDir, istream.name(), opt);
    }
    return animations.size();
}

std::size_t extractMaterials(const InputStream& istream, const fs::path& outDir, const ExtractOptions& opt)
{
    if (!opt.mat.extract) {
        return 0;
    }

    if (!opt.verboseOutput) printProgress("Extracting materials... ", 0, 1);

    auto materials = CND::readMaterials(istream);
    if (!materials.isEmpty())
    {
        if (opt.verboseOutput) {
            std:: cout << " Found: " << materials.size() << std::endl;
        }
        writeMaterials(materials, outDir, opt);
    }
    return materials.size();
}

std::size_t extractSounds(const InputStream& istream, const fs::path& outDir, const ExtractOptions& opt)
{
    if (!opt.sound.extract) {
        return 0;
    }

    if (!opt.verboseOutput) printProgress("Extracting sounds... ", 0, 1);

    // Parse sounds
    const auto header = CND::readHeader(istream);
    const std::size_t sbtIdx = getSoundBankTrackIdx(header.state & CndWorldState::Static);

    SoundBank sb(sbtIdx + 1);
    sb.setStaticTrack(sbtIdx, header.state & CndWorldState::Static); // Never forget this or the exported track sounds will have incorrect indices.
    CND::readSounds(istream, sb, sbtIdx); // Import to correct track idx so the original soundbank num is preserved

    if (opt.sound.exportSoundbank)
    {
        auto bankPath = outDir / (getBaseName(istream.name()) + "_soundbank.bin");
        LOG_DEBUG("Exporting soundbank track % to file: %", sbtIdx, bankPath);
        sb.exportTrack(sbtIdx, OutputFileStream(bankPath, /*truncate=*/true));
    }

    auto& sounds = sb.getTrack(sbtIdx);
    if (!sounds.isEmpty())
    {
        if (opt.verboseOutput) {
            std::cout << "\nFound: " << sounds.size() << std::endl;
        }
        writeSounds(sounds, outDir, opt);
    }
    return sounds.size();
}

std::size_t extractTemplates(const InputStream& istream, const fs::path& outDir, const ExtractOptions& opt)
{
    if (!opt.templates.extract) {
        return 0;
    }

    auto templates = CND::readTemplates(istream);
    LOG_DEBUG("Thing template(s) to extract: %", templates.size());

    std::size_t numWritten = 0;
    if (!templates.isEmpty())
    {
        if (!opt.verboseOutput) std::cout << "\rExtracting templates... " << std::flush;
        numWritten = writeTemplates(templates, outDir, opt);
        if (!opt.verboseOutput) std::cout << "\rExtracting templates... " << kSuccess << std::endl;
    }
    return numWritten;
}

void extractAssets(const fs::path& cndFile, const fs::path& outDir, const ExtractOptions& opt)
{
    if (!opt.key.extract   &&
        !opt.mat.extract   &&
        !opt.sound.extract &&
        !opt.templates.extract)
    {
        std::cout << "Nothing to be done!\n";
        return;
    }

    InputFileStream ifstream(cndFile);

    auto nExtAnimFiles = extractAnimations(ifstream, outDir, opt);
    auto nExtMatFiles  = extractMaterials(ifstream, outDir, opt);
    auto nExtSndFiles  = extractSounds(ifstream, outDir, opt);
    auto nExtTemplates = extractTemplates(ifstream, outDir, opt);

    std::cout << "\n-------------------------------------\n";
    if (opt.key.extract) {
        std::cout << "Total extracted animations: " << nExtAnimFiles << std::endl;
    }
    if (opt.mat.extract) {
        std::cout << "Total extracted materials:  " << nExtMatFiles << std::endl;
    }
    if (opt.sound.extract) {
        std::cout << "Total extracted sounds:     " << nExtSndFiles << std::endl;
    }
    if (opt.templates.extract) {
        std::cout << "Total extracted templates:  " << nExtTemplates << std::endl;
    }
}

int execCmdExtract(const CndToolArgs& args)
{
    try
    {
        fs::path input = args.cndFile();
        if (input.empty()) {
            input = !args.subcmd().empty()
                ? fs::path(args.subcmd())
                : !args.positionalArgs().empty()
                    ? fs::path(args.positionalArgs().at(0))
                    : fs::path();
        }

        if (input.empty() || (!fileExists(input) && !dirExists(input)))
        {
            printError("Invalid positional argument for input CND file path or folder path!\n");
            printHelp(cmdExtract);
            return 1;
        }

        if (!isDirPath(getOptOutputDir(args)))
        {
            printError("Output path is not directory!\n");
            return 1;
        }

        std::vector<fs::path> cndFiles = getFilesFromPath(input, kExtCnd);
        if (cndFiles.empty())
        {
            printError("No CND file found!\n");
            return 1;
        }

        ExtractOptions opt;
        opt.verboseOutput      = hasOptVerbose(args);
        opt.key.extract        = !args.hasArg(optNoAnimations);
        opt.mat.extract        = !args.hasArg(optNoMaterials);

        opt.mat.convertToBmp   = args.hasArg(optExtractAsBmpShort) || args.hasArg(optExtractAsBmp);
        opt.mat.convertToPng   = args.hasArg(optConvertToPngShort) || args.hasArg(optConvertToPng);
        opt.mat.convertMipMap  = args.hasArg(optExtractLod);

        opt.sound.extract         = !args.hasArg(optNoSounds);
        opt.sound.convertToWav    = args.hasArg(optConvertToWavShort) || args.hasArg(optConvertToWav);
        opt.sound.exportSoundbank = args.hasArg(optExportSoundbank);

        opt.templates.extract   = !args.hasArg(optNoTemplates);
        opt.templates.overwrite = args.hasArg(optOverwriteTemplates);

        if (args.hasArg(optMaxTex)) {
            opt.mat.maxTex = args.uintArg(optMaxTex);
        }

        /* Extract animations, materials & sounds */
        for (const auto& cndFile : cndFiles)
        {
            if (cndFiles.size() > 1) std::cout << "\nExtracting assets from: " << cndFile.filename().string() << std::endl;
            auto outDir = getOptOutputDir(args, cndFile.stem());
            makePath(outDir);
            extractAssets(cndFile, outDir, opt);
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << std::endl;
        printError("Failed to extract assets from CND file!");
        std::cerr << "       Reason: " << e.what() << std::endl;
        return 1;
    }
}

int execSubCmdConvertToCnd(const CndToolArgs& args)
{
    try
    {
        // Get NDY file path or folder path
        fs::path ndyPath = args.ndyFile();
        if (ndyPath.empty())
        {
            if (!args.cndFile().empty())
            {
                printError("Positional argument is CND file but NDY file is required!\n");
                return 1;
            }

            // Get folder
            ndyPath = !args.subcmd().empty() && args.positionalArgs().size() > 0
                ? fs::path(args.positionalArgs().at(0))
                : args.positionalArgs().size() > 1
                    ? fs::path(args.positionalArgs().at(1))
                    : fs::path();

        }

        if (ndyPath.empty() || (!fileExists(ndyPath) && !dirExists(ndyPath)))
        {
            printError("Invalid positional argument for input NDY file path or folder path!\n");
            printHelp(cmdConvert, scmdCnd);
            return 1;
        }

        std::vector<fs::path> ndyFiles = getFilesFromPath(ndyPath, kExtNdy);
        if (ndyFiles.empty())
        {
            if (fileExists(ndyPath)) {
                printError("Positional argument is not a valid NDY file!\n");
            }
            else {
                printError("No NDY file(s) found in specified folder!\n");
            }
            return 1;
        }

        if (args.positionalArgs().empty())
        {
            printError("Missing positional argument for game assets folder!\n");
            return 1;
        }

        fs::path resourceDir = args.positionalArgs().at(0);
        if (args.ndyFile().empty()) {
            resourceDir = !args.subcmd().empty() && args.positionalArgs().size() > 1
                ? fs::path(args.positionalArgs().at(1))
                : args.positionalArgs().size() > 2
                    ? fs::path(args.positionalArgs().at(2))
                    : fs::path();
        }

        if (!isDirPath(resourceDir) || !dirExists(resourceDir))
        {
            printError("Game assets path is not directory!\n");
            return 1;
        }

        if (!isDirPath(getOptOutputDir(args)))
        {
            printError("Output path is not directory!\n");
            return 1;
        }

        VirtualFileSystem vfs;
        vfs.addSysFolder(resourceDir);
        if (!vfs.tryLoadGobContainer(resourceDir / "cd1.gob")) {
            vfs.tryLoadGobContainer(resourceDir / "Resource" / "cd1.gob");
        }

        if (!vfs.tryLoadGobContainer(resourceDir / "cd2.gob")) {
            vfs.tryLoadGobContainer(resourceDir / "Resource" / "cd2.gob");
        }

        const bool staticCnd = args.hasArg(optStatic);
        if (ndyFiles.size() > 1 && staticCnd)
        {
            printError("Option '%' can't be used in batch mode!", optStatic);
            return 1;
        }

        const bool verify  = args.hasArg(optStrict);
        const bool cleanUp = !args.hasArg(optNoCleanup);

        SoundHandle sndStartHandle = getDefaultStartSoundHandle(staticCnd);
        if (args.hasArg(optSoundStartHandle)){
            sndStartHandle = SoundHandle(args.uintArg(optSoundStartHandle));
        }
        else if (args.hasArg(optSoundStartHandleShort)){
            sndStartHandle = SoundHandle(args.uintArg(optSoundStartHandleShort));
        }

        // Init static resources
        StaticResourceNames staticResources;
        staticResources.setDefault();

        /* Extract animations, materials & sounds */
        for (const auto& ndyFile : ndyFiles)
        {
            if (ndyFiles.size() > 1) std::cout << "\nConverting to CND: " << ndyFile.filename().string() << std::endl;
            auto ndyOutDir = getOptOutputDir(args, ndyFile.stem());
            makePath(ndyOutDir);
            convertNdyToCnd(ndyFile, vfs, staticResources, ndyOutDir, sndStartHandle, staticCnd, verify, cleanUp, hasOptVerbose(args));
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << std::endl;
        printError("Failed to convert NDY file(s) to CND format!");
        std::cerr << "       Reason: " << e.what() << std::endl;
        return 1;
    }
}

int execSubCmdConvertToNdy(const CndToolArgs& args)
{
    try
    {
        fs::path cndPath = args.cndFile();
        if (cndPath.empty())
        {
            if (!args.ndyFile().empty())
            {
                printError("Positional argument is NDY file but CND file is required!\n");
                return 1;
            }

            // Get folder path
            cndPath = !args.subcmd().empty() && args.positionalArgs().size() > 0
                ? fs::path(args.positionalArgs().at(0))
                : args.positionalArgs().size() > 1
                    ? fs::path(args.positionalArgs().at(1))
                    : fs::path();
        }

        if (cndPath.empty() || (!fileExists(cndPath) && !dirExists(cndPath)))
        {
            printError("Invalid positional argument for input CND file path or folder path!\n");
            printHelp(cmdConvert, scmdNdy);
            return 1;
        }

        std::vector<fs::path> cndFiles = getFilesFromPath(cndPath, kExtCnd);
        if (cndFiles.empty())
        {
            if (isFilePath(cndPath)) {
                printError("Positional argument is not a valid CND file!\n");
            }
            else {
                printError("No CND file(s) found in specified folder!\n");
            }
            return 1;
        }

        if (args.positionalArgs().empty())
        {
            printError("Missing positional argument for COG scripts folder!\n");
            return 1;
        }

        fs::path resourceDir = args.positionalArgs().at(0);
        if (args.cndFile().empty()) {
            resourceDir = !args.subcmd().empty() && args.positionalArgs().size() > 1
                ? fs::path(args.positionalArgs().at(1))
                : args.positionalArgs().size() > 2
                    ? fs::path(args.positionalArgs().at(2))
                    : fs::path();
        }

        if (!isDirPath(resourceDir) || !dirExists(resourceDir))
        {
            printError("COG scripts path is not directory!\n");
            return 1;
        }

        if (!isDirPath(getOptOutputDir(args)))
        {
            printError("Output path is not directory!\n");
            return 1;
        }

        VirtualFileSystem vfs;
        vfs.addSysFolder(resourceDir);
        if (!vfs.tryLoadGobContainer(resourceDir / "cd1.gob")) {
            vfs.tryLoadGobContainer(resourceDir / "Resource" / "cd1.gob");
        }

        if (!vfs.tryLoadGobContainer(resourceDir / "cd2.gob")) {
            vfs.tryLoadGobContainer(resourceDir / "Resource" / "cd2.gob");
        }

        ExtractOptions eopt;
        eopt.verboseOutput     = hasOptVerbose(args);
        eopt.key.extract       = !args.hasArg(optNoAnimations);
        eopt.mat.extract       = !args.hasArg(optNoMaterials);
        eopt.sound.extract     = !args.hasArg(optNoSounds);
        eopt.templates.extract = false;

        bool bExtractAssets = eopt.key.extract || eopt.mat.extract || eopt.sound.extract;

        /* Extract animations, materials & sounds */
        for (const auto& cndFile : cndFiles)
        {
            if (cndFiles.size() > 1) std::cout << "\nConverting to NDY: " << cndFile.filename().string() << std::endl;
            auto ndyOutDir = getOptOutputDir(args, cndFile.stem());
            makePath(ndyOutDir);
            if (convertCndToNdy(cndFile, vfs, ndyOutDir, eopt.verboseOutput) && bExtractAssets)
            {
                LOG_DEBUG("Extracting assets key:% mat:% sound:%", eopt.key.extract, eopt.mat.extract, eopt.sound.extract);
                extractAssets(cndFile, ndyOutDir, eopt);
            }
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << std::endl;
        printError("Failed to convert CND file(s) to NDY format!");
        std::cerr << "       Reason: " << e.what() << std::endl;
        return 1;
    }
}

int execSubCmdConvertToObj(const CndToolArgs& args)
{
    try
    {
        const fs::path inputFile = args.cndFile();
        if (!fileExists(inputFile))
        {
            printErrorInvalidCnd(inputFile, cmdConvert, args.subcmd());
            return 1;
        }

        fs::path outDir;
        if (args.hasArg(optOutputDirShort)){
            outDir = args.arg(optOutputDirShort);
        }
        else if (args.hasArg(optOutputDir)){
            outDir = args.arg(optOutputDir);
        }
        else {
            outDir = getBaseName(inputFile.string());
        }

        if (!isDirPath(outDir))
        {
            printError("Output path is not directory!\n");
            return 1;
        }

        // Init static resources
        StaticResourceNames staticResources;
        staticResources.setDefault();

        std::cout << "Converting level geometry to OBJ ... " << std::flush;
        convertCndToObj(inputFile, staticResources, outDir, !args.hasArg(optNoMaterials));
        std::cout << kSuccess << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << std::endl;
        printError("Failed to convert level geometry to OBJ file format!");
        if (hasOptVerbose(args)) {
            std::cerr << "  Reason: " << e.what() << std::endl;
        }
        return 1;
    }
}

int execCmdConvert(std::string_view scmd, const CndToolArgs& args)
{
    if (scmd == scmdCnd) {
        return execSubCmdConvertToCnd(args);
    }
    else if (scmd == scmdNdy) {
        return execSubCmdConvertToNdy(args);
    }
    else if (scmd == scmdObj) {
        return execSubCmdConvertToObj(args);
    }

    if (scmd.empty()) {
        printError("Subcommand required!\n");
    }
    else {
        printError("Unknown subcommand '%'\n", scmd);
    }

    printHelp(cmdConvert);
    return 1;
}

int execCmdList(const CndToolArgs& args)
{
    const fs::path cndFile = args.cndFile();
    if (!fileExists(cndFile))
    {
        printErrorInvalidCnd(cndFile, cmdList);
        return 1;
    }

    bool listAnim = args.hasArg(optAnimations);
    bool listMat  = args.hasArg(optMaterials);
    bool listSnd  = args.hasArg(optSounds);

    if (!listAnim && !listMat && !listSnd) {
        listAnim = listMat = listSnd = true;
    }

    auto printList = [](const auto& l) {
        for(std::size_t i = 0; i < l.size(); i++) {
            std::cout << "  " << i << ": " << l.value(i).name() << std::endl;
        }
        std::cout << std::endl;
    };

    InputFileStream istream(cndFile);
    if (listAnim)
    {
        auto anims = CND::readKeyframes(istream);
        std::cout << "Animations:\n";
        printList(anims);
    }

    if (listMat)
    {
        auto mats = CND::readMaterials(istream);
        std::cout << "Materials:\n";
        printList(mats);
    }

    if (listSnd)
    {
        SoundBank sb(1);
        sb.importTrack(0, istream);
        auto& sounds = sb.getTrack(0);

        std::cout << "Sounds:\n";
        std::size_t i = 0;
        for(const auto& s : sounds)
        {
            std::cout << "  " << i++ << ": " << s.name() << std::endl;
                      // << SETW(38 - p.first.size(), ' ')
                      // << " | handle: " << s.handle() << " idx: " << s.idx() << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}

template<typename AssetT, typename CndReadF, typename CndWriteF>
bool execCmdRemoveAssets(const CndToolArgs& args, CndReadF&& cndReadAssets, CndWriteF&& cndWriteAssets)
{
    //const bool bVerbose = hasOptVerbose(args);
    const fs::path cndFile = args.cndFile();
    if (!fileExists(cndFile))
    {
        printErrorInvalidCnd(cndFile, cmdRemove, args.subcmd());
        return false;
    }

    try
    {
        std::cout << "Patching CND file... " << std::flush;
        InputFileStream ifstream(cndFile);
        auto mapAssets = cndReadAssets(ifstream);
        ifstream.close();

        for(const auto& asset : args.positionalArgs())
        {
            if (mapAssets.contains(asset)) {
                mapAssets.erase(asset);
            }
            else {
                std::cout << "WARNING: CND file doesn't contain asset: '" << asset << "'\n";
            }
        }

        const bool success = cndWriteAssets(cndFile, mapAssets);
        if (success) {
            std::cout << kSuccess << std::endl;
        }
        return success;
    }
    catch (const std::exception& e)
    {
        printError("Failed to patch CND file!\n       Reason: %\n", e.what());
        return false;
    }
}

int execCmdRemoveAnimation(const CndToolArgs& args)
{
    bool success = execCmdRemoveAssets<Animation>(args,
        [](const auto& istream){
            return CND::readKeyframes(istream);
        },
        [](const auto& cndFile, const auto& animations){
            return patchCndAnimations(cndFile, animations);
        }
    );
    return int(!success);
}

int execCmdRemoveMaterial(const CndToolArgs& args)
{
    bool success = execCmdRemoveAssets<Material>(args,
        [](const auto& istream){
            return CND::readMaterials(istream);
        },
        [](const auto& cndFile, const auto& materials){
            return patchCndMaterials(cndFile, materials);
        }
    );
    return int(!success);
}

int execCmdRemove(std::string_view scmd, const CndToolArgs& args)
{
    if (scmd == scmdAnimation) {
        return execCmdRemoveAnimation(args);
    }
    else if (scmd == scmdMaterial) {
        return execCmdRemoveMaterial(args);
    }

    if (scmd.empty()) {
        printError("Subcommand required!\n");
    }
    else {
        printError("Unknown subcommand '%'\n", scmd);
    }

    printHelp(cmdRemove);
    return 1;
}

int execCmd(std::string_view cmd, const CndToolArgs& args)
{

    if (cmd == cmdAdd) {
        return execCmdAdd(args.subcmd(), args);
    }
    else if (cmd == cmdConvert) {
        return execCmdConvert(args.subcmd(), args);
    }
    else if (cmd == cmdExtract) {
        return execCmdExtract(args);
    }
    else if (cmd == cmdList) {
        return execCmdList(args);
    }
    else if (cmd == cmdRemove) {
        return execCmdRemove(args.subcmd(), args);
    }
    else
    {
        if (cmd != cmdHelp) {
            printError("Unknown command '%'\n", cmd);
        }

        auto scmd = args.positionalArgs().empty() ? "" : args.positionalArgs().at(0);
        printHelp(args.subcmd(), scmd);
    }

    return 1;
}

int main(int argc, const char* argv[])
{
    std::cout << "\nIndiana Jones and the Infernal Machine CND file tool v" << kVersion << std::endl;
    std::cout << kProgramUrl << std::endl << std::endl;

    CndToolArgs args(argc, argv);
    if (argc < 2 ||
       args.cmd().empty())
    {
        printHelp();
        return 1;
    }

    gLogLevel = LogLevel::Warning;
    if (hasOptVerbose(args)) {
        gLogLevel = LogLevel::Verbose;
    }

    return execCmd(args.cmd(), args);
}
