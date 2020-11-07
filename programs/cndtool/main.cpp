#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

#include <cmdutils/cmdutils.h>
#include <matool/utils.h>

#include <libim/common.h>
#include <libim/content/asset/animation/animation.h>
#include <libim/content/asset/material/material.h>
#include <libim/content/asset/material/texture.h>
#include <libim/content/asset/material/texture_view.h>
#include <libim/content/asset/material/texutils.h>
#include <libim/content/asset/world/impl/serialization/cnd/cnd.h>
#include <libim/content/audio/soundbank.h>
#include <libim/content/text/text_resource_writer.h>
#include <libim/io/filestream.h>
#include <libim/log/log.h>
#include <libim/types/safe_cast.h>

#include "config.h"
#include "cndtoolargs.h"
#include "obj.h"
#include "patch.h"

using namespace cmdutils;
using namespace cndtool;
using namespace libim;
using namespace libim::content::audio;
using namespace libim::content::asset;
using namespace libim::content::text;

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace fs = std::filesystem;

static constexpr auto kExtKey = ".key"sv;
static constexpr auto kExtMat = ".mat"sv;

constexpr static auto cmdAdd     = "add"sv;
constexpr static auto cmdConvert = "convert"sv;
constexpr static auto cmdExtract = "extract"sv;
constexpr static auto cmdList    = "list"sv;
constexpr static auto cmdRemove  = "remove"sv;
constexpr static auto cmdHelp    = "help"sv;

constexpr static auto scmdAnimation = "animation"sv;
constexpr static auto scmdMaterial  = "material"sv;
constexpr static auto scmdObj       = "obj"sv;

constexpr static auto optAnimations        = "--key"sv;
constexpr static auto optExtractAsBmp      = "--mat-bmp"sv;
constexpr static auto optExtractAsBmpShort = "-b"sv;
constexpr static auto optExtractLod        = "--mat-mipmap"sv;
constexpr static auto optMaxTex            = "--mat-max-tex"sv;
constexpr static auto optMaterials         = "--mat"sv;
constexpr static auto optNoAnimations      = "--no-key"sv;
constexpr static auto optNoMaterials       = "--no-mat"sv;
constexpr static auto optNoSounds          = "--no-sound"sv;
constexpr static auto optOutputDir         = "--output-dir"sv;
constexpr static auto optOutputDirShort    = "-o"sv;
constexpr static auto optConvertToPng      = "--mat-png"sv;
constexpr static auto optConvertToPngShort = "-p"sv;
constexpr static auto optReplace           = "--replace"sv;
constexpr static auto optReplaceShort      = "-r"sv;
constexpr static auto optSounds            = "--sound"sv;
constexpr static auto optVerbose           = "--verbose"sv;
constexpr static auto optVerboseShort      = "-v"sv;
constexpr static auto optConvertToWav      = "--sound-wav"sv;
constexpr static auto optConvertToWavShort = "-w"sv;


[[maybe_unused]] constexpr static auto kFailed  = "FAILED"sv;
constexpr static auto kSuccess = "SUCCESS"sv;

struct ExtractOptions final
{
    bool verboseOutput;
    struct {
        bool extract;
    } key;

    struct
    {
        bool extract;
        bool convertToBmp;
        bool convertToPng;
        std::optional<uint64_t> maxTex;
        bool convertMipMap;
    } mat;

    struct
    {
        bool extract;
        bool convertToWav;
    } sound;
};

bool hasOptVerbose(const CndToolArgs& args)
{
    return args.hasArg(optVerbose) || args.hasArg(optVerboseShort);
}

