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
constexpr static auto optConvertToWav      = "--wav"sv;
constexpr static auto optConvertToWavShort = "-w"sv;
constexpr static auto optMaterials         = "--materials"sv;
constexpr static auto optOutputDir         = "--output-dir"sv;
constexpr static auto optOutputDirShort    = "-o"sv;
constexpr static auto optReplace           = "--replace"sv;
constexpr static auto optReplaceShort      = "-r"sv;
constexpr static auto optSounds            = "--sounds"sv;
constexpr static auto optVerbose           = "--verbose"sv;
constexpr static auto optVerboseShort      = "-v"sv;


constexpr static auto kFailed  = "FAILED"sv;
constexpr static auto kSuccess = "SUCCESS"sv;


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

// Functions fro extractiong resources
int execCmdExtract(const CndToolArgs& args);
bool extractResources(const std::string& cndFile, std::string outDir, bool convertMaterials, bool convertSoundsToWav, bool verbose = false);
int32_t extractAnimations(const InputStream& istream, const std::string& outDir, bool verbose);
int32_t extractMaterials(const InputStream& istream, const std::string& outDir, bool convertMaterials , bool verbose);
int32_t extractSounds(const InputStream& istream, const std::string& outDir, bool convetToWav, bool verbose);

// Functions for adding resources
int execCmdAdd(std::string_view scmd, const CndToolArgs& args);
int execCmdAddAnimation(const CndToolArgs& args);
int execCmdAddMaterial(const CndToolArgs& args);

// Functions for listing resources
int execCmdList(const CndToolArgs& args);

// Functions for removing resources
int execCmdRemove(std::string_view scmd, const CndToolArgs& args);
int execCmdRemoveAnimation(const CndToolArgs& args);
int execCmdRemoveMaterial(const CndToolArgs& args);

template<typename ResourceT, typename ResLoadF, typename CndReadF, typename CndWriteF>
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

    std::vector<ResourceT> newAssets;
    newAssets.reserve(args.unspecified().size());
    for (const auto& assetFile : args.unspecified())
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
            std::cerr << "Failed to load resource file: " << assetFile << std::endl;
            if(bVerbose) {
                std::cerr << "Error: " << e.what() << std::endl;
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
                std::cerr << "\nERROR: Resource '" << asset.name() << "' already exists in CND file and no '--replace' option was provided.\n";
                std::cerr << "       CND file was not modified!\n\n";
                return 1;
            }

            mapAssets[asset.name()] = std::move(asset);
        }

        const bool success = cndWriteAssets(cndFile, mapAssets);
        std::cout << (success ? kSuccess : kFailed) << std::endl;
        return success;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: An exception was encountered while updating resources in CND file!\n";
        std::cerr <<         "e: " << e.what() << std::endl << std::endl;
        printHelp(cmdAdd, args.subcmd());
        return false;
    }
}


