#include <fstream>
#include <iomanip>
#include <iostream>
#ifdef _WIN32
# include <direct.h>
# define MKDIR(dir) mkdir(dir)
# else
# include <sys/stat.h>
# define MKDIR(dir) mkdir(dir,0775)
#endif

#include "libim/gob.h"
#include "cmdutils/options.h"

#define SETW(n, f)  std::right << std::setfill(f) << std::setw(n)
#define SET_FINFO_LW(n) SETW(10 + n, '.')

#define OPT_OTPUT_DIR         "--output-dir"
#define OPT_OTPUT_DIR_SHORT   "-o"
#define OPT_VERBOSE           "--verbose"
#define OPT_VERBOSE_SHORT     "-v"
#define OPT_HELP              "--help"
#define OPT_HELP_SHORT        "-h"

void print_help()
{
    std::cout << "\nIndiana Jones and The Infernal Machine GOB file extractor\n";
    std::cout << "Extracts resources from CND file!\n";
    std::cout << "  Usage: gobext <gob file> [options]" << std::endl << std::endl;

    std::cout << "Option        Long option        Meaning\n";
    std::cout << OPT_HELP_SHORT        << SETW(18, ' ') << OPT_HELP        << SETW(31, ' ') << "Show this message\n";
    std::cout << OPT_OTPUT_DIR_SHORT   << SETW(24, ' ') << OPT_OTPUT_DIR   << SETW(34, ' ') << "Output folder <output dir>\n";
    std::cout << OPT_VERBOSE_SHORT     << SETW(21, ' ') << OPT_VERBOSE     << SETW(25, ' ') << "Verbose output\n";
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

    /* Extract files from gob file */
    auto gobDir = LoadGobFromFile(inputFile);
    if(gobDir)
    {
        outdir += (outdir.empty() ? "" : std::string(&PATH_SEP_CH, 1)) + GetBaseNameFromPath(inputFile) + "_GOB" + PATH_SEP_CH;
        MKDIR(outdir.c_str());
    }

    /* Save entries to files */
    for(const auto& entry : gobDir->entries)
    {
        std::cout << "Extracting file: " << entry.name << std::endl;
        if(bVerboseOutput)
        {
            std::string strSize = std::to_string(entry.size);
            std::cout << "  offset in gob:"  << SET_FINFO_LW((14 - (strSize.size() + 6)) + 11) << std::hex << std::showbase << entry.offset << std::endl;
            std::cout << "  file size:"      << SET_FINFO_LW(14) << std::dec << strSize<< " bytes\n";
        }

        /* Seek to entry offset */
        if(!gobDir->stream.seekg(entry.offset) || gobDir->stream.peek() == EOF)
        {
            std::cerr << "Error extracting gob entry, failed to seek to the entry offset: " << IosErrorStr(gobDir->stream) << "\n!";
            return 1;
        }

        /* Set entry  file path */
        std::string outPath(GetNativePath(outdir + entry.name));
        auto nPos = outPath.rfind(PATH_SEP_CH);
        if(nPos != std::string::npos) {
            MKDIR(outPath.substr(0, nPos).c_str());
        }

        /* Open output file stream */
        std::ofstream ofs(outPath, std::ios::out | std::ios::trunc | std::ios::binary);
        if (!ofs.is_open())
        {
            std::cerr << "Error opening file stream for writing entry!\n";
            return false;
        }

        /* Write entry to file */
        std::vector<char> buffer(4096);
        std::ifstream::pos_type offEntryEnd = entry.offset + entry.size;
        std::size_t nWritten = 0;
        while(gobDir->stream.tellg() < offEntryEnd && !gobDir->stream.eof())
        {
            if(std::size_t(gobDir->stream.tellg()) + buffer.size() >= offEntryEnd) {
                buffer.resize(offEntryEnd - gobDir->stream.tellg());
            }

            if(!gobDir->stream.read(buffer.data(), buffer.size()))
            {
                std::cerr << "Error reading GOB entry into buffer: " << IosErrorStr(gobDir->stream) << "!\n";
                return false;
            }

            if(!ofs.write(buffer.data(), buffer.size()))
            {
                std::cerr << "Error writing GOB entry to file: " << IosErrorStr(ofs) << "!\n";
                return false;
            }

            nWritten += buffer.size();
        }

        if(bVerboseOutput) {
            std::cout << "  bytes written to disk:" << SET_FINFO_LW(2) << std::dec << nWritten << " bytes\n\n";
        }

        if(nWritten < entry.size) {
            std::cerr << "  Warning: not all bytes were written to disk!\n\n";
        } else if(nWritten > entry.size) {
            std::cerr << "  Warning: too many bytes were written to disk!\n\n";
        }
    }

    std::cout << (!bVerboseOutput ? "\n" : "") << "--------------------------\nTotal files extracted: " << gobDir->entries.size() << std::endl << std::endl;
    return 0;
}
