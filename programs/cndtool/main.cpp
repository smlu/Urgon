#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

#include <libim/common.h>
#include <libim/content/asset/animation/animation.h>
#include <libim/content/asset/material/bmp.h>
#include <libim/content/asset/material/material.h>
#include <libim/content/asset/world/impl/serialization/cnd/cnd.h>
#include <libim/content/audio/soundbank.h>
#include <libim/content/text/text_resource_writer.h>
#include <libim/io/filestream.h>
#include <libim/log/log.h>
#include <libim/types/safe_cast.h>

#include "config.h"
#include "cndtoolargs.h"
#include "patch.h"

#define SETW(n, f)  std::right << std::setfill(f) << std::setw(n)
#define SET_VINFO_LW(n) SETW(32 + n, '.')

using namespace cmdutils;
using namespace cndtool;
using namespace libim;
using namespace libim::content::audio;
using namespace libim::content::asset;
using namespace libim::content::text;

using namespace std::string_literals;
using namespace std::string_view_literals;

static constexpr auto extKey = ".key"sv;
static constexpr auto extMat = ".mat"sv;

constexpr static auto cmdAdd     = "add"sv;
constexpr static auto cmdExtract = "extract"sv;
constexpr static auto cmdList    = "list"sv;
constexpr static auto cmdRemove  = "remove"sv;
constexpr static auto cmdHelp    = "help"sv;

constexpr static auto scmdAnimation = "animation"sv;
constexpr static auto scmdMaterial  = "material"sv;

constexpr static auto optAnimations        = "--animations"sv;
constexpr static auto optConvertToBmp      = "--bmp"sv;
constexpr static auto optConvertToBmpShort = "-b"sv;
constexpr static auto optMaxTex            = "--max-tex"sv;
constexpr static auto optMipmap            = "--mipmap"sv;
constexpr static auto optMaterials         = "--materials"sv;
constexpr static auto optNoAnimations      = "--no-animations"sv;
constexpr static auto optNoMaterials       = "--no-materials"sv;
constexpr static auto optNoSounds          = "--no-sounds"sv;
constexpr static auto optOutputDir         = "--output-dir"sv;
constexpr static auto optOutputDirShort    = "-o"sv;
constexpr static auto optReplace           = "--replace"sv;
constexpr static auto optReplaceShort      = "-r"sv;
constexpr static auto optSounds            = "--sounds"sv;
constexpr static auto optVerbose           = "--verbose"sv;
constexpr static auto optVerboseShort      = "-v"sv;
constexpr static auto optConvertToWav      = "--wav"sv;
constexpr static auto optConvertToWavShort = "-w"sv;


constexpr static auto kFailed  = "FAILED"sv;
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


void printHelp(std::string_view cmd= ""sv, std::string_view subcmd= ""sv);
void printMaterialInfo(const Material& mat);
void printMipmapInfo(const Mipmap& mipmap, uint32_t mmIdx);
void printErrorInvalidCnd(std::string_view cndPath, std::string_view cmd, std::string_view subcmd = ""sv);

int execCmd(std::string_view cmd, const CndToolArgs& args);

// Functions for extracting assets from level
int execCmdExtract(const CndToolArgs& args);
void extractAssets(const std::string& cndFile, std::string outDir, const ExtractOptions& opt);
std::size_t extractAnimations(const InputStream& istream, const std::string& outDir, const ExtractOptions& opt);
std::size_t extractMaterials(const InputStream& istream, const std::string& outDir, const ExtractOptions& opt);
std::size_t extractSounds(const InputStream& istream, const std::string& outDir, const ExtractOptions& opt);

// Functions for adding assets to level
int execCmdAdd(std::string_view scmd, const CndToolArgs& args);
int execCmdAddAnimation(const CndToolArgs& args);
int execCmdAddMaterial(const CndToolArgs& args);

// Functions for listing level assets
int execCmdList(const CndToolArgs& args);

// Functions for removing assets from level
int execCmdRemove(std::string_view scmd, const CndToolArgs& args);
int execCmdRemoveAnimation(const CndToolArgs& args);
int execCmdRemoveMaterial(const CndToolArgs& args);

