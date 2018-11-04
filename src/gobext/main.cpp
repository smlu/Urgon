#include <fstream>
#include <iomanip>
#include <iostream>

#include "libim/gob.h"
#include "libim/common.h"
#include "libim/io/filestream.h"
#include "cmdutils/options.h"

#define SETW(n, f)  std::right << std::setfill(f) << std::setw(n)
#define SET_FINFO_LW(n) SETW(10 + n, '.')

static constexpr auto OPT_OTPUT_DIR       ("--output-dir");
static constexpr auto OPT_OTPUT_DIR_SHORT ("-o");
static constexpr auto OPT_VERBOSE         ("--verbose");
static constexpr auto OPT_VERBOSE_SHORT   ("-v");
static constexpr auto OPT_HELP            ("--help");
static constexpr auto OPT_HELP_SHORT      ("-h");

void print_help();
bool ExtractGob(std::shared_ptr<const GobFileDirectory> gobDir, std::string outDir, const bool verbose);

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
    if(!FileExists(inputFile)) 
    {
        std::cerr << "Error: File \"" << inputFile << "\" does not exists!"; 
        return 1;
    }
    
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
    int result = 0;
    auto gobDir = LoadGobFromFile(inputFile);
    if(gobDir)
    {
        outdir += (outdir.empty() ? "" : "/") + GetBaseName(inputFile) + "_GOB";
        MakePath(outdir);

        if(!ExtractGob(gobDir, outdir, bVerboseOutput)) {
            result = 1;
        }
    }
    else
    {
        std::cerr << "Error reading GOB file!\n";
        result =  1;
    }

    return result;
}

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

bool ExtractGob(std::shared_ptr<const GobFileDirectory> gobDir, std::string outDir, const bool verbose)
{
    try
    {
        /* Save entries to files */
        for(const auto& entry : gobDir->entries)
        {
            std::cout << "Extracting file: " << entry.name << std::endl;
            if(verbose)
            {
                std::string strSize = std::to_string(entry.size);
                std::cout << "  offset in gob:"  << SET_FINFO_LW((14 - (strSize.size() + 6)) + 11) << std::hex << std::showbase << entry.offset << std::endl;
                std::cout << "  file size:"      << SET_FINFO_LW(14) << std::dec << strSize<< " bytes\n";
            }

            /* Seek to entry offset */
            gobDir->stream->seek(entry.offset);

            /* Set entry file path */
            auto outPath = outDir + '/' + entry.name;
            if(!MakePath(outPath))
            {
                std::cerr << "Error: could not make file path: " << outPath << "!\n";
                return 1;
            }

            /* Open output file stream */
            OutputFileStream ofs(outPath);

            /* Write entry to file */
            ByteArray buffer(4096);
            std::size_t offEntryEnd = entry.offset + entry.size;
            std::size_t nWritten = 0;

            while(gobDir->stream->tell() < offEntryEnd && !gobDir->stream->atEnd())
            {
                if(gobDir->stream->tell() + buffer.size() >= offEntryEnd) {
                    buffer.resize(offEntryEnd - gobDir->stream->tell());
                }

                if(gobDir->stream->read(reinterpret_cast<byte_t*>(buffer.data()), buffer.size()) != buffer.size())
                {
                    std::cerr << "Error reading GOB entry into buffer!\n";
                    return false;
                }

                ofs.write(buffer);
                nWritten += buffer.size();
            }

            if(verbose) {
                std::cout << "  bytes written to disk:" << SET_FINFO_LW(2) << std::dec << nWritten << " bytes\n\n";
            }

            if(nWritten < entry.size) {
                std::cerr << "  Warning: not all bytes were written to disk!\n\n";
            } else if(nWritten > entry.size) {
                std::cerr << "  Warning: too many bytes were written to disk!\n\n";
            }
        }

        std::cout << (!verbose ? "\n" : "") << "--------------------------\nTotal files extracted: " << gobDir->entries.size() << std::endl << std::endl;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "An exception was thrown while extracting GOB dir: " << e.what() << std::endl;
        return false;
    }
}
