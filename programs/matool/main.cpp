#include <cmath>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include <cmdutils/cmdutils.h>

#include <libim/common.h>
#include <libim/content/asset/material/material.h>
#include <libim/content/asset/material/texture.h>
#include <libim/content/asset/material/texture_view.h>
#include <libim/content/asset/material/texutils.h>
#include <libim/io/filestream.h>
#include <libim/math/math.h>
#include <libim/utils/utils.h>

#include "config.h"
#include "matoolargs.h"
#include "utils.h"


using namespace cmdutils;
using namespace matool;
using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace fs = std::filesystem;

constexpr static auto cmdConvert = "convert"sv;
constexpr static auto cmdExtract = "extract"sv;
constexpr static auto cmdHelp    = "help"sv;
constexpr static auto cmdInfo    = "info"sv;
constexpr static auto cmdModify  = "modify"sv;

constexpr static auto scmdBatch  = "batch"sv;

constexpr static auto optExtractAsBmp      = "--bmp"sv;
constexpr static auto optExtractAsBmpShort = "-b"sv;
constexpr static auto optEncoding          = "--encoding"sv;
constexpr static auto optEncodingShort     = "-e"sv;
constexpr static auto optMaxTex            = "--max-tex"sv;
constexpr static auto optExtractLod            = "--mipmap"sv;
constexpr static auto optNoSRGB            = "--no-srgb"sv;
constexpr static auto optOutputDir         = "--output-dir"sv;
constexpr static auto optOutputDirShort    = "-o"sv;
constexpr static auto optSRGB              = "--srgb"sv;
constexpr static auto optForce8bpc         = "--force-8bpc"sv;
constexpr static auto optVerbose           = "--verbose"sv;
constexpr static auto optVerboseShort      = "-v"sv;


[[maybe_unused]] constexpr static auto kFailed  = "FAILED"sv;
constexpr static auto kSuccess = "SUCCESS"sv;


const auto kEncodingToColorFormat = std::map<std::string, ColorFormat> {
    { "rgb555"     , RGB555     },
    { "rgb555be"   , RGB555be   },
    { "rgb565"     , RGB565     },
    { "rgb565be"   , RGB565be   },
    { "rgba4444"   , RGBA4444   },
    { "rgba4444be" , RGBA4444be },
    { "argb4444"   , ARGB4444   },
    { "argb4444be" , ARGB4444be },
    { "rgba5551"   , RGBA5551   },
    { "rgba5551be" , RGBA5551be },
    { "argb1555"   , ARGB1555   },
    { "argb1555be" , ARGB1555be },
    { "rgb24"      , RGB24      },
    { "rgb24be"    , RGB24be    },
    { "rgba32"     , RGBA32     },
    { "rgba32be"   , RGBA32be   },
    { "argb32"     , ARGB32     },
    { "argb32be"   , ARGB32be   },
};

bool getColorFormat(std::string cfstr, ColorFormat& cf) {
    to_lower(cfstr);
    auto it = kEncodingToColorFormat.find(cfstr);
    if(it == kEncodingToColorFormat.end()) {
        return false;
    }

    cf = it->second;
    return true;
}

bool hasOptVerbose(const MatoolArgs& args)
{
    return args.hasArg(optVerbose) || args.hasArg(optVerboseShort);
}

fs::path getOptOutputDir(const MatoolArgs& args, std::optional<fs::path> optPath = std::nullopt)
{
    if (args.hasArg(optOutputDirShort)){
        return args.arg(optOutputDirShort);
    }
    else if(args.hasArg(optOutputDir)){
        return args.arg(optOutputDir);
    }
    return optPath.value_or(fs::path());
}

