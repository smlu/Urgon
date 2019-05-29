#ifndef LIBIM_COLOR_H
#define LIBIM_COLOR_H
#include "abstract_vector.h"

namespace libim {
    struct color_vector_tag {};

    struct Color : public AbstractVector<float, 4, color_vector_tag>
    {
        using Base_ = AbstractVector<float, 4, color_vector_tag>;
        using Base_::AbstractVector;

        constexpr inline Color(float r, float g, float b, float a) noexcept :
            Base_{{{ r, g, b, a }}}
        {}

        explicit constexpr inline Color(std::array<float, 4> a) noexcept :
            Base_{{ std::move(a) }}
        {}

        constexpr inline float red() const
        {
            return this->at(0);
        }

        constexpr inline void setRed(float r)
        {
            set(0, r);
        }

        constexpr inline float green() const
        {
            return this->at(1);
        }

        constexpr inline void setGreen(float g)
        {
            set(1, g);
        }

        constexpr inline float blue() const
        {
            return this->at(2);
        }

        constexpr inline void setBlue(float b)
        {
            set(2, b);
        }

        constexpr inline float alpha() const
        {
            return this->at(3);
        }

        constexpr inline void setAlpha(float a)
        {
            set(3, a);
        }
    };
}

#endif // LIBIM_COLOR_H