template<typename AssetT, typename ResLoadF, typename CndReadF, typename CndWriteF>
bool execCmdAddAssets(const CndToolArgs& args, std::string_view assetFileExt, ResLoadF&& loadAsset, CndReadF&& cndReadAssets, CndWriteF&& cndWriteAssets)
{
    const bool bVerbose = hasOptVerbose(args);
    std::string cndFile = args.cndFile();
    if(!fileExists(cndFile))
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
            const bool bValidFile = fileExtMatch(assetFile, assetFileExt);
            const bool bFileExists = bValidFile && fileExists(assetFile);
            if(!bValidFile)
            {
                std::cerr << "ERROR: " << "Invalid file '" << assetFile << "', expected file with an extension '" << assetFileExt << "'!\n\n";
                printHelp(cmdAdd, args.subcmd());
                return false;
            }
            else if(!bFileExists)
            {
                std::cerr << "ERROR: " << "File '" << assetFile << "' does not exists!\n\n";
                printHelp(cmdAdd, args.subcmd());
                return false;
            }

            InputFileStream as(assetFile);
            newAssets.push_back(loadAsset(as));
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to load asset file: " << assetFile << std::endl;
            if(bVerbose) {
                std::cerr << "Reason: " << e.what() << std::endl;
            }
        }
    }

    if(newAssets.empty())
    {
        std::cerr << "ERROR: Valid '" << assetFileExt << "' file path required!\n\n";
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
            if(mapAssets.contains(asset.name()) && !hasOptReplace(args))
            {
                std::cerr << "\nERROR: Asset '" << asset.name() << "' already exists in CND file and no '--replace' option was provided.\n";
                std::cerr << "         CND file was not modified!\n\n";
                return false;
            }

            mapAssets[asset.name()] = std::move(asset);
        }

        const bool success = cndWriteAssets(cndFile, mapAssets);
        if(success) {
            std::cout << kSuccess << std::endl;
        }
        return success;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: Failed to patch CND file!\n";
        std::cerr <<        "Reason: " << e.what() << std::endl << std::endl;
        printHelp(cmdAdd, args.subcmd());
        return false;
    }
}


template<typename AssetT, typename CndReadF, typename CndWriteF>
bool execCmdRemoveAssets(const CndToolArgs& args, CndReadF&& cndReadAssets, CndWriteF&& cndWriteAssets)
{
    const bool bVerbose = hasOptVerbose(args);
    std::string cndFile = args.cndFile();
    if(!fileExists(cndFile))
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
            if(mapAssets.contains(asset)) {
                mapAssets.erase(asset);
            }
            else if (bVerbose) {
                std::cerr << "CND file doesn't contain asset '" << asset << "'\n";
            }
        }

        const bool success = cndWriteAssets(cndFile, mapAssets);
        if(success) {
            std::cout << kSuccess << std::endl;
        }
        return success;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: Failed to patch CND file!\n";
        std::cerr <<        "Reason: " << e.what() << std::endl << std::endl;
        return false;
    }
}


int main(int argc, const char *argv[])
{
    gLogLevel = LogLevel::Warning;

    std::cout << "\nIndiana Jones and the Infernal Machine CND file tool v" << kVersion << std::endl;
    CndToolArgs args(argc, argv);
    if(argc < 2 ||
       args.cmd().empty())
    {
        printHelp();
        return 1;
    }

    return execCmd(args.cmd(), args);
}


int execCmd(std::string_view cmd, const CndToolArgs& args)
{
    if(cmd == cmdExtract) {
        return execCmdExtract(args);
    }
    else if(cmd == cmdAdd) {
        return execCmdAdd(args.subcmd(), args);
    }
    else if(cmd == cmdList) {
        return execCmdList(args);
    }
    else if(cmd == cmdRemove) {
        return execCmdRemove(args.subcmd(), args);
    }
    else
    {
        if(cmd != cmdHelp) {
            std::cerr << "ERROR: Unknown command\n\n";
        }
        printHelp();
    }

    return 1;
}

