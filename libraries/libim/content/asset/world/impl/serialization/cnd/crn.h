#ifndef LIBIM_CNDRESOURCENAME_H
#define LIBIM_CNDRESOURCENAME_H
#include <cstdint>
#include <libim/types/fixed_string.h>

namespace libim::content::asset {
    static constexpr std::size_t kCndMaxNameLen = 64;
    using CndResourceName = FixedString<kCndMaxNameLen>;
}
#endif // LIBIM_CNDRESOURCENAME_H
