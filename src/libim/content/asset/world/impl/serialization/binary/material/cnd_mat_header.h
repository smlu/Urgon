#ifndef LIBIM_CND_MAT_HEADER_H
#define LIBIM_CND_MAT_HEADER_H
#include "../../../../../material/colorformat.h"

namespace libim::content::asset {
    struct CndMatHeader final
    {
        char name[64];
        int width;
        int height;
        int mipmapCount;
        int texturesPerMipmap;
        ColorFormat colorInfo;
    };
}
#endif // LIBIM_CND_MAT_HEADER_H
