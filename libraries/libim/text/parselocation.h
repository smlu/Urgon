#ifndef LIBIM_PARSELOCATION_H
#define LIBIM_PARSELOCATION_H
#include <string_view>

namespace libim::text {
    struct ParseLocation
    {
        std::string_view filename;
        std::size_t first_line;
        std::size_t first_col;
        std::size_t last_line;
        std::size_t last_col;
    };
}

#endif // LIBIM_PARSE_LOCATION_H