void printHelp(std::string_view cmd = ""sv, std::string_view subcmd = ""sv)
{
    if (cmd == cmdConvert)
    {
        if (subcmd == scmdBatch)
        {
            std::cout << "Convert BMP or PNG image files to MAT file format in bulk using existing MAT files as a reference." << std::endl << std::endl;
            std::cout << "By default images are converted only if coresponding reference MAT file can be found" << std::endl;
            std::cout << "and the number of images to go into MAT file match the referenced MAT file." << std::endl;
            std::cout << "The images to be used in one MAT file must have the same size (width, height) and" << std::endl;
            std::cout << "file name must be suffixed with '__cel_'x, where x is the position number in the MAT file starting with 0." << std::endl;
            std::cout << "  e.g.: test__cel_0.png test__cel_1.bmp test__cel_2.png ..." << std::endl << std::endl;
            std::cout << "  Usage: matool convert batch [options] <path-to-image-folder> <path-to-mat-ref-folder>" << std::endl << std::endl;

            printOptionHeader();
            printOption( optSRGB        , ""                  , "Do sRGB conversion when generating MipMaps."       );
            printOption( ""             , ""                  , "By default no sRGB conversion is done."            );
            printOption( optForce8bpc   , ""                  , "Use RGB24 or RGBA32 color format for encoding"     );
            printOption( ""             , ""                  , "instead of color format from referenced MAT file." );
            printOption( optOutputDir   , optOutputDirShort   , "Output folder"                                     );
            printOption( optVerbose     , optVerboseShort     , "Verbose printout to the console\n"                 );
        }
        else
        {
            std::cout << "Convert BMP or PNG image files to MAT file format." << std::endl;
            std::cout << "Images to convert must have the same size (width, height) or conversion will fail." << std::endl << std::endl;
            std::cout << "  Usage: matool convert [options] <encoding> <path-to-image> [path-to-images] ..." << std::endl << std::endl;

            printPositionalArgHeader("Encoding");
            printPositionalArg( "rgb555"    , "Encode textures in RGB555 format"                           );
            printPositionalArg( "rgb555be"  , "Encode textures in big-endian byte order RGB555 format\n"   );
            printPositionalArg( "rgb565"    , "Encode textures in RGB565 format"                           );
            printPositionalArg( "rgb565be"  , "Encode textures in big-endian byte order RGB565 format\n"   );
            printPositionalArg( "rgba4444"  , "Encode textures in RGBA4444 format"                         );
            printPositionalArg( "rgba4444be", "Encode textures in big-endian byte order RGBA4444 format"   );
            printPositionalArg( "argb4444"  , "Encode textures in ARGB4444 format"                         );
            printPositionalArg( "argb4444be", "Encode textures in big-endian byte order ARGB4444 format\n" );
            printPositionalArg( "rgba5551"  , "Encode textures in RGBA5551 format"                         );
            printPositionalArg( "rgba5551be", "Encode textures in big-endian byte order RGBA5551 format"   );
            printPositionalArg( "argb1555"  , "Encode textures in ARGB1555 format"                         );
            printPositionalArg( "argb1555be", "Encode textures in big-endian byte order ARGB1555 format\n" );
            printPositionalArg( "rgb24"     , "Encode textures in RGB888 format"                           );
            printPositionalArg( "rgb24be"   , "Encode textures in big-endian byte order RGB888 format\n"   );
            printPositionalArg( "rgba32"    , "Encode textures in RGBA8888 format"                         );
            printPositionalArg( "rgba32be"  , "Encode textures in big-endian byte order RGBA8888 format"   );
            printPositionalArg( "argb32"    , "Encode textures in ARGB8888 format"                         );
            printPositionalArg( "argb32be"  , "Encode textures in big-endian byte order ARGB8888 format\n" );

            printOptionHeader();
            printOption( optExtractLod   , ""               , "Number of MipMap levels to generate."                        );
            printOption( ""          , ""               , "If no value is provided or value is 0 then"                  );
            printOption( ""          , ""               , "max number of levels will be generated."                     );
            printOption( ""          , ""               , "If this option is not provided MipMap won't be generated.\n" );
            printOption( optNoSRGB   , ""               , "No sRGB conversion when generating MipMap.\n"                );
            printOption( optOutputDir, optOutputDirShort, "Output folder"                                               );
            printOption( optVerbose  , optVerboseShort  , "Verbose printout to the console\n"                           );

            printSubCommandHeader();
            printSubCommand( scmdBatch, "Convert images in bulk" );
        }
    }
    else if (cmd == cmdExtract)
    {
        std::cout << "Extract images from MAT files." << std::endl;
        std::cout << "  Usage: matool extract [options] <path-mat-file|folder>" << std::endl << std::endl;
        printOptionHeader();
        printOption( optExtractAsBmp, optExtractAsBmpShort, "Extract images in BMP format."                                      );
        printOption( ""             , ""                  , "By default images are extrected in PNG format."                     );
        printOption( optMaxTex      , ""                  , "Max number of textures to extract from MAT file."                   );
        printOption( ""             , ""                  , "By default all are extracted."                                      );
        printOption( optExtractLod      , ""                  , "Extract also mipmap LOD images from MAT file."                      );
        printOption( ""             , ""                  , "By default only top image at LOD 0 is extracted from each texture." );
        printOption( optOutputDir   , optOutputDirShort   , "Output folder"                                                      );
        printOption( optVerbose     , optVerboseShort     , "Verbose printout to the console"                                    );
    }
    else if (cmd == cmdInfo)
    {
        std::cout << "Print MAT file information to the console" << std::endl << std::endl;
        std::cout << "  Usage: matool info [options] <mat-file-path>" << std::endl << std::endl;
        printOptionHeader();
        printOption( optVerbose, optVerboseShort, "Verbose printout to the console" );
    }
    else if (cmd == cmdModify)
    {
        std::cout << "Modify and update existing MAT file.\nAt least one mod option must be provided." << std::endl << std::endl;
        std::cout << "  Usage: matool modify <options> <mat-file-path>" << std::endl << std::endl;

        std::cout << "Mod Option:    Long option:        Description:\n";
        printOption( optEncoding, optEncodingShort, "Change encoding color format."                                   );
        printOption( ""          , ""             , "Supported formats:"                                              );
        printOption( ""          , ""             , "  rgb24    | rgb24be    "                                        );
        printOption( ""          , ""             , "  rgb555   | rgb555be   | rgb565   | rgb565be"                   );
        printOption( ""          , ""             , "  rgba4444 | rgba4444be | argb4444 | argb4444be"                 );
        printOption( ""          , ""             , "  rgba5551 | rgba5551be | argb1555 | argb1555be"                 );
        printOption( ""          , ""             , "  rgba32   | rgba32be   | argb32   | argb32be\n"                 );
        printOption( optExtractLod   , ""             , "Change number of MipMap levels."                                 );
        printOption( ""          , ""             , "If no value is provided or value is 0 then"                      );
        printOption( ""          , ""             , "MipMap chain with max number of LOD levels will be generated.\n" );

        printOptionHeader();
        printOption( optNoSRGB   , ""             , "No sRGB conversion when generating MipMap." );
        printOption( optVerbose, optVerboseShort  , "Verbose printout to the console"            );
    }
    else
    {
        std::cout << "Command line interface tool for MAT image format.\n\n";
        std::cout << "  Usage: matool <command> [sub-command] [options]" << std::endl << std::endl;
        printCommandHeader();
        printCommand( cmdConvert, "Convert image file to MAT file"                  );
        printCommand( cmdExtract, "Extract images from MAT file"                    );
        printCommand( cmdInfo   , "Print to the console information about MAT file" );
        printCommand( cmdModify , "Modify existing MAT file" );
        printCommand( cmdHelp   , "Show this message"                               );
    }
}

