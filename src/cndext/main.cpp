#include <iomanip>
#include <iostream>
#ifdef _WIN32
# include <direct.h>
# define MKDIR(dir) mkdir(dir)
# else
# include <sys/stat.h>
# define MKDIR(dir) mkdir(dir,0775)
#endif

#include "libim/material/bmp.h"
#include "libim/material/mat.h"
#include "libim/cnd.h"
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

void print_help()
{
    std::cout << "\nIndiana Jones and The Infernal Machine CND file extractor\n";
    std::cout << "Extracts or replaces material resources in CND file!\n";
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
    std::cout << "    Width:"   << SET_VINFO_LW(10) << tex.width()<< std::endl;
    std::cout << "    Height:"  << SET_VINFO_LW(9)  << tex.width()<< std::endl;
    std::cout << "    Mipmap textures:" << SET_VINFO_LW(0) << mipmap.size() << std::endl;
    std::cout << "    Color info:\n";

    auto cmLw = colorMode.size() /2;
    cmLw = (colorMode.size()  % 8 == 0 ? cmLw -1 : cmLw);
    std::cout << "      Color mode:" << SET_VINFO_LW(cmLw) << colorMode << std::endl;
    std::cout << "      Bit depth:"  << SET_VINFO_LW(4) << tex.colorInfo().bpp << std::endl;
    std::cout << "      Bit depth per channel:" << std::endl;
    std::cout << "        Red:"   << SET_VINFO_LW(8) << tex.colorInfo().redBPP << std::endl;
    std::cout << "        Green:" << SET_VINFO_LW(6) << tex.colorInfo().greenBPP << std::endl;
    std::cout << "        Blue:"  << SET_VINFO_LW(7) << tex.colorInfo().blueBPP << std::endl;
    std::cout << "        Alpha:" << SET_VINFO_LW(6) << tex.colorInfo().alphaBPP << std::endl;
    std::cout << "      Left shift per channel:" << std::endl;
    std::cout << "        Red:"   << SET_VINFO_LW(8) << tex.colorInfo().RedShl << std::endl;
    std::cout << "        Green:" << SET_VINFO_LW(6) << tex.colorInfo().GreenShl << std::endl;
    std::cout << "        Blue:"  << SET_VINFO_LW(7) << tex.colorInfo().BlueShl << std::endl;
    std::cout << "        Alpha:" << SET_VINFO_LW(6) << tex.colorInfo().AlphaShl << std::endl;
    std::cout << "      Right shift per channel:" << std::endl;
    std::cout << "        Red:"   << SET_VINFO_LW(8) << tex.colorInfo().RedShr << std::endl;
    std::cout << "        Green:" << SET_VINFO_LW(6) << tex.colorInfo().GreenShr << std::endl;
    std::cout << "        Blue:"  << SET_VINFO_LW(7) << tex.colorInfo().BlueShr << std::endl;
    std::cout << "        Alpha:" << SET_VINFO_LW(6) << tex.colorInfo().AlphaShr << std::endl << std::endl;
}

int main(int argc, const char *argv[])
{
    Options opt(argc, argv);
    if(argc < 2 ||
       opt.hasOpt(OPT_HELP) ||
       opt.hasOpt(OPT_HELP_SHORT) ||
       opt.unspecified().empty())
    {
        print_help();
        return 1;
    }

    std::string inputFile = opt.unspecified().at(0);
    std::string outdir;
    if(opt.hasOpt(OPT_OTPUT_DIR_SHORT)){
        outdir = opt.arg(OPT_OTPUT_DIR_SHORT);
    }
    else if(opt.hasOpt(OPT_OTPUT_DIR)){
        outdir = opt.arg(OPT_OTPUT_DIR);
    }

    bool bVerboseOutput = false;
    if(opt.hasOpt(OPT_VERBOSE_SHORT)){
        bVerboseOutput = true;;
    }
    else if(opt.hasOpt(OPT_VERBOSE)){
        bVerboseOutput = true;
    }

    bool bConvertMatToBmp = false;
    if(opt.hasOpt(OPT_CONVERT_MAT) || opt.hasOpt(OPT_CONVERT_MAT_SHORT)){
        bConvertMatToBmp = true;
    }

    std::string matOutPath;
    std::string bmpOutPath;
    if(opt.hasOpt(OPT_MAT_PATCH) || opt.hasOpt(OPT_MAT_PATCH_SHORT))
    {
        auto args = opt.args(OPT_MAT_PATCH);
        auto args2 = opt.args(OPT_MAT_PATCH_SHORT);
        args.insert(args.end(),
                    std::make_move_iterator(args2.begin()),
                    std::make_move_iterator(args2.end()));

        if(!args.empty())
        {
             for(const auto& matFile : args)
             {
                auto mat = LoadMaterialFromFile(matFile);
                if(!mat) return 1;

                if(!ReplaceMaterialInCndFile(*mat, inputFile)) {
                    return 1;
                }
             }

             std::cout << "CND file has been successfully patched!\n";
        } else {
            print_help();
        }
    }
    else // Extract materials
    {
        auto materials = LoadMaterialsFromCndFile(inputFile);
        if(!materials.empty())
        {
            std::cout << "Found materials: " << materials.size()<< std::endl;
            outdir += (outdir.empty() ? "" : std::string(PATH_SEP_CH,1)) + GetBaseNameFromPath(inputFile);
            MKDIR(outdir.c_str());

            matOutPath = outdir + PATH_SEP_CH + "mat";
            MKDIR(matOutPath.c_str());

            if(bConvertMatToBmp)
            {
                bmpOutPath = outdir + PATH_SEP_CH + "bmp";
                MKDIR(bmpOutPath.c_str());
            }
        }

        /* Save extracted materials to files */
        for(const auto& mat : materials)
        {
            std::cout << "Extracting material: " << mat.name() << std::endl;
            SaveMaterialToFile(mat, matOutPath + PATH_SEP_CH + mat.name());

            if(bVerboseOutput)
            {
                std::cout << "  ================== Material Info ===================\n";
                PrintMaterialInfo(mat);
            }

            /* Print material mipmaps info and convert to bmp */
            uint32_t mmIdx = 0;
            for(const auto& mipmap : mat.mipmaps())
            {
                if(bVerboseOutput) {
                    PrintMipmapInfo(mipmap, mmIdx);
                }

                /* Save as bmp */
                if(bConvertMatToBmp)
                {
                    std::string sufix = (mat.mipmaps().size() < 2) ? ".bmp" : "_" + std::to_string(mmIdx) + ".bmp";
                    SaveBmpToFile(bmpOutPath + PATH_SEP_CH + GetBaseNameFromPath(mat.name())+ sufix, mipmap.at(0).toBmp()); // extract first texture
                }

                mmIdx++;
            }

            if(bVerboseOutput) {
                std::cout << "  =============== Material Info End =================\n\n\n";
            }
        }

        std::cout << (!bVerboseOutput ? "\n" : "") << "--------------------------\nTotal materials extracted: " << materials.size() << std::endl << std::endl;
    }

    return 0;
}
