#include <filesystem>
#include <iomanip>
#include <iostream>

#include <libim/io/vfstream.h>
#include <libim/common.h>
#include <libim/io/filestream.h>
#include <libim/log/log.h>
#include <cmdutils/cmdutils.h>
#include <cmdutils/options.h>
#include "config.h"

#define SETW(n, f)  std::right << std::setfill(f) << std::setw(n)
#define SET_FINFO_LW(n) SETW(10 + n, '.')

static constexpr auto OPT_OTPUT_DIR       ("--output-dir");
static constexpr auto OPT_OTPUT_DIR_SHORT ("-o");
static constexpr auto OPT_VERBOSE         ("--verbose");
static constexpr auto OPT_VERBOSE_SHORT   ("-v");
static constexpr auto OPT_HELP            ("--help");
static constexpr auto OPT_HELP_SHORT      ("-h");

using namespace cmdutils;
using namespace gobext;
using namespace libim;

namespace fs = std::filesystem;

void printHelp()
{
    std::cout << "Extracts resources from CND file!\n";
    std::cout << "  Usage: gobext <gob file> [options]" << std::endl << std::endl;

    std::cout << "Option        Long option        Meaning\n";
    std::cout << OPT_HELP_SHORT        << SETW(18, ' ') << OPT_HELP        << SETW(31, ' ') << "Show this message\n";
    std::cout << OPT_OTPUT_DIR_SHORT   << SETW(24, ' ') << OPT_OTPUT_DIR   << SETW(34, ' ') << "Output folder <output dir>\n";
    std::cout << OPT_VERBOSE_SHORT     << SETW(21, ' ') << OPT_VERBOSE     << SETW(25, ' ') << "Verbose output\n";
}

bool extractGob(const VfContainer c, const fs::path& outDir, const bool verbose)
{
    try
    {
        /* Save entries to files */
        for(const auto& [filePath, file] : c)
        {
            std::cout << "Extracting file: " << filePath << std::endl;

            /* Set entry file path */
            auto outPath = outDir / filePath;
            if(!makePath(outPath))
            {
                printError("Could not make file path %!", outPath);
                return false;
            }

            /* Open output file stream */
            OutputFileStream ofs(outPath, /*truncate=*/true);
            ofs.write(file);
        }

        std::cout << (!verbose ? "\n" : "") << "--------------------------\nTotal files extracted: " << c.size() << std::endl << std::endl;
        return true;
    }
    catch (const std::exception& e)
    {
        printError("An exception was thrown while extracting assets from GOB file: %", e.what());
        return false;
    }
}

int main(int argc, const char *argv[])
{
    gLogLevel = LogLevel::Error;

    std::cout << "\nIndiana Jones and The Infernal Machine GOB file extractor v" << kVersion << std::endl;
    std::cout << kProgramUrl << std::endl << std::endl;

    CmdArgs opt(argc, argv);
    if(argc < 2 ||
       opt.hasArg(OPT_HELP) ||
       opt.hasArg(OPT_HELP_SHORT) ||
       opt.positionalArgs().empty())
    {
        printHelp();
        return 1;
    }

    fs::path inputFile = opt.positionalArgs().at(0);
    if(!fileExists(inputFile))
    {
        printError("File % does not exists!", inputFile);
        return 1;
    }

    fs::path outdir;
    if (opt.hasArg(OPT_OTPUT_DIR_SHORT)) {
        outdir = opt.arg(OPT_OTPUT_DIR_SHORT);
    }
    else if (opt.hasArg(OPT_OTPUT_DIR)) {
        outdir = opt.arg(OPT_OTPUT_DIR);
    }

    bool bVerboseOutput = false;
    if (opt.hasArg(OPT_VERBOSE_SHORT)) {
        bVerboseOutput = true;
    }
    else if (opt.hasArg(OPT_VERBOSE)) {
        bVerboseOutput = true;
    }

    /* Extract files from gob file */
    int result = 0;
    try
    {
        auto vfs = gobLoad(inputFile);

        if (outdir == "")
        {
            outdir /= inputFile.stem().string() + "_GOB";
        }
        makePath(outdir);

        if(!extractGob(vfs, outdir, bVerboseOutput)) {
            result = 1;
        }

    }
    catch(const std::exception& e)
    {
        printError("Failed to read GOB file!\n  Error: %", e.what());
        result =  1;
    }
    return result;
}