void printMatFilePathError(const fs::path& matPath, std::string_view cmd = ""sv, std::string_view subcmd = ""sv)
{
    if (matPath.empty()) {
        std::cerr << "ERROR: A valid MAT file path required!\n\n";
    } else {
        std::cerr << "ERROR: MAT file \"" << matPath << "\" does not exist!\n\n";
    }
    printHelp(cmd, subcmd);
}

// @param mipLevels == 1 means no mipmap will be generated.
//        mipLevels == 0 means full mipmap chain will be generated.
//        Any other number defines number of mipmaps.
Material imagesToMaterial(const std::vector<fs::path>& imgFiles, std::string matName, const ColorFormat& cf, uint32_t mipLevels, bool bSRGBConv)
{
    Material mat;
    mat.setName(std::move(matName));
    for (const auto& file : imgFiles)
    {
        try
        {
            if (!fileExists(file)) {
                throw std::runtime_error("No such file: '" + file.u8string() + "'");
            }

            Texture tex;
            if (fileExtMatch(file, kExtBmp)) {
                tex = bmpReadTexture(InputFileStream(file));
            }
            else if (fileExtMatch(file, kExtPng)) {
                tex = pngReadTexture(InputFileStream(file));
            }
            else {
                throw std::runtime_error("ERROR: Invalid image file '" + file.u8string() + "'");
            }

            if (mipLevels != 1)
            {
                tex.generateMipmaps(
                    mipLevels == 0 ? std::nullopt : std::optional(mipLevels),
                    cf,
                    bSRGBConv
                );
            }

            tex.convert(cf);
            mat.addCel(std::move(tex));
        }
        catch (const StreamError& e) {
            throw std::runtime_error("Error loading image file: '" + file.u8string() + "'");
        }
        catch (const TextureError&) { // catch exception when creating tex or adding cel to material
            throw std::runtime_error("Corrupted image: '" + file.u8string() + "'");
        }
        catch (const MaterialError&) { // catch exception when creating tex or adding cel to material
            throw std::runtime_error("Image '" + file.u8string() + "' has different resolution than the first image");
        }
    }

    return mat;
}

