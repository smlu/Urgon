#ifndef LIBIM_COLOR_H
#define LIBIM_COLOR_H
#include "abstract_vector.h"

#include <algorithm>
#include <cstdint>
#include <type_traits>

namespace libim {
    struct color_vector_tag {};

    template<typename T, std::size_t N>
    struct AbstractColor : public AbstractVector<T, N, color_vector_tag>
    {
        //using Base_ = AbstractVector<T, N, color_vector_tag>;
        //using Base_::AbstractVector;
       // using AbstractVector<T, N, color_vector_tag>::AbstractVector;

        static_assert(N  == 3 || N == 4);
        static_assert(std::is_floating_point_v<T> || std::is_unsigned_v<T>);

//        constexpr inline Color(float r, float g, float b, float a) noexcept :
//            Base_{{{ r, g, b, a }}}
//        {}

//        explicit constexpr inline AbstractColor(std::array<T, N> a) noexcept :
//            Base_{{ std::move(a) }}
//        {}

        constexpr inline static T max()
        {
            if constexpr (std::is_floating_point_v<T>) {
                return T(1.0);
            }
            else {
                return T(255);
            }
        }

        constexpr inline static T min()
        {
            if constexpr (std::is_floating_point_v<T>) {
                return T(0.0);
            }
            else {
                return T(0);
            }
        }

        constexpr inline static T clamp(T v) {
            return std::clamp<T>(v, min(), max());
        }

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

        template<bool hasAlpha = (N == 4UL), typename = std::enable_if_t<hasAlpha>>
        constexpr inline T alpha() const
        {
            return this->at(3);
        }

        template<bool hasAlpha = (N == 4UL), typename = std::enable_if_t<hasAlpha>>
        constexpr inline void setAlpha(float a)
        {
            this->set(3, a);
        }
    };


    struct LinearColor : AbstractColor<float, 4> {};
    struct LinearColorRgb : AbstractColor<float, 3> {};


    constexpr inline LinearColor makeLinearColor(float red, float green, float blue, float alpha)
    {
        return LinearColor{{{{ LinearColor::clamp(red), LinearColor::clamp(green), LinearColor::clamp(blue), LinearColor::clamp(alpha) }}}};
    }

    constexpr inline LinearColor makeLinearColor(const LinearColorRgb& rgb, float alpha = 1.0f)
    {
        return LinearColor{{{{ LinearColor::clamp(rgb.red()), LinearColor::clamp(rgb.green()), LinearColor::clamp(rgb.blue()), LinearColor::clamp(alpha) }}}};
    }

    constexpr inline LinearColorRgb makeColorRgb(float red, float green, float blue)
    {
        return LinearColorRgb{{{{ LinearColorRgb::clamp(red), LinearColorRgb::clamp(green), LinearColorRgb::clamp(blue) }}}};
    }

    constexpr inline LinearColorRgb makeColorRgb(const LinearColor& rgba)
    {
        return LinearColorRgb{{{{ LinearColorRgb::clamp(rgba.red()), LinearColorRgb::clamp(rgba.green()), LinearColorRgb::clamp(rgba.blue()) }}}};
    }
}

#endif // LIBIM_COLOR_H
