#ifndef LIBIM_COLORFORMAT_H
#define LIBIM_COLORFORMAT_H
#include <cstdint>

namespace libim {
    struct ColorFormat
    {
        int32_t colorMode; // RGB565 = 1, RGBA4444 = 2
        int32_t bpp      ; // Bit depth per pixel

        int32_t redBPP  ;
        int32_t greenBPP;
        int32_t blueBPP ;

        int32_t RedShl  ;
        int32_t GreenShl;
        int32_t BlueShl ;

        int32_t RedShr  ;
        int32_t GreenShr;
        int32_t BlueShr ;

        int32_t alphaBPP;
        int32_t AlphaShl;
        int32_t AlphaShr;
    };

    static constexpr ColorFormat RGB_565   { 1, 16, 5, 6, 5, 11, 5, 0, 3, 2, 3, 0,  0, 0 };
    static constexpr ColorFormat RGBA_4444 { 2, 16, 4, 4, 4, 12, 8, 4, 4, 4, 4, 4,  0, 4 };
    static constexpr ColorFormat ARGB_4444 { 2, 16, 4, 4, 4,  8, 4, 0, 4, 4, 4, 4, 12, 4 };
    static constexpr ColorFormat ARGB_5551 { 2, 16, 5, 5, 5, 10, 5, 0, 3, 3, 3, 1, 16, 7 };
}
#endif // LIBIM_COLORFORMAT_H