int execCmdConvertBatch(const MatoolArgs& args)
{
    if (args.positionalArgs().size() < 3)
    {
        std::cerr << "ERROR: missing positional arguments for paths to the image and mat reference folders!\n\n";
        printHelp(cmdConvert, scmdBatch);
        return 1;
    }

    auto imgFolder = args.positionalArgs().at(1);
    if (!dirExists(imgFolder))
    {
        std::cerr << "ERROR: invalid or non-existing path to the image folder!\n\n";
        return 1;
    }

    if (args.positionalArgs().size() < 2)
    {
        std::cerr << "ERROR: missing positional argument for path to the MAT reference folder!\n\n";
        return 1;
    }

    auto matFolder = args.positionalArgs().at(2);
    if (!dirExists(matFolder))
    {
        std::cerr << "ERROR: invalid or non-existing path to the MAT reference folder!\n\n";
        return 1;
    }
    else if (matFolder == imgFolder)
    {
        std::cerr << "ERROR: " << "Path to the images folder is the same as path to ref MAT folder!";
        return 1;
    }

    try
    {
        /* Find images in provided image directory */
        std::map<std::string, std::set<fs::path>> mapImgs;
        {
            for (const auto& entry : fs::directory_iterator(imgFolder))
            {
                if (entry.is_regular_file())
                {
                    const auto& path = entry.path();
                    if (fileExtMatch(path, kExtPng) || fileExtMatch(path, kExtPng))
                    {
                        std::string name = path.stem().u8string();
                        if (!hasLodSeqSuf(name))
                        {
                            name = removeSeqSuffix(name);
                            mapImgs[name + std::string(kExtMat)].insert(path);
                        }
                        else {
                            std::cout << "INFO: Skipping image file because assuming it's MipMap LOD image: " << name << std::endl;
                        }
                    }
                }
            }

            if (mapImgs.empty())
            {
                std::cout << "No images found to convert to MAT!\n";
                return 0;
            }
        }

        /* Find MAT files in provided mat directory */
        std::map<std::string, fs::path> mapMats;
        {
            for (const auto& entry : fs::directory_iterator(matFolder))
            {
                if (entry.is_regular_file())
                {
                    const auto& path = entry.path();
                    if (fileExtMatch(path, kExtMat)) {
                        mapMats[path.filename().u8string()] = path;
                    }
                }
            }
            if (mapMats.empty())
            {
                std::cout << "ERROR: No reference MAT files found in folder: " << matFolder << std::endl << std::endl;
                return 1;
            }
        }

        /* Convert found images to mat */
        fs::path outDir = getOptOutputDir(args, "out_mat");
        for (auto[idx, pair]: enumerate(mapImgs))
        {
            printProgress("Converting... ", idx + 1, mapImgs.size());

            auto&[name, setImgPaths] = pair;
            auto itMat = mapMats.find(name);
            if (itMat == mapMats.end())
            {
                std::cerr << "\rERROR: No reference MAT file found for image: "
                          << *setImgPaths.begin() << std::endl;
                continue; // Skip converting
            }

            /* Load Material and verify found images*/
            const auto refMat = Material(InputFileStream(itMat->second));
            if (refMat.isEmpty())
            {
                std::cerr << "\rERROR: Reference MAT file is empty: " << name << std::endl;
                if (hasOptVerbose(args))
                {
                    std::cerr << "       " << "Skipped images: " << std::endl;
                    for (const auto& img : setImgPaths) {
                        std::cerr << "         "  << img << std::endl;
                    }
                }
                continue; // Skip converting
            }
            else if (setImgPaths.size() < refMat.count())
            {
                std::cerr << "\rERROR: Not enough images found to convert to MAT file: " << name << std::endl;
                if (hasOptVerbose(args)) {
                    std::cerr << "       " << "Ref MAT tex count: " << refMat.count() << " found images: " << setImgPaths.size() << std::endl;
                }
                continue; // Skip converting
            }
            else if (setImgPaths.size() > refMat.count()) // Too many images for ref Material?
            {
                std::cerr
                    << "\rWARNING: Not all images will be converted because too many images were found to convert to reference MAT file: "
                    << name << std::endl;
                if (hasOptVerbose(args))
                {
                    std::cerr << "         " << "Ref MAT tex count: " << refMat.count() << " found images: " << setImgPaths.size() << std::endl;
                    std::cerr << "         " << "Skipped images:" << std::endl;
                    auto itr = std::next(setImgPaths.begin(), refMat.count());
                    while (itr != setImgPaths.end())
                    {
                        std::cerr << "           " << *itr << std::endl;
                        itr = setImgPaths.erase(itr);
                    }
                }
            }

            /* Make new mat file */
            const fs::path outPath = outDir / name;
            const auto mipLevels   = refMat.cel(0).mipLevels();
            const auto& format     = args.hasArg(optForce8bpc)
                                    ? (refMat.format().mode == ColorMode::RGB ? RGB24 : RGBA32)
                                    : refMat.format();
            const auto imgPaths  = std::vector<fs::path>(setImgPaths.begin(), setImgPaths.end());
            auto mat = imagesToMaterial(imgPaths, name, format, mipLevels, args.hasArg(optSRGB));

            /* Save to file */
            makePath(outPath);
            mat.serialize(OutputFileStream(outPath, /*truncate=*/true));
        }

        std::cout << "\rConverting... FINISHED\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nERROR: An exception was encountered while converting images to MAT file format!\n";
        if (hasOptVerbose(args)) {
            std::cerr << "       error: " << e.what() << std::endl;
        }
        return 1;
    }
}

