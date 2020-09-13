#ifndef CMDUTILS_H
#define CMDUTILS_H
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

#include <libim/math/math.h>
#include <libim/types/safe_cast.h>
#include <libim/utils/utils.h>

/**
 * Header file provides cmd utility functions for printing
 * program commands and options to the console
 */

#define CMDUTILS_SETW(n, f)  std::right << std::setfill(f) << std::setw(libim::safe_cast<int>(n))

namespace cmdutils {

    inline void printCommandHeader(std::string_view t = "Command", std::size_t s = 43)
    {
        auto [min, max] = libim::minmax(t.size(), s);
        std::cout << t << ":" << CMDUTILS_SETW(max - min, ' ') << "Description:\n";
    }

    inline void printCommand(std::string_view cmd, std::string_view desc, std::size_t s = 31)
    {
        auto[min, max] = libim::minmax(cmd.size(), desc.size() + s);
        std::cout << "  " << cmd << CMDUTILS_SETW(max - min, ' ') << desc << std::endl;
    }

    inline void printSubCommandHeader()
    {
        printCommandHeader("Sub-command", 46);
    }

    inline void printSubCommand(std::string_view scmd, std::string_view desc)
    {
        printCommand(scmd, desc, 34);
    }

    inline void printPositionalArgHeader(std::string_view name, std::size_t width = 46)
    {
        printCommandHeader(name, width);
    }

    inline void printPositionalArg(std::string_view arg, std::string_view desc, std::size_t width = 34)
    {
        printCommand(arg, desc, width);
    }

    inline void printOptionHeader(std::string_view t = "Option")
    {
        auto[min, max] = libim::minmax<std::size_t>(t.size(), 45);
        std::cout << t << ": " << CMDUTILS_SETW(max - min, ' ') << "Short:       Description:\n";
    }

    static void printOption(std::string_view option, std::string_view shortOption, std::string_view desc)
    {
        using namespace libim;
        const int  lo  = safe_cast<int>(option.size());
        const int  lso = safe_cast<int>(shortOption.size());
        const int  ld  = safe_cast<int>(desc.size());
        const auto lw  = lo + 2;
        const auto mw  = std::abs((lso + 21) - lo);
        const auto rw  = std::abs((ld  + 13) - lso);

        std::cout << CMDUTILS_SETW(lw, ' ') << option
                  << CMDUTILS_SETW(mw, ' ') << shortOption
                  << CMDUTILS_SETW(rw, ' ') << desc
                  << std::endl;
    }

    inline void printProgress(std::string_view message, std::size_t progress, std::size_t total)
    {
        std::cout << "\r" << message << std::fixed << std::setprecision(2)
            << (double(progress) / double(total)) * 100.00  << "%" << std::flush;
    }

    template<typename ...Args>
    inline void printError(std::string_view errorMsg, Args&& ... args)
    {
        std::cerr << "ERROR: ";
        libim::utils::ssprintf(std::cerr, errorMsg, std::forward<Args>(args)...);
        std::cerr << "\n";
    }
}

#endif //CMDUTILS_H