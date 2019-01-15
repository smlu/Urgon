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

#include "cmdutils/options.h"

#define SETW(n, f)  std::right << std::setfill(f) << std::setw(n)
#define SET_VINFO_LW(n) SETW(32 + n, '.')

#define OPT_OTPUT_DIR         "--output-dir"
#define OPT_OTPUT_DIR_SHORT   "-o"
#define OPT_MAT_PATCH         "--mat-patch"
#define OPT_MAT_PATCH_SHORT   "-mp"
#define OPT_CONVERT_MAT       "--bmp"
#define OPT_CONVERT_MAT_SHORT "-b"
#define OPT_VERBOSE           "--verbose"
#define OPT_VERBOSE_SHORT     "-v"
#define OPT_HELP              "--help"
#define OPT_HELP_SHORT        "-h"

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;

using namespace std::string_literals;
using namespace std::string_view_literals;


static constexpr auto kProgramName    = "cndext"sv;
static constexpr auto kProgramVersion = "0.2.0"sv;


void print_help();
void PrintMaterialInfo(const Material& mat);
void PrintMipmapInfo(const Mipmap& mipmap, uint32_t mmIdx);

bool ReplaceMaterial(const std::string& cndFile, std::vector<std::string> matFiles);
bool ExtractResources(const std::string& cndFile, std::string outDir, bool convertMaterials, bool verbose = false);

