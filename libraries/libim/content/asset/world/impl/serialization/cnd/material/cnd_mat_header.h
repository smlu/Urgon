#ifndef LIBIM_CND_MAT_HEADER_H
#define LIBIM_CND_MAT_HEADER_H
#include "../cnd.h"
#include <cstddef>
#include <libim/content/asset/material/colorformat.h>

namespace libim::content::asset {
    struct CndMatHeader final
    {
        CndResourceName name;
        int width;
        int height;
        int celCount;           // number of cells
        int mipLevels;          // number of mipmap LOD levels in each texture
        ColorFormat colorInfo;
    };
    static_assert(sizeof(CndMatHeader) == 136);
}
#endif // LIBIM_CND_MAT_HEADER_H
