#ifndef LIBIM_CND_MAT_HEADER_H
#define LIBIM_CND_MAT_HEADER_H
#include "../cnd.h"
#include "../../../../../material/colorformat.h"
#include <cstddef>
namespace libim::content::asset {
    constexpr static std::size_t kCndMatNameLen = kCndMaxNameLen;
    struct CndMatHeader final
    {
        char name[kCndMatNameLen] = {0};
        int width;
        int height;
        int mipmapCount;
        int texturesPerMipmap;
        ColorFormat colorInfo;
    };
}
#endif // LIBIM_CND_MAT_HEADER_H
