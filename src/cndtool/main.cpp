#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

#include "libim/common.h"
#include "libim/io/filestream.h"
#include "libim/content/asset/animation/animation.h"
#include "libim/content/asset/material/bmp.h"
#include "libim/content/asset/material/material.h"
#include "libim/content/asset/world/impl/serialization/binary/cnd.h"
#include "libim/content/text/text_resource_writer.h"
#include "libim/log/log.h"

#include "cndtoolargs.h"
#include "patch.h"

#define SETW(n, f)  std::right << std::setfill(f) << std::setw(n)
#define SET_VINFO_LW(n) SETW(32 + n, '.')


using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;

using namespace std::string_literals;
using namespace std::string_view_literals;


static constexpr auto kProgramName    = "cndtool"sv;
static constexpr auto kProgramVersion = "0.1.0"sv;

static constexpr auto extKey = ".key"sv;
static constexpr auto extMat = ".mat"sv;

constexpr static auto cmdAdd     = "add"sv;
constexpr static auto cmdExtract = "extract"sv;
constexpr static auto cmdList    = "list"sv;
constexpr static auto cmdRemove  = "remove"sv;
constexpr static auto cmdHelp    = "help"sv;


constexpr static auto scmdAnimation = "animation"sv;
constexpr static auto scmdMaterial  = "material"sv;

constexpr static auto optAnimations        =  "--animations"sv;
constexpr static auto optConvertToBmp      =  "--bmp"sv;
constexpr static auto optConvertToBmpShort = "-b"sv;
constexpr static auto optMaterials         =  "--materials"sv;
constexpr static auto optOutputDir         =  "--output-dir"sv;
constexpr static auto optOutputDirShort    = "-o"sv;
constexpr static auto optReplace           = "--replace"sv;
constexpr static auto optReplaceShort      = "-r"sv;
constexpr static auto optVerbose           = "--verbose"sv;
constexpr static auto optVerboseShort      = "-v"sv;

bool HasOptVerbose(const CndToolArgs& args)
{
    return args.hasArg(optVerbose) || args.hasArg(optVerboseShort);
}

bool HasOptReplace(const CndToolArgs& args)
{
    return args.hasArg(optReplace) || args.hasArg(optReplaceShort);
}


void PrintHelp(std::string_view cmd= ""sv, std::string_view subcmd= ""sv);
void PrintMaterialInfo(const Material& mat);
void PrintMipmapInfo(const Mipmap& mipmap, uint32_t mmIdx);
void PrintErrorInvalidCnd(std::string_view cndPath, std::string_view cmd, std::string_view subcmd = ""sv);

int ExecCmd(std::string_view cmd, const CndToolArgs& args);

int ExecCmdExtract(const CndToolArgs& args);
bool ExtractResources(const std::string& cndFile, std::string outDir, bool convertMaterials, bool verbose = false);
std::size_t ExtractAnimations(const InputStream& istream, const std::string& outDir);
int32_t ExtractMaterials(const InputStream& istream, const std::string& outDir, bool convertMaterials , bool verbose);

int ExecCmdAdd(std::string_view scmd, const CndToolArgs& args);
int ExecCmdAddAnimation(const CndToolArgs& args);
int ExecCmdAddMaterial(const CndToolArgs& args);

int ExecCmdList(const CndToolArgs& args);

int ExecCmdRemove(std::string_view scmd, const CndToolArgs& args);
int ExecCmdRemoveAnimation(const CndToolArgs& args);
int ExecCmdRemoveMaterial(const CndToolArgs& args);

