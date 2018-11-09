#ifndef DIAGNOSTIC_LOCATION_H
#define DIAGNOSTIC_LOCATION_H
#include <string_view>

namespace libim::text {
    struct diagnostic_location
    {
        std::string_view filename;
        uint32_t first_line;
        uint32_t first_col;
        uint32_t last_line;
        uint32_t last_col;
    };
}

#endif // DIAGNOSTIC_LOCATION_H
