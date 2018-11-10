#ifndef DIAGNOSTIC_LOCATION_H
#define DIAGNOSTIC_LOCATION_H
#include <string_view>

namespace libim::text {
    struct diagnostic_location
    {
        std::string_view filename;
        std::size_t first_line;
        std::size_t first_col;
        std::size_t last_line;
        std::size_t last_col;
    };
}

#endif // DIAGNOSTIC_LOCATION_H
