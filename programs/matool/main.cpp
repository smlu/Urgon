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

constexpr static auto cmdCreate  = "create"sv;
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
constexpr static auto optExtractLod        = "--mipmap"sv;
constexpr static auto optNoSRGB            = "--no-srgb"sv;
constexpr static auto optOutput            = "--output"sv;
constexpr static auto optOutputDir         = "--output-dir"sv;
constexpr static auto optOutputShort       = "-o"sv;
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
    if (it == kEncodingToColorFormat.end()) {
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
    if (args.hasArg(optOutputShort)){
        return args.arg(optOutputShort);
    }
    else if (args.hasArg(optOutputDir)){
        return args.arg(optOutputDir);
    }
    return optPath.value_or(fs::path());
}

fs::path getOptOutput(const MatoolArgs& args, std::optional<fs::path> optPath = std::nullopt)
{
    if (args.hasArg(optOutputShort)){
        return args.arg(optOutputShort);
    }
    else if (args.hasArg(optOutput)){
        return args.arg(optOutput);
    }
    return optPath.value_or(fs::path());
}


void printHelp(std::string_view cmd = ""sv, std::string_view subcmd = ""sv)
{
    if (cmd == cmdCreate)
    {
        if (subcmd == scmdBatch)
        {
            std::cout << "Create new MAT files in bulk from BMP or PNG image files using existing MAT files as a reference." << std::endl << std::endl;
            std::cout << "By default, the new MAT file is created only if a corresponding reference MAT file can be found," << std::endl;
            std::cout << "and the number of images to go into the new MAT file match the referenced MAT file." << std::endl;
            std::cout << "The images to be used in the new MAT file must have the same size (width, height), and" << std::endl;
            std::cout << "the file  name must be suffixed with '__cel_'x  where 'x' represents the positional number" << std::endl;
            std::cout << "to place the image in the new MAT file starting with 0." << std::endl;
            std::cout << "  e.g.: test__cel_0.png test__cel_1.bmp test__cel_2.png ..." << std::endl << std::endl;
            std::cout << "  Usage: matool create batch [options] <path-to-image-folder> <path-to-mat-ref-folder>" << std::endl << std::endl;

            printOptionHeader();
            printOption( optSRGB     , ""             , "Do sRGB conversion when generating MipMaps."       );
            printOption( ""          , ""             , "By default, no sRGB conversion is done."           );
            printOption( optForce8bpc, ""             , "Use RGB24 or RGBA32 color format for encoding"     );
            printOption( ""          , ""             , "instead of color format from referenced MAT file." );
            printOption( optOutputDir, optOutputShort , "Output folder"                                     );
            printOption( optVerbose  , optVerboseShort, "Verbose printout to the console\n"                 );
        }
        else
        {
            std::cout << "Create a new MAT file using BMP or PNG images." << std::endl;
            std::cout << "Images to use in the new MAT file must all have the same size (width, height)," << std::endl;
            std::cout << "or the generation will fail." << std::endl << std::endl;
            std::cout << "  Usage: matool create [options] <encoding> <path-to-image> [path-to-images] ..." << std::endl;
            std::cout << "         matool create <sub-command>" << std::endl << std::endl;

            printPositionalArgHeader("Encoding");
            printPositionalArg( "rgb555"    , "Encode images in RGB555 format"                           );
            printPositionalArg( "rgb555be"  , "Encode images in big-endian byte order RGB555 format\n"   );
            printPositionalArg( "rgb565"    , "Encode images in RGB565 format"                           );
            printPositionalArg( "rgb565be"  , "Encode images in big-endian byte order RGB565 format\n"   );
            printPositionalArg( "rgba4444"  , "Encode images in RGBA4444 format"                         );
            printPositionalArg( "rgba4444be", "Encode images in big-endian byte order RGBA4444 format"   );
            printPositionalArg( "argb4444"  , "Encode images in ARGB4444 format"                         );
            printPositionalArg( "argb4444be", "Encode images in big-endian byte order ARGB4444 format\n" );
            printPositionalArg( "rgba5551"  , "Encode images in RGBA5551 format"                         );
            printPositionalArg( "rgba5551be", "Encode images in big-endian byte order RGBA5551 format"   );
            printPositionalArg( "argb1555"  , "Encode images in ARGB1555 format"                         );
            printPositionalArg( "argb1555be", "Encode images in big-endian byte order ARGB1555 format\n" );
            printPositionalArg( "rgb24"     , "Encode images in RGB888 format"                           );
            printPositionalArg( "rgb24be"   , "Encode images in big-endian byte order RGB888 format\n"   );
            printPositionalArg( "rgba32"    , "Encode images in RGBA8888 format"                         );
            printPositionalArg( "rgba32be"  , "Encode images in big-endian byte order RGBA8888 format"   );
            printPositionalArg( "argb32"    , "Encode images in ARGB8888 format"                         );
            printPositionalArg( "argb32be"  , "Encode images in big-endian byte order ARGB8888 format\n" );

            printOptionHeader();
            printOption( optExtractLod, ""             , "Number of mipmap levels to generate."                        );
            printOption( ""           , ""             , "If no value is provided or value is 0,"                      );
            printOption( ""           , ""             , "then max number of levels will be generated."                );
            printOption( ""           , ""             , "If this option is not provided MipMap won't be generated.\n" );
            printOption( optNoSRGB    , ""             , "No sRGB conversion when generating MipMap.\n"                );
            printOption( optOutput    , optOutputShort , "Output file"                                                 );
            printOption( optVerbose   , optVerboseShort, "Verbose printout to the console\n"                           );

            printSubCommandHeader();
            printSubCommand( scmdBatch, "Create multiple MAT files in bulk" );
        }
    }
    else if (cmd == cmdExtract)
    {
        std::cout << "Extract images from MAT files." << std::endl;
        std::cout << "  Usage: matool extract [options] <path-mat-file|folder>" << std::endl << std::endl;
        printOptionHeader();
        printOption( optExtractAsBmp, optExtractAsBmpShort, "Extract images in BMP format."                                       );
        printOption( ""             , ""                  , "By default, images are extracted in PNG format."                     );
        printOption( optMaxTex      , ""                  , "Max number of cel images to extract from MAT file."                  );
        printOption( ""             , ""                  , "By default, all images are extracted."                               );
        printOption( optExtractLod  , ""                  , "Extract also mipmap LOD images from MAT file."                       );
        printOption( ""             , ""                  , "By default, only top image at LOD 0 is extracted from each texture." );
        printOption( optOutputDir   , optOutputShort      , "Output folder"                                                       );
        printOption( optVerbose     , optVerboseShort     , "Verbose printout to the console"                                     );
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

        printOptionHeader("Mod Option");
        printOption( optEncoding   , optEncodingShort, "Change encoding color format."                       );
        printOption( ""            , ""              , "Supported formats:"                                  );
        printOption( ""            , ""              , "  rgb24    | rgb24be    "                            );
        printOption( ""            , ""              , "  rgb555   | rgb555be   | rgb565   | rgb565be"       );
        printOption( ""            , ""              , "  rgba4444 | rgba4444be | argb4444 | argb4444be"     );
        printOption( ""            , ""              , "  rgba5551 | rgba5551be | argb1555 | argb1555be"     );
        printOption( ""            , ""              , "  rgba32   | rgba32be   | argb32   | argb32be\n"     );
        printOption( optExtractLod , ""              , "Change number of mipmap levels."                     );
        printOption( ""            , ""              , "If no value is provided or value is 0,"              );
        printOption( ""            , ""              , "then the mipmap chain with max number of LOD levels" );
        printOption( ""            , ""              , "will be generated.\n"                                );

        printOptionHeader();
        printOption( optNoSRGB , ""             , "No sRGB conversion when generating mipmap." );
        printOption( optVerbose, optVerboseShort, "Verbose printout to the console"            );
    }
    else
    {
        std::cout << "Command-line interface tool for MAT image format.\n\n";
        std::cout << "  Usage: matool <command> [sub-command] [options]" << std::endl << std::endl;
        printCommandHeader();
        printCommand( cmdCreate , "Create new MAT file"                             );
        printCommand( cmdExtract, "Extract images from MAT file"                    );
        printCommand( cmdInfo   , "Print to the console information about MAT file" );
        printCommand( cmdModify , "Modify existing MAT file"                        );
        printCommand( cmdHelp   , "Show this message"                               );
    }
}

