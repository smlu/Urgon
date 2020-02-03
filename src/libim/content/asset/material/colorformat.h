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

        ColorMode colorMode; // RGB = 1, RGBA = 2
        uint32_t bpp;        // Bit depth per pixel

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

    static constexpr ColorFormat RGB_565   { ColorMode::RGB,   16,   5, 6, 5,   11,  5,  0,   3, 2, 3,   0,  0, 0 };
    static constexpr ColorFormat RGBA_4444 { ColorMode::RGBA,  16,   4, 4, 4,   12,  8,  4,   4, 4, 4,   4,  0, 4 };
    static constexpr ColorFormat ARGB_4444 { ColorMode::RGBA,  16,   4, 4, 4,    8,  4,  0,   4, 4, 4,   4, 12, 4 };
    static constexpr ColorFormat ARGB_5551 { ColorMode::RGBA,  16,   5, 5, 5,   10,  5,  0,   3, 3, 3,   1, 16, 7 };
    static constexpr ColorFormat RGB_888   { ColorMode::RGB,   24,   8, 8, 8,   16,  8,  0,   0, 0, 0,   0,  0, 0 };
    static constexpr ColorFormat BGR_888   { ColorMode::RGB,   24,   8, 8, 8,    0,  8, 16,   0, 0, 0,   0,  0, 0 };
    static constexpr ColorFormat RGBA_8888 { ColorMode::RGBA,  32,   8, 8, 8,   24, 16,  0,   0, 0, 0,   8,  0, 0 };
}
#endif // LIBIM_COLORFORMAT_H
