#ifndef LIBIM_CND_MAT_HEADER_H
#define LIBIM_CND_MAT_HEADER_H
#include "../../../../../material/colorformat.h"
#include <cstddef>
namespace libim::content::asset {
    constexpr static std::size_t kCndMatNameLen = 64;
    struct CndMatHeader final
    {
        char name[kCndMatNameLen];
        int width;
        int height;
        int mipmapCount;
        int texturesPerMipmap;
        ColorFormat colorInfo;
    };
}
#endif // LIBIM_CND_MAT_HEADER_H