void printHelp(std::string_view cmd, std::string_view subcmd)
{
    if (cmd == cmdAdd)
    {
        if(subcmd == scmdAnimation)
        {
            std::cout << "Add or replace existing animation assets in a CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add animation [options] <cnd-file-path> <animation-files>" << std::endl << std::endl;
            std::cout << "Option:        Long option:        Description:\n";
            std::cout << "  " << optReplaceShort << SETW(22, ' ') << optReplace << SETW(44, ' ') << "Replace existing animation asset\n";
            std::cout << "  " << optVerboseShort << SETW(22, ' ') << optVerbose << SETW(43, ' ') << "Verbose printout to the console\n";
        }
        else if(subcmd == scmdMaterial)
        {
            std::cout << "Add or replace existing material assets in a CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add material [options] <cnd-file-path> <material-files>" << std::endl << std::endl;
            std::cout << "Option:        Long option:        Description:\n";
            std::cout << "  " << optReplaceShort << SETW(22, ' ') << optReplace << SETW(43, ' ') << "Replace existing material asset\n";
            std::cout << "  " << optVerboseShort << SETW(22, ' ') << optVerbose << SETW(43, ' ') << "Verbose printout to the console\n";
        }
        else
        {
            std::cout << "Add or replace existing assets in a CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add <sub-command> " << std::endl << std::endl;
            std::cout << "Sub-commands:" << SETW(31, ' ') << "Description:\n";
            std::cout << "  " << scmdAnimation << SETW(43, ' ') << "Add animation assets\n";
            std::cout << "  " << scmdMaterial  << SETW(43, ' ') << "Add material assets\n";
        }
    }
    else if(cmd == cmdExtract)
    {
        std::cout << "Extract animation [KEY], material [MAT] and sound [IndyWV] assets from CND level file." << std::endl << std::endl;
        std::cout << "  Usage: cndtool extract [options] <cnd-file-path>" << std::endl << std::endl;
        std::cout << "Option:        Long option:        Description:\n";
        std::cout << "  " << optConvertToBmpShort << SETW(18, ' ') << optConvertToBmp << SETW(63, ' ') << "Convert extracted material assets to BMP format\n";
        std::cout << "  " <<                         SETW(24, ' ') << optMaxTex       << SETW(70, ' ') << "Max number of textures to convert from each material file.\n";
        std::cout <<                                                                     SETW(67, ' ') << "By default all are converted.\n";
        std::cout << "  " <<                         SETW(23, ' ') << optMipmap       << SETW(65, ' ') << "Convert mipmap LOD textures from each material file\n\n";
        std::cout << "  " << optConvertToWavShort << SETW(18, ' ') << optConvertToWav << SETW(68, ' ') << "Convert extracted IndyWV sound assets to WAV format\n\n";
        std::cout << "  " <<                         SETW(30, ' ') << optNoAnimations << SETW(36, ' ') << "Don't extract animation assets\n";
        std::cout << "  " <<                         SETW(29, ' ') << optNoMaterials  << SETW(36, ' ') << "Don't extract material assets\n";
        std::cout << "  " <<                         SETW(26, ' ') << optNoSounds     << SETW(37, ' ') << "Don't extract sound assets\n\n";
        std::cout << "  " << optOutputDirShort    << SETW(25, ' ') << optOutputDir    << SETW(22, ' ') << "Output folder\n";
        std::cout << "  " << optVerboseShort      << SETW(22, ' ') << optVerbose      << SETW(43, ' ') << "Verbose printout to the console\n";
    }
    else if(cmd == cmdList)
    {
        std::cout << "List assets in a CND level file." << std::endl << std::endl;
        std::cout << "  Usage: cndtool list [options] <cnd-file-path>" << std::endl << std::endl;
        std::cout << "Option:" << SETW(31, ' ') << " Description:\n";
        std::cout << "  " << optAnimations << SETW(40, ' ') << "List only animation assets\n";
        std::cout << "  " << optMaterials  << SETW(40, ' ') << "List only material assets\n";
        std::cout << "  " << optSounds     << SETW(40, ' ') << "List only sound assets\n";
    }
    else if (cmd == cmdRemove)
    {
        if(subcmd == scmdAnimation)
        {
            std::cout << "Remove animation assets from CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool remove animation [options] <cnd-file-path> <animation_name>.key ..." << std::endl << std::endl;
            std::cout << "Options:        Long options:        Description:\n";
            std::cout << "  " << optVerboseShort << SETW(23, ' ') << optVerbose << SETW(44, ' ') << "Verbose printout to the console\n";
        }
        else if(subcmd == scmdMaterial)
        {
            std::cout << "Remove material assets from a CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool remove material [options] <cnd-file-path> <animation_name>.mat ..." << std::endl << std::endl;
            std::cout << "Options:        Long options:        Description:\n";
            std::cout << "  " << optVerboseShort << SETW(23, ' ') << optVerbose << SETW(44, ' ') << "Verbose printout to the console\n";
        }
        else
        {
            std::cout << "Remove assets from CND level file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool remove <sub-command> " << std::endl << std::endl;
            std::cout << "Sub-commands:" << SETW(31, ' ') << "Description:\n";
            std::cout << "  " << scmdAnimation << SETW(46, ' ') << "Remove animation assets\n";
            std::cout << "  " << scmdMaterial  << SETW(46, ' ') << "Remove material assets\n";
        }
    }
    else
    {
        std::cout << "\nCommand line interface tool to extract and modify\ngame assets stored in a CND level file.\n\n";
        std::cout << "  Usage: cndtool <command> [sub-command] [options]" << std::endl << std::endl;
        std::cout << "Commands:" << SETW(35, ' ') << "Description:\n";
        std::cout << "  " << cmdAdd     << SETW(55, ' ') << "Add or replace game assets\n";
        std::cout << "  " << cmdExtract << SETW(44, ' ') << "Extract game assets\n";
        std::cout << "  " << cmdList    << SETW(67, ' ') << "Print to the console stored game assets\n";
        std::cout << "  " << cmdRemove  << SETW(56, ' ') << "Remove one or more game assets\n";
        std::cout << "  " << cmdHelp    << SETW(45, ' ') << "Show this message\n";
    }
}