void printMatFilePathError(const fs::path& matPath, std::string_view cmd = ""sv, std::string_view subcmd = ""sv)
{
    if (matPath.empty()) {
        printError("A valid MAT file path required!\n");
    } else {
        printError(" MAT file % doesn't exist!\n", matPath);
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
                tex = bmpLoad(InputFileStream(file));
            }
            else if (fileExtMatch(file, kExtPng)) {
                tex = pngLoad(InputFileStream(file));
            }
            else {
                throw std::runtime_error("Invalid image file '" + file.u8string() + "'");
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

int execCmdCreateBatch(const MatoolArgs& args)
{
    if (args.positionalArgs().size() < 3)
    {
        printError("Missing positional arguments for paths to the image and mat reference folders!\n");
        printHelp(cmdCreate, scmdBatch);
        return 1;
    }

    auto imgFolder = args.positionalArgs().at(1);
    if (!dirExists(imgFolder))
    {
        printError("Invalid or non-existing path to the image folder!\n");
        return 1;
    }

    if (args.positionalArgs().size() < 2)
    {
        printError("Missing positional argument for path to the MAT reference folder!\n");
        return 1;
    }

    auto matFolder = args.positionalArgs().at(2);
    if (!dirExists(matFolder))
    {
        printError("Invalid or non-existing path to the MAT reference folder!\n");
        return 1;
    }
    else if (matFolder == imgFolder)
    {
        printError("Path to the images folder is the same as path to ref MAT folder!\n");
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
                std::cout << "No images were found to generate new MAT files!\n";
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
                printError("No reference MAT files found in folder: %\n", matFolder);
                return 1;
            }
        }

        /* Convert found images to MAT */
        fs::path outDir = getOptOutputDir(args, "out_mat");
        for (auto[idx, pair] : enumerate(mapImgs))
        {
            printProgress("Generating... ", idx + 1, mapImgs.size());

            auto&[name, setImgPaths] = pair;
            auto itMat = mapMats.find(name);
            if (itMat == mapMats.end())
            {
                std::cerr << "\r";
                printError("No reference MAT file found for image: %", *setImgPaths.begin());
                continue; // Skip generation process
            }

            /* Load Material and verify found images*/
            const auto refMat = matLoad(InputFileStream(itMat->second));
            if (refMat.isEmpty())
            {
                std::cerr << "\r";
                printError("Reference MAT file is empty: %", name);
                if (hasOptVerbose(args))
                {
                    std::cerr << "       " << "Skipped images: " << std::endl;
                    for (const auto& img : setImgPaths) {
                        std::cerr << "         "  << img << std::endl;
                    }
                }
                continue; // Skip generation process
            }
            else if (setImgPaths.size() < refMat.count())
            {
                std::cerr << "\r";
                printError("Not enough images found to create a new MAT file: %", name);
                if (hasOptVerbose(args)) {
                    std::cerr << "       " << "Ref MAT tex count: " << refMat.count() << " found images: " << setImgPaths.size() << std::endl;
                }
                continue; // Skip generation process
            }
            else if (setImgPaths.size() > refMat.count()) // Too many images for ref Material?
            {
                std::cerr
                    << "\rWARNING: Not all images will be used due to more images found than needed by the referenced MAT file: "
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
            matWrite(mat, OutputFileStream(outPath, /*truncate=*/true));
        }

        std::cout << "\nGenerating... FINISHED\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << std::endl;
        printError("An exception was encountered while generating MAT files!");
        if (hasOptVerbose(args)) {
            std::cerr << "       error: " << e.what() << std::endl;
        }
        return 1;
    }
}

int execCmdCreate(const MatoolArgs& args)
{
    try
    {
        ColorFormat cf;
        if (args.subcmd() == scmdBatch) {
            return execCmdCreateBatch(args);
        }
        else if (args.positionalArgs().empty())
        {
            printError("Missing positional argument for encoding or sub-command!\n");
            printHelp(cmdCreate);
            return 1;
        }
        else if (!getColorFormat(args.positionalArgs().at(0), cf))
        {
            printError("Invalid encoding argument or sub-command: '%'\n", args.subcmd());
            printHelp(cmdCreate);
            return 1;
        }

        const auto& imgFiles = args.imageFiles();
        if (imgFiles.empty())
        {
            printError("Missing position argument of image file paths!\n");
            printHelp(cmdCreate);
            return 1;
        }

        std::cout << "Creating MAT file... " << std::flush;
        const auto mipLevels = args.hasArg(optExtractLod) ? args.uintArg(optExtractLod, 0) : 1;
        const auto bSRGBConv = !args.hasArg(optNoSRGB);
        const auto matName   = imgFiles.begin()->filename().replace_extension(kExtMat).u8string();
        auto mat = imagesToMaterial(imgFiles, matName, cf, safe_cast<uint32_t>(mipLevels), bSRGBConv);

        fs::path outPath = getOptOutput(args);
        if (outPath.empty()) {
            outPath = mat.name();
        }

        makePath(outPath);
        matWrite(mat, OutputFileStream(outPath, /*truncate=*/true));
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
            printError("%", e.what());
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
            printError("Invalid positional argument for input MAT file|folder path!\n");
            printHelp(cmdExtract);
            return 1;
        }

        fs::path outDir = getOptOutputDir(args, "extracted");
        if (!isDirPath(outDir))
        {
            printError("Output dir path is not directory!");
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
            printError("Option '%' must not be 0!", optMaxTex);
            return 1;
        }

        /* Extract images from material files */
        makePath(outDir);
        if (!bVerbose) std::cout << "Extracting... " << std::flush;
        for (const auto [idx, file] : enumerate(matFiles))
        {
            auto mat = matLoad(InputFileStream(file));
            const uint64_t numImgs = min<std::size_t>(maxCelCount, mat.count());
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
        if (hasOptVerbose(args))
        {
            std::cerr << std::endl;
            printError("%", e.what());
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

        auto mat = matLoad(InputFileStream(inputFile));
        matPrintInfo(mat);
        return 0;
    }
    catch (const std::exception& e)
    {
        printError("Invalid MAT file!");
        if (hasOptVerbose(args)) {
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
        if (args.hasArg(optEncodingShort) || args.hasArg(optEncoding))
        {
            ColorFormat cf;
            auto enc = args.hasArg(optEncodingShort) ? args.arg(optEncodingShort) : args.arg(optEncoding);
            if (!getColorFormat(enc, cf))
            {
                printError("Invalid encoding argument '%'!\n", enc);
                printHelp(cmdCreate);
                return 1;
            }
            optFormat = std::move(cf);
        }

        const auto optMipLevels = args.hasArg(optExtractLod) ? std::optional<uint32_t>(args.uintArg(optExtractLod, 0)) : std::nullopt;
        const auto bSRGBConv    = !args.hasArg(optNoSRGB);

        if (!optMipLevels && !optFormat)
        {
            printError("Missing an option!\n");
            printHelp(cmdModify);
            return 1;
        }

        std::cout << "Updating... " << std::flush;

        auto imat = matLoad(InputFileStream(inputFile));
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

        matWrite(omat, OutputFileStream(inputFile));
        std::cout << kSuccess << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << kFailed << std::endl;
        if (hasOptVerbose(args)) {
            printError("%", e.what());
        }
        return 1;
    }
}

int execCmd(std::string_view cmd, const MatoolArgs& args)
{
    if (cmd == cmdCreate) {
        return execCmdCreate(args);
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
        if (cmd != cmdHelp) {
            printError("Unknown command '%'\n", cmd);
        }
        auto scmd = args.positionalArgs().size() > 1 ? args.positionalArgs().at(1) : "";
        printHelp(args.subcmd(), scmd);
    }

    return 1;
}

int main(int argc, const char* argv[])
{
    try
    {
        std::cout << "\nIndiana Jones and The Infernal Machine MAT file tool v" << kVersion << std::endl;
        std::cout << kProgramUrl << std::endl << std::endl;

        MatoolArgs args(argc, argv);
        if (argc < 2)
        {
            printHelp();
            return 1;
        }

        return execCmd(args.cmd(), args);
    }
    catch (const std::exception& e)
    {
        printError("%", e.what());
        return 1;
    }
}