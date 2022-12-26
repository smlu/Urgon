#ifndef LIBIM_PARSELOCATION_H
#define LIBIM_PARSELOCATION_H
#include <string_view>

namespace libim::text {
    struct ParseLocation
    {
        std::string_view filename;
        std::size_t firstLine;
        std::size_t firstColumn;
        std::size_t lastLine;
        std::size_t lastColumn;
    };
}

#endif // LIBIM_PARSE_LOCATION_H