bool hasOptReplace(const CndToolArgs& args)
{
    return args.hasArg(optReplace) || args.hasArg(optReplaceShort);
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
        if (subcmd == scmdObj)
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
            printSubCommand( scmdObj, "Extract level geometry and convert to Wavefront OBJ file format." );
        }
    }
    else if (cmd == cmdExtract)
    {
        std::cout << "Extract animation [KEY], material [MAT] and sound [IndyWV] assets from CND level file." << std::endl << std::endl;
        std::cout << "  Usage: cndtool extract [options] <cnd-file-path>" << std::endl << std::endl;
        printOptionHeader();
        printOption( optExtractAsBmp, optExtractAsBmpShort, "Convert extracted material assets to BMP format."                );
        printOption( optConvertToPng, optConvertToPngShort, "Convert extracted material assets to PNG format."                );
        printOption( optMaxTex      , ""                  , "Max number of images to convert from each material file."        );
        printOption( ""             , ""                  , "By default all are converted."                                   );
        printOption( optExtractLod  , ""                  , "Extract also mipmap LOD images when converting material file.\n" );
        printOption( optConvertToWav, optConvertToWavShort, "Convert extracted IndyWV sound assets to WAV format\n"           );
        printOption( optNoAnimations, ""                  , "Don't extract animation assets."                                 );
        printOption( optNoMaterials , ""                  , "Don't extract material assets."                                  );
        printOption( optNoSounds    , ""                  , "Don't extract sound assets.\n"                                   );
        printOption( optOutputDir   , optOutputDirShort   , "Output folder"                                                   );
        printOption( optVerbose     , optVerboseShort     , "Verbose printout to the console"                                 );
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
        std::cout << "\nCommand-line interface tool to extract and modify\ngame assets stored in a CND level file.\n\n";
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
            return Animation(TextResourceReader(istream));
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
            return Material(istream);
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
        printError("Unknown subcommand '%'!\n", scmd);
    }

    printHelp(cmdAdd);
    return 1;
}

int execCmdConvertToObj(const CndToolArgs& args)
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
            outDir = getBaseName(inputFile.u8string());
        }

        if (!isDirPath(outDir))
        {
            printError("Output path is not directory!\n");
            return 1;
        }

        std::cout << "Converting level geometry to OBJ ... " << std::flush;
        convertCndToObj(inputFile, outDir, !args.hasArg(optNoMaterials));
        std::cout << kSuccess << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << std::endl;
        printError("Failed to convert level geometry to OBJ file format!");
        if (hasOptVerbose(args)) {
            std::cerr << "       Reason: " << e.what() << std::endl;
        }
        return 1;
    }
}

int execCmdConvert(std::string_view scmd, const CndToolArgs& args)
{
    if (scmd == scmdObj) {
        return execCmdConvertToObj(args);
    }

    if (scmd.empty()) {
        printError("Subcommand required!\n");
    }
    else {
        printError("Unknown subcommand '%'!\n", scmd);
    }

    printHelp(cmdConvert);
    return 1;
}

std::size_t extractAnimations(const InputStream& istream, const fs::path& outDir, const ExtractOptions& opt)
{
    if (!opt.key.extract) {
        return 0;
    }

    if (!opt.verboseOutput) printProgress("Extracting animations... ", 1, 0);
    auto mapAnimations = CND::readKeyframes(istream);

    fs::path keyDir;
    if (!mapAnimations.isEmpty())
    {
        if (opt.verboseOutput) {
            std::cout << "\nFound: " << mapAnimations.size() << std::endl;
        }

        keyDir = outDir / "key";
        makePath(keyDir);
    }

    /* Save extracted animations to file */
    const std::string keyHeaderComment = [&]() {
        return "Extracted from CND file '"s + istream.name() + "' with " +
            std::string(kProgramName) + " v" + std::string(kVersion);
    }();

    for(const auto[idx, key] : enumerate(mapAnimations))
    {
        if (opt.verboseOutput){
            std::cout << "Extracting animation: " << key.name() << std::endl;
        }
        else {
            printProgress("Extracting animations... ", idx, mapAnimations.size());
        }

        fs::path keyFilePath = keyDir / key.name();
        OutputFileStream ofs(std::move(keyFilePath), /*truncate=*/true);
        key.serialize(TextResourceWriter(ofs), keyHeaderComment);
    }

    if (!opt.verboseOutput) std::cout << "\rExtracting animations... " << kSuccess << std::endl;
    return mapAnimations.size();
}

std::size_t extractMaterials(const InputStream& istream, const fs::path& outDir, const ExtractOptions& opt)
{
    if (!opt.mat.extract) {
        return 0;
    }

    if (!opt.verboseOutput) printProgress("Extracting materials... ", 0, 1);
    auto materials = CND::readMaterials(istream);

    fs::path matDir;
    fs::path bmpDir;
    fs::path pngDir;
    if (!materials.isEmpty())
    {
        if (opt.verboseOutput) {
            std:: cout << " Found: " << materials.size() << std::endl;
        }

        matDir = outDir / "mat";
        makePath(matDir);

        if (opt.mat.convertToBmp)
        {
            bmpDir = outDir / "bmp";
            makePath(bmpDir);
        }

        if (opt.mat.convertToPng)
        {
            pngDir = outDir / "png";
            makePath(pngDir);
        }
    }

    /* Save extracted materials to persistent storage */
    const bool convert    = opt.mat.convertToBmp || opt.mat.convertToPng;
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
        mat.serialize(OutputFileStream(outMatFile, /*truncate=*/true));

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
    return materials.size();
}