int execCmdConvert(const MatoolArgs& args)
{
    try
    {
        ColorFormat cf;
        if (args.subcmd() == scmdBatch) {
            return execCmdConvertBatch(args);
        }
        else if (args.positionalArgs().empty())
        {
            std::cerr << "ERROR: Missing encoding argument!\n\n";
            printHelp(cmdConvert);
            return 1;
        }
        else if (!getColorFormat(args.positionalArgs().at(0), cf))
        {
            std::cerr << "ERROR: Invalid encoding argument '" << args.subcmd() << "'!\n\n";
            printHelp(cmdConvert);
            return 1;
        }

        const auto& imgFiles = args.imageFiles();
        if (imgFiles.empty())
        {
            std::cerr << "ERROR: Missing position argument of image file paths!\n\n";
            printHelp(cmdConvert);
            return 1;
        }

        std::cout << "Converting images... " << std::flush;
        const auto mipLevels = args.hasArg(optExtractLod) ? args.uintArg(optExtractLod, 0) : 1;
        const auto bSRGBConv = !args.hasArg(optNoSRGB);
        const auto matName   = imgFiles.begin()->filename().replace_extension(kExtMat).u8string();
        auto mat = imagesToMaterial(imgFiles, matName, cf, safe_cast<uint32_t>(mipLevels), bSRGBConv);

        fs::path outPath = getOptOutputDir(args);
        outPath = outPath / mat.name();
        makePath(outPath);
        mat.serialize(OutputFileStream(outPath, /*truncate=*/true));
        std::cout << kSuccess << std::endl;

        if (hasOptVerbose(args)) {
            matPrintInfo(mat);
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << kFailed << std::endl;
        if (hasOptVerbose(args)) {
            std::cerr << "ERROR: " << e.what() << std::endl;
        }
        return 1;
    }
}

int execCmdExtract(const MatoolArgs& args)
{
    try
    {
        fs::path input = args.matFile();
        if (input.empty() && !args.positionalArgs().empty()) {
            input = args.positionalArgs().at(0);
        }

        if (!fileExists(input) && !dirExists(input))
        {
            std::cerr << "ERROR: Invalid positional argument for file|folder path!\n\n";
            printHelp(cmdExtract);
            return 1;
        }

        fs::path outDir = getOptOutputDir(args, "extracted");
        if (!isDirPath(outDir))
        {
            std::cerr << "ERROR: Output dir path is not directory!\n";
            return 1;
        }

        std::vector<fs::path> matFiles;
        if (isFilePath(input)) {
            matFiles.push_back(input);
        }
        else // Folder
        {
            matFiles.reserve(256);
            for (const auto& entry : fs::directory_iterator(input))
            {
                if (entry.is_regular_file())
                {
                    const auto& path = entry.path();
                    if (fileExtMatch(path, kExtMat)) {
                        matFiles.push_back(path);
                    }
                }
            }
        }

        /* Get additional options */
        const bool bVerbose        = hasOptVerbose(args);
        const bool bExtractAsBmp   = args.hasArg(optExtractAsBmpShort) || args.hasArg(optExtractAsBmp);
        const bool bExtractLod     = args.hasArg(optExtractLod);
        const uint64_t maxCelCount = args.uintArg(optMaxTex, std::numeric_limits<uint64_t>::max());
        if (maxCelCount == 0)
        {
            std::cerr << "ERROR: Option '" << optMaxTex << "' must not be 0!\n";
            return 1;
        }

        /* Extract images from material files */
        makePath(outDir);
        if (!bVerbose) std::cout << "Extracting... " << std::flush;
        for (const auto [idx, file] : enumerate(matFiles))
        {
            auto mat = Material(InputFileStream(file));
            const uint64_t numImgs = min(maxCelCount, mat.count());
            if (bVerbose) {
                std::cout << "Extracting " << numImgs << " image(s) from file: "
                          << file.filename() << " ... " << std::flush;
            }
            else if (matFiles.size() > 1) {
                printProgress("Extracting... ", idx + 1, matFiles.size());
            }

            matExtractImages(mat, outDir, numImgs, bExtractLod, bExtractAsBmp);
            if (bVerbose) std::cout << kSuccess << std::endl;
        }

        if (!bVerbose) std::cout << "\rExtracting... "<< kSuccess << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << kFailed << std::endl;
        if (hasOptVerbose(args)) {
            std::cerr << "\nERROR: " << e.what() << std::endl;
        }
        return 1;
    }

    printHelp(cmdExtract);
    return 1;
}

int execCmdInfo(const MatoolArgs& args)
{
    try
    {
        const auto inputFile = args.matFile();
        if (!fileExists(inputFile))
        {
            printMatFilePathError(inputFile, cmdInfo);
            return 1;
        }

        auto mat = Material(InputFileStream(inputFile));
        matPrintInfo(mat);
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: Invalid MAT file!" << std::endl;
        if(hasOptVerbose(args)) {
            std::cerr << "Reason: " << e.what() << std::endl;
        }
        return 1;
    }
}

int execCmdModify(const MatoolArgs& args)
{
    try
    {
        const auto inputFile = args.matFile();
        if (!fileExists(inputFile))
        {
            printMatFilePathError(inputFile, cmdModify);
            return 1;
        }


        std::optional<ColorFormat> optFormat;
        if ( args.hasArg(optEncodingShort) || args.hasArg(optEncoding))
        {
            ColorFormat cf;
            auto enc = args.hasArg(optEncodingShort) ? args.arg(optEncodingShort) : args.arg(optEncoding);
            if (!getColorFormat(enc, cf))
            {
                std::cerr << "ERROR: Invalid encoding argument '" << enc << "'!\n\n";
                printHelp(cmdConvert);
                return 1;
            }
            optFormat = std::move(cf);
        }

        const auto optMipLevels = args.hasArg(optExtractLod) ? std::optional<uint32_t>(args.uintArg(optExtractLod, 0)) : std::nullopt;
        const auto bSRGBConv    = !args.hasArg(optNoSRGB);

        if (!optMipLevels && !optFormat)
        {
            std::cerr << "ERROR: Missing an option!\n\n";
            printHelp(cmdModify);
            return 1;
        }

        std::cout << "Updating... " << std::flush;

        auto imat = Material(InputFileStream(inputFile));
        auto omat = Material(imat.name());
        for (auto tex : imat.cells())
        {
            if (optMipLevels)
            {
                tex.generateMipmaps(
                    optMipLevels.value() == 0 ? std::nullopt : optMipLevels, // if 0 generate full mipmap chain
                    optFormat,
                    bSRGBConv
                );
            }
            else if (optFormat) {
                tex.convert(optFormat.value());
            }
            omat.addCel(std::move(tex));
        }

        omat.serialize(OutputFileStream(inputFile));
        std::cout << kSuccess << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << kFailed << std::endl;
        if(hasOptVerbose(args)) {
            std::cerr << "ERROR: " << e.what() << std::endl;
        }
        return 1;
    }
}

int execCmd(std::string_view cmd, const MatoolArgs& args)
{
    if (cmd == cmdConvert) {
        return execCmdConvert(args);
    }
    else if (cmd == cmdExtract) {
        return execCmdExtract(args);
    }
    else if (cmd == cmdInfo) {
        return execCmdInfo(args);
    }
    else if (cmd == cmdModify) {
        return execCmdModify(args);
    }
    else
    {
        if(cmd != cmdHelp) {
            std::cerr << "ERROR: Unknown command\n\n";
        }
        auto scmd = args.positionalArgs().size() > 1 ?  args.positionalArgs().at(1) : "";
        printHelp(args.subcmd(), scmd);
    }

    return 1;
}

int main(int argc, const char *argv[])
{
    try
    {
        std::cout << "\nIndiana Jones and The Infernal Machine MAT file tool v" << kVersion << std::endl;
        MatoolArgs args(argc, argv);
        if(argc < 2)
        {
            printHelp();
            return 1;
        }

        return execCmd(args.cmd(), args);
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}