int main(int argc, const char *argv[])
{

    CmdArgs opt(argc, argv);
    if(argc < 2 ||
       opt.hasArg(OPT_HELP) ||
       opt.hasArg(OPT_HELP_SHORT) ||
       opt.unspecified().empty())
    {
        print_help();
        return 1;
    }

    std::string inputFile = opt.unspecified().at(0);
    if(!FileExists(inputFile))
    {
        std::cerr << "Error: File \"" << inputFile << "\" does not exist!";
        return 1;
    }

    std::string outDir;
    if(opt.hasArg(OPT_OTPUT_DIR_SHORT)){
        outDir = opt.arg(OPT_OTPUT_DIR_SHORT);
    }
    else if(opt.hasArg(OPT_OTPUT_DIR)){
        outDir = opt.arg(OPT_OTPUT_DIR);
    }

    const bool bVerboseOutput   = opt.hasArg(OPT_VERBOSE_SHORT) || opt.hasArg(OPT_VERBOSE) ;
    const bool bConvertMatToBmp = opt.hasArg(OPT_CONVERT_MAT)   || opt.hasArg(OPT_CONVERT_MAT_SHORT);
    const bool patchMats        = opt.hasArg(OPT_MAT_PATCH)     || opt.hasArg(OPT_MAT_PATCH_SHORT);

    int result = 0;
    try
    {
        /* Patch materials */
        if(patchMats)
        {
            auto matFiles  = opt.args(OPT_MAT_PATCH);
            auto matFiles2 = opt.args(OPT_MAT_PATCH_SHORT);
            matFiles.insert(matFiles.end(),
                std::make_move_iterator(matFiles2.begin()),
                std::make_move_iterator(matFiles2.end())
            );

            if(!ReplaceMaterial(inputFile, matFiles)) {
                result = 1;
            }
        }
        /* Extract animations & materials */
        else if(!ExtractResources(inputFile, std::move(outDir), bConvertMatToBmp, bVerboseOutput)) {
            result = 1;
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("An exception was encountered during %: %", patchMats ?
            "a CND file pathing": "the resources extraction", e.what()
        );
        result = 1;
    }

    return result;
}

void print_help()
{
    std::cout << "\nIndiana Jones and The Infernal Machine CND file extractor v" << kProgramVersion << std::endl;
    std::cout << "\nExtracts material and animation resources in CND file.\n";
    std::cout << "Can replace existing material with another material.\n\n\n";
    std::cout << "  Usage: cndext <cnd file> [options] ..." << std::endl << std::endl;

    std::cout << "Option        Long option        Meaning\n";
    std::cout << OPT_CONVERT_MAT_SHORT << SETW(17, ' ') << OPT_CONVERT_MAT << SETW(49, ' ') << "Convert extracted materials to bmp\n";
    std::cout << OPT_HELP_SHORT        << SETW(18, ' ') << OPT_HELP        << SETW(31, ' ') << "Show this message\n";
    std::cout << OPT_MAT_PATCH_SHORT   << SETW(22, ' ') << OPT_MAT_PATCH   << SETW(95, ' ') << "Replace materials in cnd file <material files>. No material is extracted from CND file\n";
    std::cout << OPT_OTPUT_DIR_SHORT   << SETW(24, ' ') << OPT_OTPUT_DIR   << SETW(34, ' ') << "Output folder <output dir>\n";
    std::cout << OPT_VERBOSE_SHORT     << SETW(21, ' ') << OPT_VERBOSE     << SETW(25, ' ') << "Verbose output\n";
}

void PrintMaterialInfo(const Material& mat)
{
    if(mat.mipmaps().empty()) return;
    std::cout << "    Total mipmaps:" << SET_VINFO_LW(2) << mat.mipmaps().size() << std::endl << std::endl;
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

bool ReplaceMaterial(const std::string& cndFile, std::vector<std::string> matFiles)
{
    bool bSuccess = false;
    if(!matFiles.empty())
    {
        for(const auto& matFile : matFiles)
        {
            Material mat{InputFileStream(matFile)};
            if(!CND::ReplaceMaterial(mat, cndFile)) {
                return false;
            }
        }

        LOG_INFO("CND file has been successfully patched!");
        bSuccess = true;
    }
    else {
        print_help();
    }

    return bSuccess;
}

std::size_t ExtractAnimations(const InputStream& istream, const std::string& outDir)
{
    istream.seek(0);
    auto vecAnimations = CND::ReadAnimations(istream);

    std::string keyDir;
    if(!vecAnimations.isEmpty())
    {
        LOG_INFO("Found animations: %", vecAnimations.size());
        keyDir = outDir + "/" + "key";
        MakePath(keyDir);
    }

    /* Save extracted animations to file */
    const std::string keyHeaderComment = [&]() {
        return "Extracted from CND file '"s + istream.name() + "' with " +
            std::string(kProgramName) + " v" + std::string(kProgramVersion);
    }();

    for(const auto& key : vecAnimations)
    {
        LOG_INFO("Extracting animation: %", key.name());

        std::string keyFilePath(keyDir + "/" + key.name());
        OutputFileStream ofs(std::move(keyFilePath));
        key.serialize(TextResourceWriter(ofs), keyHeaderComment);
    }

    LOG_INFO("");
    return vecAnimations.size();
}

int32_t ExtractMaterials(const InputStream& istream, std::string outDir, bool convertMaterials , bool verbose)
{
    istream.seek(0);
    auto materials = CND::ReadMaterials(istream);

    std::string matDir;
    std::string bmpDir;
    if(!materials.isEmpty())
    {
        LOG_INFO("Found materials: %", materials.size());

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
        LOG_INFO("Extracting material: %", mat.name());
        std::string matFilePath(matDir + "/" + mat.name());
        mat.serialize(OutputFileStream(std::move(matFilePath)));
        /*if(!SaveMaterialToFile(std::move(matFilePath), mat)) {
            return false;
        }*/

        if(verbose)
        {
            LOG_VERBOSE("  ================== Material Info ===================");
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
            LOG_VERBOSE("  =============== Material Info End =================\n\n");
        }
    }

    if(!verbose) {
        LOG_INFO("");
    }

    return static_cast<int32_t>(materials.size());
}

bool ExtractResources(const std::string& cndFile, std::string outDir, bool convertMaterials, bool verbose)
{
    InputFileStream ifstream(cndFile);
    outDir += (outDir.empty() ? "" : "/" ) + GetBaseName(cndFile);

    auto nExtAnimFiles = ExtractAnimations(ifstream, outDir);
    auto nExtMatFiles  = ExtractMaterials(ifstream, outDir, convertMaterials, verbose);
    if(nExtMatFiles < 0) return false;

    LOG_INFO("\n-----------------------------------------");
    LOG_INFO("Total extracted animations: %", nExtAnimFiles);
    LOG_INFO("Total extracted materials: %", nExtMatFiles);
    return true;
}