template<typename ResourceT, typename CndReadF, typename CndWriteF>
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

        for(const auto& asset : args.unspecified())
        {
            if(mapAssets.contains(asset)) {
                mapAssets.erase(asset);
            }
            else if (bVerbose) {
                std::cerr << "CND file doesn't contain asset '" << asset << "'\n";
            }
        }

        const bool success = cndWriteAssets(cndFile, mapAssets);
        std::cout << (success ? kSuccess : kFailed) << std::endl;
        return success;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: An exception was encountered while updating resources in CND file!\n";
        std::cerr <<         "e: " << e.what() << std::endl << std::endl;
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
        std::cerr << "ERROR: Unknown command\n\n";
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
            std::cout << "Add or replace existing animations in CND file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add material [options] <cnd-file-path> <animation-files>" << std::endl << std::endl;
            std::cout << "Option:        Long option:        Description:\n";
            std::cout << "  " << optReplaceShort << SETW(23, ' ') << optReplace << SETW(51, ' ') << "Replace existing animation in CND file\n";
            std::cout << "  " << optVerboseShort << SETW(23, ' ') << optVerbose << SETW(44, ' ') << "Verbose printout to the console\n";
        }
        else if(subcmd == scmdMaterial)
        {
            std::cout << "Add or replace existing materials in CND file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add material [options] <cnd-file-path> <material-files>" << std::endl << std::endl;
            std::cout << "Option:        Long option:        Description:\n";
            std::cout << "  " << optReplaceShort << SETW(23, ' ') << optReplace << SETW(50, ' ') << "Replace existing material in CND file\n";
            std::cout << "  " << optVerboseShort << SETW(23, ' ') << optVerbose << SETW(44, ' ') << "Verbose printout to the console\n";
        }
        else
        {
            std::cout << "Add or replace existing resources in CND file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add <sub-command> " << std::endl << std::endl;
            std::cout << "Sub-commands:" << SETW(31, ' ') << "Description:\n";
            std::cout << "  " << scmdAnimation << SETW(58, ' ') << "Add animation resources to CND file\n";
            std::cout << "  " << scmdMaterial  << SETW(58, ' ') << "Add material resources to CND file\n";
        }
    }
    else if(cmd == cmdExtract)
    {
        std::cout << "Extract animations [KEY], materials [MAT] and sounds [IndyWV] from CND file." << std::endl << std::endl;
        std::cout << "  Usage: cndtool extract [options] <cnd-file-path>" << std::endl << std::endl;
        std::cout << "Option:        Long option:        Description:\n";
        std::cout << "  " << optConvertToBmpShort       << SETW(19, ' ') << optConvertToBmp << SETW(51, ' ') << "Convert extracted materials to BMP\n";
        std::cout << "  " << optConvertToWavShort       << SETW(19, ' ') << optConvertToWav << SETW(48, ' ') << "Convert extracted sounds to WAV\n";

        std::cout << "  " << optOutputDirShort    << SETW(26, ' ') << optOutputDir    << SETW(36, ' ') << "Output folder <output dir>\n";
        std::cout << "  " << optVerboseShort      << SETW(23, ' ') << optVerbose      << SETW(44, ' ') << "Verbose printout to the console\n";
    }
    else if(cmd == cmdList)
    {
        std::cout << "List resources in the CND file." << std::endl << std::endl;
        std::cout << "  Usage: cndtool list [options] <cnd-file-path>" << std::endl << std::endl;
        std::cout << "Option:              Description:\n";
        std::cout << "  " << optAnimations << SETW(52, ' ') << "List only animation resources in cnd file\n";
        std::cout << "  " << optMaterials  << SETW(52, ' ') << "List only material resources in cnd file\n";
        std::cout << "  " << optSounds     << SETW(52, ' ') << "List only sound resources in cnd file\n";
    }
    else if (cmd == cmdRemove)
    {
        if(subcmd == scmdAnimation)
        {
            std::cout << "Remove animations from CND file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool remove material [options] <cnd-file-path> <animation_name>.key ..." << std::endl << std::endl;
            std::cout << "Options:        Long options:        Description:\n";
            std::cout << "  " << optVerboseShort << SETW(23, ' ') << optVerbose << SETW(44, ' ') << "Verbose printout to the console\n";
        }
        else if(subcmd == scmdMaterial)
        {
            std::cout << "Remove materials from CND file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool remove material [options] <cnd-file-path> <animation_name>.mat ..." << std::endl << std::endl;
            std::cout << "Options:        Long options:        Description:\n";
            std::cout << "  " << optVerboseShort << SETW(23, ' ') << optVerbose << SETW(44, ' ') << "Verbose printout to the console\n";
        }
        else
        {
            std::cout << "Remove resources from CND file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool remove <sub-command> " << std::endl << std::endl;
            std::cout << "Sub-commands:" << SETW(31, ' ') << "Description:\n";
            std::cout << "  " << scmdAnimation << SETW(63, ' ') << "Remove animation resources from CND file\n";
            std::cout << "  " << scmdMaterial  << SETW(63, ' ') << "Remove material resources from CND file\n";
        }
    }
    else
    {
        std::cout << "\nCommand line interface tool to extract and modify game\nresources stored in a CND archive file.\n\n";
        std::cout << "  Usage: cndtool <command> [sub-command] [options]" << std::endl << std::endl;
        std::cout << "Commands:" << SETW(35, ' ') << "Description:\n";
        std::cout << "  " << cmdAdd   << SETW(57, ' ') << "Add or replace game resource\n";
        std::cout << "  " << cmdExtract   << SETW(47, ' ') << "Extract game resources\n";
        std::cout << "  " << cmdList   << SETW(70, ' ') << "Print to the console stored game resources\n";
        std::cout << "  " << cmdRemove  << SETW(58, ' ') << "Remove one or more game resource\n";
        std::cout << "  " << cmdHelp        << SETW(45, ' ') << "Show this message\n";
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
        std::cerr << "ERROR: Subcommand requirded!\n\n";
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

    const bool bVerboseOutput      = args.hasArg(optVerboseShort) || args.hasArg(optVerbose) ;
    const bool bConvertMatToBmp    = args.hasArg(optConvertToBmpShort) || args.hasArg(optConvertToBmp);
    const bool bConvertIndyWVToWav = args.hasArg(optConvertToWavShort) || args.hasArg(optConvertToWav);

    int result = 0;
    try
    {
        /* Extract animations, materials & sounds */
        if(!extractResources(inputFile, std::move(outDir), bConvertMatToBmp, bConvertIndyWVToWav, bVerboseOutput)) {
            result = 1;
        }
    }
    catch(const std::exception& e)
    {
        std::cout << "An exception was encountered during resources extraction: " << e.what() << std::endl;
        result = 1;
    }

    return result;
}

