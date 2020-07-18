#ifndef LIBIM_COLORFORMAT_H
#define LIBIM_COLORFORMAT_H
#include <cstdint>

namespace libim::content::asset {
    enum class ColorMode : uint32_t
    {
        Indexed = 0,
        RGB     = 1,
        RGBA    = 2
    };

    struct ColorFormat final
    {
        ColorMode mode;
        uint32_t bpp;        // Color bit depth per pixel

        uint32_t redBPP  ;
        uint32_t greenBPP;
        uint32_t blueBPP ;

        uint32_t redShl  ;
        uint32_t greenShl;
        uint32_t blueShl ;

        uint32_t redShr  ;
        uint32_t greenShr;
        uint32_t blueShr ;

        uint32_t alphaBPP;
        uint32_t alphaShl;
        uint32_t alphaShr;
    };
    static_assert(sizeof(ColorFormat) == 56);


    // Predefined color formats
    // Naming is done in little-endian byte order.
    // Suffix 'be' names color format in big-endian byte order
    static constexpr ColorFormat RGB555     { ColorMode::RGB,   16,   5, 5, 5,   10,  5,  0,   3, 3, 3,   0,  0, 0 };
    static constexpr ColorFormat RGB555be   { ColorMode::RGB,   16,   5, 5, 5,    0,  5, 10,   3, 3, 3,   0,  0, 0 };

    static constexpr ColorFormat RGB565     { ColorMode::RGB,   16,   5, 6, 5,   11,  5,  0,   3, 2, 3,   0,  0, 0 };
    static constexpr ColorFormat RGB565be   { ColorMode::RGB,   16,   5, 6, 5,    0,  5, 11,   3, 2, 3,   0,  0, 0 };

    static constexpr ColorFormat RGBA4444   { ColorMode::RGBA,  16,   4, 4, 4,   12,  8,  4,   4, 4, 4,   4,  0, 4 };
    static constexpr ColorFormat RGBA4444be { ColorMode::RGBA,  16,   4, 4, 4,    0,  4,  8,   4, 4, 4,   4, 12, 4 };
    static constexpr ColorFormat ARGB4444   { ColorMode::RGBA,  16,   4, 4, 4,    8,  4,  0,   4, 4, 4,   4, 12, 4 };
    static constexpr ColorFormat ARGB4444be { ColorMode::RGBA,  16,   4, 4, 4,    4,  8, 12,   4, 4, 4,   4,  0, 4 };

    static constexpr ColorFormat RGBA5551   { ColorMode::RGBA,  16,   5, 5, 5,   11,  6,  1,   3, 3, 3,   1,  0, 7 };
    static constexpr ColorFormat RGBA5551be { ColorMode::RGBA,  16,   5, 5, 5,    0,  5, 10,   3, 3, 3,   1, 15, 7 };
    static constexpr ColorFormat ARGB1555   { ColorMode::RGBA,  16,   5, 5, 5,   10,  5,  0,   3, 3, 3,   1, 15, 7 };
    static constexpr ColorFormat ARGB1555be { ColorMode::RGBA,  16,   5, 5, 5,    1,  6, 11,   3, 3, 3,   1,  0, 7 };

    // RGB888
    static constexpr ColorFormat RGB24      { ColorMode::RGB,   24,   8, 8, 8,   16,  8,  0,   0, 0, 0,   0,  0, 0 };
    static constexpr ColorFormat RGB24be    { ColorMode::RGB,   24,   8, 8, 8,    0,  8, 16,   0, 0, 0,   0,  0, 0 };

    // RGBA8888
    static constexpr ColorFormat RGBA32     { ColorMode::RGBA,  32,   8, 8, 8,   24, 16,  8,   0, 0, 0,   8,  0, 0 };
    static constexpr ColorFormat RGBA32be   { ColorMode::RGBA,  32,   8, 8, 8,    0,  8, 16,   0, 0, 0,   8, 24, 0 };
    static constexpr ColorFormat ARGB32     { ColorMode::RGBA,  32,   8, 8, 8,   16,  8,  0,   0, 0, 0,   8, 24, 0 };
    static constexpr ColorFormat ARGB32be   { ColorMode::RGBA,  32,   8, 8, 8,    8, 16, 24,   0, 0, 0,   8,  0, 0 };

    // ColorFormat helper functions
    inline bool operator ==(const ColorFormat& lci, const ColorFormat& rci)
    {
        return lci.mode == rci.mode           &&
               lci.bpp  == rci.bpp            &&

               lci.redBPP == rci.redBPP       &&
               lci.redShl == rci.redShl       &&
               lci.redShr == rci.redShr       &&

               lci.greenBPP == rci.greenBPP   &&
               lci.greenShl == rci.greenShl   &&
               lci.greenShr == rci.greenShr   &&

               lci.blueBPP == rci.blueBPP     &&
               lci.blueShl == rci.blueShl     &&
               lci.blueShr == rci.blueShr     &&

               lci.alphaBPP == rci.alphaBPP   &&
               lci.alphaShl == rci.alphaShl   &&
               lci.alphaShr == rci.alphaShr;
    }

    inline bool operator !=(const ColorFormat& lci, const ColorFormat& rci) {
        return !(lci == rci);
    }
}
#endif // LIBIM_COLORFORMAT_H