std::size_t extractSounds(const InputStream& istream, const fs::path& outDir, const ExtractOptions& opt)
{
    if (!opt.sound.extract) {
        return 0;
    }

    if (!opt.verboseOutput) printProgress("Extracting sounds... ", 0, 1);
    SoundBank sb(2);
    sb.importTrack(0, istream);
    auto& sounds = sb.getTrack(0);

    fs::path outPath;
    fs::path wavDir;

    if (!sounds.isEmpty())
    {
        if (opt.verboseOutput) {
            std::cout << "\nFound: " << sounds.size() << std::endl;
        }

        outPath = outDir / "sound";
        makePath(outPath);

        if (opt.sound.convertToWav)
        {
            wavDir = outDir / "wav";
            makePath(wavDir);
        }
    }

    for (const auto[idx, s] : enumerate(sounds))
    {
        if (opt.verboseOutput) {
            std::cout << "Extracting sound: " << s.name() << std::endl;
        }
        else {
            printProgress("Extracting sounds... ", idx + 1, sounds.size());
        }

        OutputFileStream ofs(outPath.append(s.name()), /*truncate=*/true);
        s.serialize(ofs, Sound::SerializeFormat::IndyWV);
        outPath = outPath.parent_path();

        /* Save in WAV format */
        if (opt.sound.convertToWav)
        {
            OutputFileStream ofs(wavDir.append(s.name()), /*truncate=*/true);
            s.serialize(ofs, Sound::SerializeFormat::WAV);
            wavDir = wavDir.parent_path();
        }
    }

    if (!opt.verboseOutput) std::cout << "\rExtracting sounds... " << kSuccess << std::endl;
    return sounds.size();
}

void extractAssets(const fs::path& cndFile, const fs::path& outDir, const ExtractOptions& opt)
{
    if (!opt.key.extract &&
       !opt.mat.extract &&
       !opt.sound.extract)
    {
        std::cout << "Nothing to be done!\n";
        return;
    }

    InputFileStream ifstream(cndFile);

    auto nExtAnimFiles = extractAnimations(ifstream, outDir, opt);
    auto nExtMatFiles  = extractMaterials(ifstream, outDir, opt);
    auto nExtSndFiles  = extractSounds(ifstream, outDir, opt);

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
}

int execCmdExtract(const CndToolArgs& args)
{
    try
    {
        const fs::path inputFile = args.cndFile();
        if (!fileExists(inputFile))
        {
            printErrorInvalidCnd(inputFile, cmdExtract);
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
            outDir = getBaseName(inputFile.u8string());
        }

        if (!isDirPath(outDir))
        {
            printError("Output path is not directory!\n");
            return 1;
        }

        ExtractOptions opt;
        opt.verboseOutput      = hasOptVerbose(args);
        opt.key.extract        = !args.hasArg(optNoAnimations);
        opt.mat.extract        = !args.hasArg(optNoMaterials);
        opt.mat.convertToBmp   = args.hasArg(optExtractAsBmpShort) || args.hasArg(optExtractAsBmp);
        opt.mat.convertToPng   = args.hasArg(optConvertToPngShort) || args.hasArg(optConvertToPng);
        opt.mat.convertMipMap  = args.hasArg(optExtractLod);
        opt.sound.extract      = !args.hasArg(optNoSounds);
        opt.sound.convertToWav = args.hasArg(optConvertToWavShort) || args.hasArg(optConvertToWav);
        if (args.hasArg(optMaxTex)) {
            opt.mat.maxTex = args.uintArg(optMaxTex);
        }

        /* Extract animations, materials & sounds */
        extractAssets(inputFile, std::move(outDir), opt);
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
            std::cout << "  " << i << ": " << l.at(i).name() << std::endl;
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
                      // << " | fileId: " << p.second.id() << " idx: " << p.second.idx() << std::endl;
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
        printError("Unknown subcommand '%'!\n", scmd);
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
    gLogLevel = LogLevel::Warning;

    std::cout << "\nIndiana Jones and the Infernal Machine CND file tool v" << kVersion << std::endl;
    CndToolArgs args(argc, argv);
    if (argc < 2 ||
       args.cmd().empty())
    {
        printHelp();
        return 1;
    }

    return execCmd(args.cmd(), args);
}