bool extractResources(const std::string& cndFile, std::string outDir, bool convertMaterials, bool convertSoundsToWav, bool verbose)
{
    InputFileStream ifstream(cndFile);

    auto nExtAnimFiles = extractAnimations(ifstream, outDir, verbose);
    if(nExtAnimFiles < 0) return false;

    auto nExtMatFiles  = extractMaterials(ifstream, outDir, convertMaterials, verbose);
    if(nExtMatFiles < 0) return false;

    auto nExtSndFiles = extractSounds(ifstream, outDir, convertSoundsToWav, verbose);
    if(nExtSndFiles < 0) return false;

    std::cout << "\n-------------------------------------\n";
    std::cout << "Total extracted animations: " << nExtAnimFiles << std::endl;
    std::cout << "Total extracted materials:  " << nExtMatFiles << std::endl;
    std::cout << "Total extracted sounds:     " << nExtSndFiles << std::endl;
    return true;
}

int32_t extractAnimations(const InputStream& istream, const std::string& outDir, bool verbose)
{
    try
    {
        std::cout << "Extracting animations... " << std::flush;
        auto mapAnimations = CND::readKeyframes(istream);

        std::string keyDir;
        if(!mapAnimations.isEmpty())
        {
            if(verbose) {
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
            if(verbose){
                std::cout << "Extracting animation: " << key.name() << std::endl;
            }

            std::string keyFilePath(keyDir + "/" + key.name());
            OutputFileStream ofs(std::move(keyFilePath));
            key.serialize(TextResourceWriter(ofs), keyHeaderComment);
        }

        std::cout << kSuccess << std::endl;
        return safe_cast<int32_t>(mapAnimations.size());

    }
    catch (const std::exception& e)
    {
        std::cout << kFailed << std::endl;
        if(verbose){
            std::cerr << "An exception was encountered while extracting animations, e=" << e.what() << std::endl;
        }
        return -1;
    }
}

int32_t extractMaterials(const InputStream& istream, const std::string& outDir, bool convertMaterials, bool verbose)
{
    std::cout << "Extracting materials... " << std::flush;
    auto materials = CND::readMaterials(istream);

    std::string matDir;
    std::string bmpDir;
    if(!materials.isEmpty())
    {
        if(verbose) {
            std:: cout << " Found: " << materials.size() << std::endl;
        }

        matDir = outDir + (outDir.empty() ? "" : "/" ) + "mat";
        makePath(matDir);

        if(convertMaterials)
        {
            bmpDir = outDir + (outDir.empty() ? "" : "/" ) + "bmp";
            makePath(bmpDir);
        }
    }

    /* Save extracted materials to files */
    for(const auto& mat : materials)
    {
        if(verbose) {
            std::cout << "Extracting material: " <<  mat.name() << std::endl;
        }

        std::string matFilePath(matDir + "/" + mat.name());
        mat.serialize(OutputFileStream(std::move(matFilePath)));

        if(verbose)
        {
            std::cout << "  ================== Material Info ===================" << std::endl;
            printMaterialInfo(mat);
        }

        /* Print material mipmaps info and convert to bmp */
        if(convertMaterials || verbose)
        {
            uint32_t mmIdx = 0;
            for(const auto& mipmap : mat.mipmaps())
            {
                if(verbose) {
                    printMipmapInfo(mipmap, mmIdx);
                }

                /* Save as bmp */
                if(convertMaterials)
                {
                    for(std::size_t mipmapIdx = 0; mipmapIdx < mipmap.size(); mipmapIdx++)
                    {
                        const std::string sufix = (mat.mipmaps().size() > 1 ? "_" + std::to_string(mmIdx) : "") + ".bmp";
                        const std::string infix = mipmap.size() > 1 ? "_" + std::to_string(mipmapIdx) : "";
                        const std::string fileName = bmpDir + "/" + getBaseName(mat.name()) + infix + sufix;

                        if(!SaveBmpToFile(fileName, mipmap.at(mipmapIdx).toBmp()))
                        {
                            std::cout << kFailed << std::endl;
                            return -1;
                        }
                    }
                }

                mmIdx++;
            }
        }

        if(verbose) {
            std::cout << "  =============== Material Info End =================\n\n\n";
        }
    }

    if(!verbose) {
        std::cout << kSuccess << std::endl;
    }

    return safe_cast<int32_t>(materials.size());
}

int32_t extractSounds(const InputStream& istream, const std::string& outDir, bool convetToWav, bool verbose)
{
    try
    {
        std::cout << "Extracting sounds... " << std::flush;
        SoundBank sb(2);
        sb.importTrack(0, istream);
        auto& sounds = sb.getTrack(0);

        std::filesystem::path outPath;
        std::filesystem::path wavDir;

        if(!sounds.isEmpty())
        {
            if(verbose) {
                std::cout << "\nFound: " << sounds.size() << std::endl;
            }

            outPath = outDir + (outDir.empty() ? "" : "/" ) + "sound";
            makePath(outPath);

            if(convetToWav)
            {
                wavDir = outDir + (outDir.empty() ? "" : "/" ) + "wav";
                makePath(wavDir);
            }
        }

        for (const auto& s : sounds)
        {
            if(verbose) {
                std::cout << "Extracting sound: " << s.name() << std::endl;
            }

            OutputFileStream ofs(outPath.append(s.name()));
            s.serialize(ofs, Sound::SerializeFormat::IndyWV);
            outPath = outPath.parent_path();

            /* Save in WAV format */
            if(convetToWav)
            {
                OutputFileStream ofs(wavDir.append(s.name()));
                s.serialize(ofs, Sound::SerializeFormat::WAV);
                wavDir = wavDir.parent_path();
            }
        }

        std::cout << kSuccess << std::endl;
        return safe_cast<int32_t>(sounds.size());
    }
    catch (const std::exception& e)
    {
        std::cout << kFailed << std::endl;
        if(verbose) {
            std::cerr << "An exception was encountered while extracting sounds from CND, e=" << e.what() << std::endl;
        }
        return -1;
    }
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
        std::cerr << "ERROR: Subcommand requirded!\n\n";
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
