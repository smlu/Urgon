#ifndef LIBIM_RESOURCE_LITERALS_H
#define LIBIM_RESOURCE_LITERALS_H
#include "../../../text/impl/schars.h"
#include <string_view>

namespace libim::content::text {
    static constexpr std::string_view kResSectionHeader = "###############";
    static constexpr std::string_view kResSectionTag    = "SECTION";
    static constexpr std::string_view kResLabelPunc     = ":";
}
#endif // LIBIM_RESOURCE_LITERALS_H
