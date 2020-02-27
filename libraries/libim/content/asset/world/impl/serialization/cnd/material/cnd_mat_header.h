#ifndef LIBIM_CND_MAT_HEADER_H
#define LIBIM_CND_MAT_HEADER_H
#include "../cndstring.h"
#include "../../../../../material/colorformat.h"
#include <cstddef>

namespace libim::content::asset {
    struct CndMatHeader final
    {
        CndResourceName name;
        int width;
        int height;
        int mipmapCount;
        int texturesPerMipmap;
        ColorFormat colorInfo;
    };
}
#endif // LIBIM_CND_MAT_HEADER_H