void printMaterialInfo(const Material& mat)
{
    if(mat.mipmaps().empty()) return;
    std::cout << "    Total mipmaps:" << SET_VINFO_LW(2) << mat.mipmaps().size() << std::endl << std::endl;
}

int execCmdAdd(std::string_view scmd, const CndToolArgs& args)
{
    if(scmd == scmdAnimation) {
        return execCmdAddAnimation(args);
    }
    else if(scmd == scmdMaterial) {
        return execCmdAddMaterial(args);
    }

    if(scmd.empty()) {
        std::cerr << "ERROR: Subcommand required!\n\n";
    }
    else {
        std::cerr << "ERROR: Unknown subcommand \"" << scmd << "\"!\n\n";
    }

    printHelp(cmdAdd);
    return 1;
}

int execCmdAddAnimation(const CndToolArgs& args)
{
    bool success = execCmdAddAssets<Animation>(args, extKey,
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
    bool success = execCmdAddAssets<Material>(args, extMat,
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

int execCmdList(const CndToolArgs& args)
{
    std::string cndFile = args.cndFile();
    if(!fileExists(cndFile))
    {
        printErrorInvalidCnd(cndFile, cmdList);
        return 1;
    }

    bool listAnim = args.hasArg(optAnimations);
    bool listMat  = args.hasArg(optMaterials);
    bool listSnd  = args.hasArg(optSounds);

    if(!listAnim && !listMat && !listSnd) {
        listAnim = listMat = listSnd = true;
    }

    auto printList = [](const auto& l) {
        for(std::size_t i = 0; i < l.size(); i++) {
            std::cout << "  " << i << ": " << l.at(i).name() << std::endl;
        }
        std::cout << std::endl;
    };

    InputFileStream istream(cndFile);
    if(listAnim)
    {
        auto anims = CND::readKeyframes(istream);
        std::cout << "Animations:\n";
        printList(anims);
    }

    if(listMat)
    {
        auto mats = CND::readMaterials(istream);
        std::cout << "Materials:\n";
        printList(mats);
    }

    if(listSnd)
    {
        SoundBank sb(1);
        sb.importTrack(0, istream);
        auto& sounds = sb.getTrack(0);

        std::cout << "Sounds:\n";
        std::size_t i = 0;
        for(const auto& s : sounds)
        {
            std::cout << "  " << i++ << ": " << s.name() << std::endl;
                      //<< SETW(38 - p.first.size(), ' ')
                      //<< " | fileId: " << p.second.id() << " idx: " << p.second.idx() << std::endl;
        }
        std::cout << std::endl;
    }

    return 1;
}

int execCmdExtract(const CndToolArgs& args)
{
    try
    {
        std::string inputFile = args.cndFile();
        if(!fileExists(inputFile))
        {
            printErrorInvalidCnd(inputFile, cmdExtract);
            return 1;
        }

        std::string outDir;
        if(args.hasArg(optOutputDirShort)){
            outDir = args.arg(optOutputDirShort);
        }
        else if(args.hasArg(optOutputDir)){
            outDir = args.arg(optOutputDir);
        }
        else {
            outDir += getBaseName(inputFile);
        }

        ExtractOptions opt;
        opt.verboseOutput      = hasOptVerbose(args);
        opt.key.extract        = !args.hasArg(optNoAnimations);
        opt.mat.extract        = !args.hasArg(optNoMaterials);
        opt.mat.convertToBmp   = args.hasArg(optConvertToBmpShort) || args.hasArg(optConvertToBmp);
        opt.mat.convertMipMap  = args.hasArg(optMipmap);
        if(args.hasArg(optMaxTex)) {
            opt.mat.maxTex = args.uintArg(optMaxTex);
        }
        opt.sound.extract      = !args.hasArg(optNoSounds);
        opt.sound.convertToWav = args.hasArg(optConvertToWavShort) || args.hasArg(optConvertToWav);

        /* Extract animations, materials & sounds */
        extractAssets(inputFile, std::move(outDir), opt);
        return 0;
    }
    catch(const std::exception& e)
    {
        std::cerr << "\nERROR: Failed to extract assets from CND file!" << std::endl;
        std::cerr << "       Reason: " << e.what() << std::endl;
        return 1;
    }
}

void extractAssets(const std::string& cndFile, std::string outDir, const ExtractOptions& opt)
{

    if(!opt.key.extract &&
       !opt.mat.extract &&
       !opt.sound.extract)
    {
        std::cout << "Nothing to do!\n";
        return;
    }

    InputFileStream ifstream(cndFile);

    auto nExtAnimFiles = extractAnimations(ifstream, outDir, opt);
    auto nExtMatFiles  = extractMaterials(ifstream, outDir, opt);
    auto nExtSndFiles  = extractSounds(ifstream, outDir, opt);

    std::cout << "\n-------------------------------------\n";
    if(opt.key.extract) {
        std::cout << "Total extracted animations: " << nExtAnimFiles << std::endl;
    }
    if(opt.mat.extract) {
        std::cout << "Total extracted materials:  " << nExtMatFiles << std::endl;
    }
    if(opt.sound.extract) {
        std::cout << "Total extracted sounds:     " << nExtSndFiles << std::endl;
    }
}

std::size_t extractAnimations(const InputStream& istream, const std::string& outDir, const ExtractOptions& opt)
{
    if(!opt.key.extract) {
        return 0;
    }

    std::cout << "Extracting animations... " << std::flush;
    auto mapAnimations = CND::readKeyframes(istream);

    std::string keyDir;
    if(!mapAnimations.isEmpty())
    {
        if(opt.verboseOutput) {
            std::cout << "\nFound: " << mapAnimations.size() << std::endl;
        }

        keyDir = outDir + (outDir.empty() ? "" : "/" ) + "key";
        makePath(keyDir);
    }

    /* Save extracted animations to file */
    const std::string keyHeaderComment = [&]() {
        return "Extracted from CND file '"s + istream.name() + "' with " +
            std::string(kProgramName) + " v" + std::string(kVersion);
    }();

    for(const auto& key : mapAnimations)
    {
        if(opt.verboseOutput){
            std::cout << "Extracting animation: " << key.name() << std::endl;
        }

        std::string keyFilePath(keyDir + "/" + key.name());
        OutputFileStream ofs(std::move(keyFilePath));
        key.serialize(TextResourceWriter(ofs), keyHeaderComment);
    }

    std::cout << kSuccess << std::endl;
    return mapAnimations.size();
}

std::size_t extractMaterials(const InputStream& istream, const std::string& outDir, const ExtractOptions& opt)
{
    if(!opt.mat.extract) {
        return 0;
    }

    std::cout << "Extracting materials... " << std::flush;
    auto materials = CND::readMaterials(istream);

    std::string matDir;
    std::string bmpDir;
    if(!materials.isEmpty())
    {
        if(opt.verboseOutput) {
            std:: cout << " Found: " << materials.size() << std::endl;
        }

        matDir = outDir + (outDir.empty() ? "" : "/" ) + "mat";
        makePath(matDir);

        if(opt.mat.convertToBmp)
        {
            bmpDir = outDir + (outDir.empty() ? "" : "/" ) + "bmp";
            makePath(bmpDir);
        }
    }

    /* Save extracted materials to files */
    const bool convert    = opt.mat.convertToBmp;
    const uint64_t maxTex = opt.mat.maxTex.value_or(std::numeric_limits<uint64_t>::max());
    if(convert && maxTex == 0) {
        std::cout << "Warning: Materials won't be converted because option '" << optMaxTex << "' is 0!\n";
    }

    for(const auto& mat : materials)
    {
        if(opt.verboseOutput) {
            std::cout << "Extracting material: " <<  mat.name() << std::endl;
        }

        std::string matFilePath(matDir + "/" + mat.name());
        mat.serialize(OutputFileStream(std::move(matFilePath)));

        if(opt.verboseOutput)
        {
            std::cout << "  ================== Material Info ===================" << std::endl;
            printMaterialInfo(mat);
        }

        /* Print material mipmaps info and convert to bmp */
        if(convert || opt.verboseOutput)
        {
            uint32_t texIdx = 0;
            for(const auto& mipmap : mat.mipmaps())
            {
                if(opt.verboseOutput) {
                    printMipmapInfo(mipmap, texIdx);
                }
                else if(texIdx >= maxTex) {
                    break;
                }

                /* Save as bmp */
                if(convert && texIdx < maxTex)
                {
                    const std::size_t mmCount = opt.mat.convertMipMap ? mipmap.size() : 1;
                    for(std::size_t mmIdx = 0; mmIdx < mmCount; mmIdx++)
                    {
                        const std::string sufix = (mat.mipmaps().size() > 1 && maxTex > 1 ? "_" + std::to_string(texIdx) : "") + ".bmp";
                        const std::string infix = mmCount > 1 ? "_lod" + std::to_string(mmIdx) : "";
                        const std::string fileName = bmpDir + "/" + getBaseName(mat.name()) + infix + sufix;

                        if(!SaveBmpToFile(fileName, mipmap.at(mmIdx).toBmp()))
                        {
                            std::cout << kFailed << std::endl;
                            return -1;
                        }
                    }
                }

                texIdx++;
            }
        }

        if(opt.verboseOutput) {
            std::cout << "  =============== Material Info End =================\n\n\n";
        }
    }

    if(!opt.verboseOutput) {
        std::cout << kSuccess << std::endl;
    }

    return materials.size();
}

std::size_t extractSounds(const InputStream& istream, const std::string& outDir, const ExtractOptions& opt)
{
    if(!opt.sound.extract) {
        return 0;
    }

    std::cout << "Extracting sounds... " << std::flush;
    SoundBank sb(2);
    sb.importTrack(0, istream);
    auto& sounds = sb.getTrack(0);

    std::filesystem::path outPath;
    std::filesystem::path wavDir;

    if(!sounds.isEmpty())
    {
        if(opt.verboseOutput) {
            std::cout << "\nFound: " << sounds.size() << std::endl;
        }

        outPath = outDir + (outDir.empty() ? "" : "/" ) + "sound";
        makePath(outPath);

        if(opt.sound.convertToWav)
        {
            wavDir = outDir + (outDir.empty() ? "" : "/" ) + "wav";
            makePath(wavDir);
        }
    }

    for (const auto& s : sounds)
    {
        if(opt.verboseOutput) {
            std::cout << "Extracting sound: " << s.name() << std::endl;
        }

        OutputFileStream ofs(outPath.append(s.name()));
        s.serialize(ofs, Sound::SerializeFormat::IndyWV);
        outPath = outPath.parent_path();

        /* Save in WAV format */
        if(opt.sound.convertToWav)
        {
            OutputFileStream ofs(wavDir.append(s.name()));
            s.serialize(ofs, Sound::SerializeFormat::WAV);
            wavDir = wavDir.parent_path();
        }
    }

    std::cout << kSuccess << std::endl;
    return sounds.size();
}

int execCmdRemove(std::string_view scmd, const CndToolArgs& args)
{
    if(scmd == scmdAnimation) {
        return execCmdRemoveAnimation(args);
    }
    else if(scmd == scmdMaterial) {
        return execCmdRemoveMaterial(args);
    }

    if(scmd.empty()) {
        std::cerr << "ERROR: Subcommand required!\n\n";
    }
    else {
        std::cerr << "ERROR: Unknown subcommand \"" << scmd << "\"!\n\n";
    }

    printHelp(cmdRemove);
    return 1;
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

void printMipmapInfo(const Mipmap& mipmap, uint32_t mmIdx)
{
    if(mipmap.empty()) return;
    const Texture& tex = mipmap.at(0);

    std::string colorMode;
    switch (tex.colorInfo().colorMode)
    {
        case ColorMode::Indexed:
            colorMode = "Indexed";
            break;
        case ColorMode::RGB:
            colorMode = "RGB";
            break;
        case ColorMode::RGBA:
            colorMode = "RGBA";
            break;
        default:
            colorMode = "Unknown";
            break;
    }

    std::cout << "    ------------------ Mipmap Info -----------------\n";
    std::cout << "    MIP num:" << SET_VINFO_LW(8)  << mmIdx << std::endl;
    std::cout << "    Width:"   << SET_VINFO_LW(10) << tex.width() << std::endl;
    std::cout << "    Height:"  << SET_VINFO_LW(9)  << tex.height() << std::endl;
    std::cout << "    Mipmap textures:" << SET_VINFO_LW(0) << mipmap.size() << std::endl;
    std::cout << "    Pixel data size:" << SET_VINFO_LW(0) << getMipmapPixelDataSize(mipmap.size(), tex.width(), tex.height(), tex.colorInfo().bpp) << std::endl;
    std::cout << "    Color info:\n";

    auto cmLw = colorMode.size() /2;
    cmLw = (colorMode.size()  % 8 == 0 ? cmLw -1 : cmLw);
    std::cout << "      Color mode:" << SET_VINFO_LW(cmLw) << colorMode << std::endl;
    std::cout << "      Bit depth:"  << SET_VINFO_LW(4) << tex.colorInfo().bpp << std::endl;
    std::cout << "      Bit depth per channel:" << std::endl;
    std::cout << "        Red:"   << SET_VINFO_LW(8) << tex.colorInfo().redBPP   << std::endl;
    std::cout << "        Green:" << SET_VINFO_LW(6) << tex.colorInfo().greenBPP << std::endl;
    std::cout << "        Blue:"  << SET_VINFO_LW(7) << tex.colorInfo().blueBPP  << std::endl;
    std::cout << "        Alpha:" << SET_VINFO_LW(6) << tex.colorInfo().alphaBPP << std::endl;
    std::cout << "      Left shift per channel:" << std::endl;
    std::cout << "        Red:"   << SET_VINFO_LW(8) << tex.colorInfo().redShl   << std::endl;
    std::cout << "        Green:" << SET_VINFO_LW(6) << tex.colorInfo().greenShl << std::endl;
    std::cout << "        Blue:"  << SET_VINFO_LW(7) << tex.colorInfo().blueShl  << std::endl;
    std::cout << "        Alpha:" << SET_VINFO_LW(6) << tex.colorInfo().alphaShl << std::endl;
    std::cout << "      Right shift per channel:" << std::endl;
    std::cout << "        Red:"   << SET_VINFO_LW(8) << tex.colorInfo().redShr   << std::endl;
    std::cout << "        Green:" << SET_VINFO_LW(6) << tex.colorInfo().greenShr << std::endl;
    std::cout << "        Blue:"  << SET_VINFO_LW(7) << tex.colorInfo().blueShr  << std::endl;
    std::cout << "        Alpha:" << SET_VINFO_LW(6) << tex.colorInfo().alphaShr << std::endl << std::endl;
}

void printErrorInvalidCnd(std::string_view cndPath, std::string_view cmd, std::string_view subcmd)
{
    if(cndPath.empty()) {
        std::cerr << "ERROR: A valid CND file path required!\n\n";
    } else {
        std::cerr << "ERROR: CND file \"" << cndPath << "\" does not exist!\n\n";
    }
    printHelp(cmd, subcmd);
}
