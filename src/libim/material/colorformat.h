#ifndef LIBIM_COLORFORMAT_H
#define LIBIM_COLORFORMAT_H
#include <cstdint>
#include <iterator>
#include <initializer_list>

struct ColorFormat
{
    int32_t colorMode = 0; // RGB565 = 1, RGBA4444 = 2
    int32_t bpp       = 0; // Bit depth per pixel

    int32_t redBPP   = 0;
    int32_t greenBPP = 0;
    int32_t blueBPP  = 0;

    int32_t RedShl   = 0;
    int32_t GreenShl = 0;
    int32_t BlueShl  = 0;

    int32_t RedShr   = 0;
    int32_t GreenShr = 0;
    int32_t BlueShr  = 0;

    int32_t alphaBPP = 0;
    int32_t AlphaShl = 0;
    int32_t AlphaShr = 0;

    ColorFormat() = default;
    constexpr ColorFormat(int32_t cm, int32_t bd, int32_t rb, int32_t gb, int32_t bb, int32_t rls, int32_t gls, int32_t bls, int32_t rrs, int32_t grs, int32_t brs, int32_t ab, int32_t als, int32_t ars) :
        colorMode(cm), bpp(bd),
        redBPP(rb),   greenBPP(gb),  blueBPP(bb),
        RedShl(rls),  GreenShl(gls), BlueShl(bls),
        RedShr(rrs),  GreenShr(grs), BlueShr(brs),
        alphaBPP(ab), AlphaShl(als), AlphaShr(ars)
    {}

    ColorFormat(const ColorFormat& rhs) noexcept :
        colorMode(rhs.colorMode), bpp(rhs.bpp),
        redBPP(rhs.redBPP), greenBPP(rhs.greenBPP), blueBPP(rhs.blueBPP),
        RedShl(rhs.RedShl), GreenShl(rhs.GreenShl), BlueShl(rhs.BlueShl),
        RedShr(rhs.RedShr), GreenShr(rhs.GreenShr), BlueShr(rhs.BlueShr),
        alphaBPP(rhs.alphaBPP), AlphaShl(rhs.AlphaShl), AlphaShr(rhs.AlphaShr)
    {}

    ColorFormat(ColorFormat&& rrhs) noexcept :
        colorMode(rrhs.colorMode), bpp(rrhs.bpp),
        redBPP(rrhs.redBPP), greenBPP(rrhs.greenBPP), blueBPP(rrhs.blueBPP),
        RedShl(rrhs.RedShl), GreenShl(rrhs.GreenShl), BlueShl(rrhs.BlueShl),
        RedShr(rrhs.RedShr), GreenShr(rrhs.GreenShr), BlueShr(rrhs.BlueShr),
        alphaBPP(rrhs.alphaBPP), AlphaShl(rrhs.AlphaShl), AlphaShr(rrhs.AlphaShr)
    {
        // Clear rrhs
        rrhs.colorMode = 0;
        rrhs.bpp       = 0;

        rrhs.redBPP   = 0;
        rrhs.greenBPP = 0;
        rrhs.blueBPP  = 0;

        rrhs.RedShl    = 0;
        rrhs.GreenShl  = 0;
        rrhs.BlueShl   = 0;

        rrhs.RedShr    = 0;
        rrhs.GreenShr  = 0;
        rrhs.BlueShr   = 0;

        rrhs.alphaBPP = 0;
        rrhs.AlphaShl = 0;
        rrhs.AlphaShr = 0;
    }

    ColorFormat& operator= (const ColorFormat& rhs) noexcept
    {
        if(&rhs != this)
        {
            colorMode = rhs.colorMode;
            bpp = rhs.bpp;

            redBPP   = rhs.redBPP;
            greenBPP = rhs.greenBPP;
            blueBPP  = rhs.blueBPP;

            RedShl   = rhs.RedShl;
            GreenShl = rhs.GreenShl;
            BlueShl  = rhs.BlueShl;

            RedShr   = rhs.RedShr;
            GreenShr = rhs.GreenShr;
            BlueShr  = rhs.BlueShr;

            alphaBPP = rhs.alphaBPP;
            AlphaShl = rhs.AlphaShl;
            AlphaShr = rhs.AlphaShr;
        }
        return *this;
    }

    ColorFormat& operator= (ColorFormat&& rrhs) noexcept
    {
        if(&rrhs != this)
        {
            colorMode = rrhs.colorMode;
            bpp = rrhs.bpp;

            redBPP   = rrhs.redBPP;
            greenBPP = rrhs.greenBPP;
            blueBPP  = rrhs.blueBPP;

            RedShl   = rrhs.RedShl;
            GreenShl = rrhs.GreenShl;
            BlueShl  = rrhs.BlueShl;

            RedShr   = rrhs.RedShr;
            GreenShr = rrhs.GreenShr;
            BlueShr  = rrhs.BlueShr;

            alphaBPP = rrhs.alphaBPP;
            AlphaShl = rrhs.AlphaShl;
            AlphaShr = rrhs.AlphaShr;

            // Clear rrhs
            rrhs.colorMode = 0;
            rrhs.bpp       = 0;

            rrhs.redBPP   = 0;
            rrhs.greenBPP = 0;
            rrhs.blueBPP  = 0;

            rrhs.RedShl    = 0;
            rrhs.GreenShl  = 0;
            rrhs.BlueShl   = 0;

            rrhs.RedShr    = 0;
            rrhs.GreenShr  = 0;
            rrhs.BlueShr   = 0;

            rrhs.alphaBPP = 0;
            rrhs.AlphaShl = 0;
            rrhs.AlphaShr = 0;
        }
        return *this;
    }
};

static constexpr ColorFormat RGB565   (2, 16, 5, 6, 5, 11, 5, 0, 3, 2, 3, 0,  0, 0);
static constexpr ColorFormat RGBA4444 (2, 16, 4, 4, 4, 12, 8, 4, 4, 4, 4, 4,  0, 4);
static constexpr ColorFormat ARGB4444 (2, 16, 4, 4, 4,  8, 4, 0, 4, 4, 4, 4, 12, 4);
static constexpr ColorFormat ARGB5551 (2, 16, 5, 5, 5, 10, 5, 0, 3, 3, 3, 1, 16, 7);

#endif // LIBIM_COLORFORMAT_H