template<typename ResourceT, typename ResLoadF, typename CndReadF, typename CndWriteF>
bool ExecCmdAddAssets(const CndToolArgs& args, std::string_view assetFileExt, ResLoadF&& loadAsset, CndReadF&& cndReadAssets, CndWriteF&& cndWriteAssets)
{
    const bool bVerbose = HasOptVerbose(args);
    std::string cndFile = args.cndFile();
    if(!FileExists(cndFile))
    {
        PrintErrorInvalidCnd(cndFile, cmdAdd, args.subcmd());
        return false;
    }

    std::cout << "Patching CND file...\n";

    std::vector<ResourceT> newAssets;
    newAssets.reserve(args.unspecified().size());
    for (const auto& assetFile : args.unspecified())
    {
        try
        {
            const bool bValidFile = FileExtMatch(assetFile, assetFileExt);
            const bool bFileExists = bValidFile && FileExists(assetFile);
            if(!bValidFile)
            {
                std::cerr << "ERROR: " << "Invalid file '" << assetFile << "', expected file with an extension '" << assetFileExt << "'!\n\n";
                PrintHelp(cmdAdd, args.subcmd());
                return false;
            }
            else if(!bFileExists)
            {
                std::cerr << "ERROR: " << "File '" << assetFile << "' does not exists!\n\n";
                PrintHelp(cmdAdd, args.subcmd());
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
        PrintHelp(cmdAdd, args.subcmd());
        return false;
    }

    try
    {
        InputFileStream ifstream(cndFile);
        auto mapAssets = cndReadAssets(ifstream);
        ifstream.close();

        for(auto&& asset : newAssets)
        {
            if(mapAssets.contains(asset.name()) && !HasOptReplace(args))
            {
                std::cerr << "ERROR: Resource '" << asset.name() << "' already exists in CND file and no '--replace' option was provided!\n";
                std::cerr << "       CND file was not modified.\n\n";
                return 1;
            }

            mapAssets[asset.name()] = std::move(asset);
        }

        const bool success = cndWriteAssets(cndFile, mapAssets);
        std::cout << (success ? "SUCCESS" : "FAILED") << std::endl;
        return success;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: An exception was encountered while updating resources in CND file!\n";
        std::cerr <<         "e: " << e.what() << std::endl << std::endl;
        PrintHelp(cmdAdd, args.subcmd());
        return false;
    }
}


template<typename ResourceT, typename CndReadF, typename CndWriteF>
bool ExecCmdRemoveAssets(const CndToolArgs& args, CndReadF&& cndReadAssets, CndWriteF&& cndWriteAssets)
{
    const bool bVerbose = HasOptVerbose(args);
    std::string cndFile = args.cndFile();
    if(!FileExists(cndFile))
    {
        PrintErrorInvalidCnd(cndFile, cmdRemove, args.subcmd());
        return false;
    }

    try
    {
        std::cout << "Patching CND file...\n";
        InputFileStream ifstream(cndFile);
        auto mapAssets = cndReadAssets(ifstream);
        ifstream.close();

        const auto bVerbose = HasOptVerbose(args);
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
        std::cout << (success ? "SUCCESS" : "FAILED") << std::endl;
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
    std::cout << "\nIndiana Jones and the Infernal Machine CND atool v" << kProgramVersion << std::endl;
    CndToolArgs args(argc, argv);
    if(argc < 2 ||
       args.cmd().empty())
    {
        PrintHelp();
        return 1;
    }

    return ExecCmd(args.cmd(), args);
}


int ExecCmd(std::string_view cmd, const CndToolArgs& args)
{
    if(cmd == cmdExtract) {
        return ExecCmdExtract(args);
    }
    else if(cmd == cmdAdd) {
        return ExecCmdAdd(args.subcmd(), args);
    }
    else if(cmd == cmdList) {
        return ExecCmdList(args);
    }
    else if(cmd == cmdRemove) {
        return ExecCmdRemove(args.subcmd(), args);
    }
    else
    {
        std::cerr << "ERROR: Unknown command\n\n";
        PrintHelp();
    }

    return 1;
}

void PrintHelp(std::string_view cmd, std::string_view subcmd)
{
    if (cmd == cmdAdd)
    {
        if(subcmd == scmdAnimation)
        {
            std::cout << "Add or replace existing animations in CND file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add material [options] <cnd-file-path> <animation-files>" << std::endl << std::endl;
            std::cout << "Options:        Long options:        Description:\n";
            std::cout << "  " << optReplaceShort << SETW(23, ' ') << optReplace << SETW(51, ' ') << "Replace existing animation in CND file\n";
            std::cout << "  " << optVerboseShort << SETW(23, ' ') << optVerbose << SETW(44, ' ') << "Verbose printout to the console\n";
        }
        else if(subcmd == scmdMaterial)
        {
            std::cout << "Add or replace existing materials in CND file." << std::endl << std::endl;
            std::cout << "  Usage: cndtool add material [options] <cnd-file-path> <material-files>" << std::endl << std::endl;
            std::cout << "Options:        Long options:        Description:\n";
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
        std::cout << "Extract materials and animations from CND file." << std::endl << std::endl;
        std::cout << "  Usage: cndtool extract [options] <cnd-file-path>" << std::endl << std::endl;
        std::cout << "Options:        Long options:        Description:\n";
        std::cout << "  " << optConvertToBmpShort << SETW(19, ' ') << optConvertToBmp << SETW(51, ' ') << "Convert extracted materials to bmp\n";
        std::cout << "  " << optOutputDirShort    << SETW(26, ' ') << optOutputDir    << SETW(36, ' ') << "Output folder <output dir>\n";
        std::cout << "  " << optVerboseShort      << SETW(23, ' ') << optVerbose      << SETW(44, ' ') << "Verbose printout to the console\n";
    }
    else if(cmd == cmdList)
    {
        std::cout << "List resources in the CND file." << std::endl << std::endl;
        std::cout << "  Usage: cndtool list [options] <cnd-file-path>" << std::endl << std::endl;
        std::cout << "Options:              Description:\n";
        std::cout << "  " << optAnimations << SETW(52, ' ') << "List only animation resources in cnd file\n";
        std::cout << "  " << optMaterials  << SETW(53, ' ') << "List only materials resources in cnd file\n";
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

void PrintMaterialInfo(const Material& mat)
{
    if(mat.mipmaps().empty()) return;
    std::cout << "    Total mipmaps:" << SET_VINFO_LW(2) << mat.mipmaps().size() << std::endl << std::endl;
}

int ExecCmdAdd(std::string_view scmd, const CndToolArgs& args)
{
    if(scmd == scmdAnimation) {
        return ExecCmdAddAnimation(args);
    }
    else if(scmd == scmdMaterial) {
        return ExecCmdAddMaterial(args);
    }

    if(scmd.empty()) {
        std::cerr << "ERROR: Subcommand requirded!\n\n";
    }
    else {
        std::cerr << "ERROR: Unknown subcommand \"" << scmd << "\"!\n\n";
    }

    PrintHelp(cmdAdd);
    return 1;
}

int ExecCmdAddAnimation(const CndToolArgs& args)
{
    bool success = ExecCmdAddAssets<Animation>(args, ".key",
        [](const auto& istream){
            return Animation(TextResourceReader(istream));
        },
        [](const auto& istream){
            return CND::ReadAnimations(istream);
        },
        [](const auto& cndFile, const auto& animations){
            return PatchCndAnimations(cndFile, animations);
        }
    );

    return int(!success);
}

int ExecCmdAddMaterial(const CndToolArgs& args)
{
    bool success = ExecCmdAddAssets<Material>(args, ".mat",
        [](const auto& istream){
            return Material(istream);
        },
        [](const auto& istream){
            return CND::ReadMaterials(istream);
        },
        [](const auto& cndFile, const auto& materials){
            return PatchCndMaterials(cndFile, materials);
        }
    );

    return int(!success);
}

int ExecCmdList(const CndToolArgs& args)
{
    std::string cndFile = args.cndFile();
    if(!FileExists(cndFile))
    {
        PrintErrorInvalidCnd(cndFile, cmdList);
        return 1;
    }

    bool listAnim = args.hasArg(optAnimations);
    bool listMat  = args.hasArg(optMaterials);

    if(!listAnim && !listMat) {
        listAnim = listMat = true;
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
        auto anims = CND::ReadAnimations(istream);
        std::cout << "Animations:\n";
        printList(anims);
    }

    if(listMat)
    {
        auto mats = CND::ReadMaterials(istream);
        std::cout << "Materials:\n";
        printList(mats);
    }

    return 1;
}

int ExecCmdExtract(const CndToolArgs& args)
{
    std::string inputFile = args.cndFile();
    if(!FileExists(inputFile))
    {
        PrintErrorInvalidCnd(inputFile, cmdExtract);
        return 1;
    }

    std::string outDir;
    if(args.hasArg(optOutputDirShort)){
        outDir = args.arg(optOutputDirShort);
    }
    else if(args.hasArg(optOutputDir)){
        outDir = args.arg(optOutputDir);
    }

    const bool bVerboseOutput   = args.hasArg(optVerboseShort) || args.hasArg(optVerbose) ;
    const bool bConvertMatToBmp = args.hasArg(optConvertToBmpShort) || args.hasArg(optConvertToBmp);

    int result = 0;
    try
    {
        /* Extract animations & materials */
        if(!ExtractResources(inputFile, std::move(outDir), bConvertMatToBmp, bVerboseOutput)) {
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

bool ExtractResources(const std::string& cndFile, std::string outDir, bool convertMaterials, bool verbose)
{
    InputFileStream ifstream(cndFile);
    outDir += (outDir.empty() ? "" : "/" ) + GetBaseName(cndFile);

    auto nExtAnimFiles = ExtractAnimations(ifstream, outDir);
    auto nExtMatFiles  = ExtractMaterials(ifstream, outDir, convertMaterials, verbose);
    if(nExtMatFiles < 0) return false;

    std::cout << "\n-----------------------------------------\n";
    std::cout << "Total extracted animations: " << nExtAnimFiles << std::endl;
    std::cout << "Total extracted materials: " << nExtMatFiles << std::endl;
    return true;
}

std::size_t ExtractAnimations(const InputStream& istream, const std::string& outDir)
{
    auto mapAnimations = CND::ReadAnimations(istream);

    std::string keyDir;
    if(!mapAnimations.isEmpty())
    {
        std::cout << "Found animations: " << mapAnimations.size() << std::endl;
        keyDir = outDir + "/" + "key";
        MakePath(keyDir);
    }

    /* Save extracted animations to file */
    const std::string keyHeaderComment = [&]() {
        return "Extracted from CND file '"s + istream.name() + "' with " +
            std::string(kProgramName) + " v" + std::string(kProgramVersion);
    }();

    for(const auto& key : mapAnimations)
    {
        std::cout << "Extracting animation: " << key.name() << std::endl;

        std::string keyFilePath(keyDir + "/" + key.name());
        OutputFileStream ofs(std::move(keyFilePath));
        key.serialize(TextResourceWriter(ofs), keyHeaderComment);
    }

    std::cout << std::endl;
    return mapAnimations.size();
}

int32_t ExtractMaterials(const InputStream& istream, const std::string& outDir, bool convertMaterials , bool verbose)
{
    auto materials = CND::ReadMaterials(istream);

    std::string matDir;
    std::string bmpDir;
    if(!materials.isEmpty())
    {
        std:: cout << "Found materials: " << materials.size() << std::endl;

        matDir = outDir + "/" + "mat";
        MakePath(matDir);

        if(convertMaterials)
        {
            bmpDir = outDir + "/" + "bmp";
            MakePath(bmpDir);
        }
    }

    /* Save extracted materials to files */
    for(const auto& mat : materials)
    {
        std::cout << "Extracting material: " <<  mat.name() << std::endl;
        std::string matFilePath(matDir + "/" + mat.name());
        mat.serialize(OutputFileStream(std::move(matFilePath)));
        /*if(!SaveMaterialToFile(std::move(matFilePath), mat)) {
            return false;
        }*/

        if(verbose)
        {
            std::cout << "  ================== Material Info ===================" << std::endl;
            PrintMaterialInfo(mat);
        }

        /* Print material mipmaps info and convert to bmp */
        if(convertMaterials || verbose)
        {
            uint32_t mmIdx = 0;
            for(const auto& mipmap : mat.mipmaps())
            {
                if(verbose) {
                    PrintMipmapInfo(mipmap, mmIdx);
                }

                /* Save as bmp */
                if(convertMaterials)
                {
                    for(std::size_t mipmapIdx = 0; mipmapIdx < mipmap.size(); mipmapIdx++)
                    {
                        const std::string sufix = (mat.mipmaps().size() > 1 ? "_" + std::to_string(mmIdx) : "") + ".bmp";
                        const std::string infix = mipmap.size() > 1 ? "_" + std::to_string(mipmapIdx) : "";
                        const std::string fileName = bmpDir + "/" + GetBaseName(mat.name()) + infix + sufix;

                        if(!SaveBmpToFile(fileName, mipmap.at(mipmapIdx).toBmp())) {
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
        std::cout << std::endl;
    }

    return static_cast<int32_t>(materials.size());
}

int ExecCmdRemove(std::string_view scmd, const CndToolArgs& args)
{
    if(scmd == scmdAnimation) {
        return ExecCmdRemoveAnimation(args);
    }
    else if(scmd == scmdMaterial) {
        return ExecCmdRemoveMaterial(args);
    }

    if(scmd.empty()) {
        std::cerr << "ERROR: Subcommand requirded!\n\n";
    }
    else {
        std::cerr << "ERROR: Unknown subcommand \"" << scmd << "\"!\n\n";
    }

    PrintHelp(cmdRemove);
    return 1;
}

int ExecCmdRemoveAnimation(const CndToolArgs& args)
{
    bool success = ExecCmdRemoveAssets<Animation>(args,
        [](const auto& istream){
            return CND::ReadAnimations(istream);
        },
        [](const auto& cndFile, const auto& animations){
            return PatchCndAnimations(cndFile, animations);
        }
    );

    return int(!success);
}

int ExecCmdRemoveMaterial(const CndToolArgs& args)
{
    bool success = ExecCmdRemoveAssets<Material>(args,
        [](const auto& istream){
            return CND::ReadMaterials(istream);
        },
        [](const auto& cndFile, const auto& materials){
            return PatchCndMaterials(cndFile, materials);
        }
    );

    return int(!success);
}

void PrintMipmapInfo(const Mipmap& mipmap, uint32_t mmIdx)
{
    if(mipmap.empty()) return;
    const Texture& tex = mipmap.at(0);

    std::string colorMode;
    switch (tex.colorInfo().colorMode)
    {
    case 1:
        colorMode = "RGB_565";
        break;
    case 2:
        colorMode = "RGB_4444";
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
    std::cout << "    Pixel data size:" << SET_VINFO_LW(0) << GetMipmapPixelDataSize(mipmap.size(), tex.width(), tex.height(), tex.colorInfo().bpp) << std::endl;
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

void PrintErrorInvalidCnd(std::string_view cndPath, std::string_view cmd, std::string_view subcmd)
{
    if(cndPath.empty()) {
        std::cerr << "ERROR: A valid CND file path required!\n\n";
    } else {
        std::cerr << "ERROR: CND file \"" << cndPath << "\" does not exist!\n\n";
    }

    PrintHelp(cmd, subcmd);
}
