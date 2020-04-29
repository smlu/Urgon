#ifndef LIBIM_COLOR_H
#define LIBIM_COLOR_H
#include "abstract_vector.h"

namespace libim {
    struct color_vector_tag {};

    template<typename T, std::size_t N>
    struct AbstractColor : public AbstractVector<T, N, color_vector_tag>
    {
        //using Base_ = AbstractVector<T, N, color_vector_tag>;
        //using Base_::AbstractVector;
       // using AbstractVector<T, N, color_vector_tag>::AbstractVector;

        static_assert (N  == 3 || N == 4);

//        constexpr inline Color(float r, float g, float b, float a) noexcept :
//            Base_{{{ r, g, b, a }}}
//        {}

//        explicit constexpr inline AbstractColor(std::array<T, N> a) noexcept :
//            Base_{{ std::move(a) }}
//        {}

        constexpr inline T red() const
        {
            return this->at(0);
        }

        constexpr inline void setRed(T r)
        {
            this->set(0, r);
        }

        constexpr inline float green() const
        {
            return this->at(1);
        }

        constexpr inline void setGreen(T g)
        {
            this->set(1, g);
        }

        constexpr inline T blue() const
        {
            return this->at(2);
        }

        constexpr inline void setBlue(T b)
        {
            this->set(2, b);
        }

        constexpr inline T alpha() const
        {
            return this->at(3);
        }

        template<bool hasAplha = (N == 4UL), typename = std::enable_if_t<hasAplha>>
        constexpr inline void setAlpha(float a)
        {
            this->set(3, a);
        }
    };




    struct Color : AbstractColor<float, 4> {};
    struct ColorRgb : AbstractColor<float, 3> {};

    constexpr inline Color makeColor(float red, float green, float blue, float alpha)
    {
        return Color{{{{ red, green, blue, alpha }}}};
    }

    constexpr inline Color makeColor(const ColorRgb& rgb, float alpha)
    {
        return Color{{{{ rgb.red(), rgb.green(), rgb.blue(), alpha }}}};
    }

    constexpr inline ColorRgb makeColorRgb(float red, float green, float blue)
    {
        return ColorRgb{{{{ red, green, blue }}}};
    }

    constexpr inline ColorRgb makeColorRgb(const Color& rgba)
    {
        return ColorRgb{{{{ rgba.red(), rgba.green(), rgba.blue() }}}};
    }
}

#endif // LIBIM_COLOR_H
