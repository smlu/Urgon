#ifndef LIBIM_COLOR_H
#define LIBIM_COLOR_H
#include "abstract_vector.h"
#include "math.h"

#include <algorithm>
#include <cstdint>
#include <type_traits>

namespace libim {

    template<typename T, std::size_t N, typename ColorTag>
    struct AbstractColor : public AbstractVector<T, N, ColorTag>
    {
        using AbstractVector<T, N, ColorTag>::AbstractVector;

        static_assert(N  == 3 || N == 4);
        static_assert(std::is_floating_point_v<T> || std::is_unsigned_v<T>);

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
            return libim::clamp<T>(v, min(), max());
        }

        constexpr inline T red() const
        {
            return this->at(0);
        }

        constexpr inline void setRed(T r)
        {
            this->set(0, r);
        }

        constexpr inline T green() const
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

        constexpr static inline bool hasAlpha()
        {
            return (N == 4UL);
        }

        template<bool hasAlpha = hasAlpha(), typename = std::enable_if_t<hasAlpha>>
        constexpr inline T alpha() const
        {
            return this->at(3);
        }

        template<bool hasAlpha = hasAlpha(), typename = std::enable_if_t<hasAlpha>>
        constexpr inline void setAlpha(T a)
        {
            this->set(3, a);
        }
    };


    struct color_vector_tag {};

    /**
     * Represents non-linear (gamma space), 8-bit/component RGBA color.
     * @note: All mathematical operations should be done in linear space.
     *        Use makeLinearColor(color, sRGB) to convert non-linear to linear space.
     *
     *        When quantizing from linear color space it should be converted to gamma space,
     *        as 8 bits of precision is not enough to store linear space colors.
     *        Use helper function makeColor(linearColor, sRGB=true) in order to achieve this.
     */
    struct Color : AbstractColor<uint8_t, 4, color_vector_tag> {};

    /**
     * Represents non-linear (gamma space), 8-bit/component RGB color.
     * @note: All mathematical operations should be done in linear space.
     *        Use makeLinearColor(color, sRGB) to convert non-linear to linear space.
     *
     *        When quantizing from linear color space it should be converted to gamma space,
     *        as 8 bits of precision is not enough to store linear space colors.
     *        Use helper function makeColorRgb(linearColor, sRGB=true) in order to achieve this.
     */
    struct ColorRgb : AbstractColor<uint8_t, 3, color_vector_tag> {};


    struct linear_color_vector_tag : math_vector_tag {};

    /** Base class to represent linear, 32-bit/component floating point color. */
    template<class LColor, std::size_t N>
    struct AbstractLinearColor :
        AbstractColor<float, N, linear_color_vector_tag>
    {
        using AbstractColor<float, N, linear_color_vector_tag>::AbstractColor;
        constexpr inline LColor clamped() const
        {
            LColor color;
            for (std::size_t i = 0; i < this->size(); i++) {
                color.at(i) = LColor::clamp(this->at(i));
            }
            return color;
        }

    private:
        friend LColor;
    };

    /**
     * Represents linear, 32-bit/component floating point RGBA color.
     * @note: When converting color from non-linear gamma space use helper function makeLinearColor(color, sRGB=true).
     */
    struct LinearColor : AbstractLinearColor<LinearColor, 4> {
        using AbstractLinearColor<LinearColor, 4>::AbstractLinearColor;
    };

    /**
     * Represents linear, 32-bit/component floating point RGB color.
     * @note: When converting color from non-linear gamma space use helper function makeLinearColorRgb(color, sRGB=true).
     */
    struct LinearColorRgb : AbstractLinearColor<LinearColorRgb, 3> {
        using AbstractLinearColor<LinearColorRgb, 3>::AbstractLinearColor;
    };


    /**
     * Converts ColorRgb object to Color.
     * @param rgb   - non-linear (gamma space) RGB color object to convert.
     * @param alpha - (optional) value of alpha color component. Default is 255.
     */
    constexpr inline Color makeColor(const ColorRgb& rgb, uint8_t alpha = Color::max())
    {
        return Color{{{ rgb.red(), rgb.green(), rgb.blue(), alpha }}};
    }

    /**
     * Quantizes LinearColor object to Color.
     * @param color - linear color object to convert.
     * @param sRGB  - (optional) do sRGB conversion. Default is true.
     */
    Color makeColor(const LinearColor& color, bool sRGB = true);

    /**
     * Quantizes LinearColorRgb object to Color.
     * @param rgb   - linear RGB color object to convert.
     * @param sRGB  - (optional) do sRGB conversion. Default is true.
     * @param alpha - (optional) value of alpha color component. Default is 255.
     */
    Color makeColor(const LinearColorRgb& rgb, bool sRGB = true, uint8_t alpha = Color::max());

    /**
     * Returns Color object from RGBA color components.
     * @param red   - red color component.
     * @param green - green color component.
     * @param blue  - blue color component.
     * @param alpha - alpha color component.
     */
    constexpr inline Color makeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    {
        return Color{{{{ red, green, blue, alpha }}}};
    }

    /**
     * Converts Color object to ColorRgb.
     * @param rgba - non-linear (gamma space) color object to convert.
     */
    constexpr inline ColorRgb makeColorRgb(const Color& rgba)
    {
        return {{{ rgba.red(), rgba.green(), rgba.blue() }}};
    }

    /**
     * Converts LinearColor object to ColorRgb.
     * @param color - linear color object to convert.
     * @param sRGB  - (optional) do sRGB conversion. Default is true.
     */
    inline ColorRgb makeColorRgb(const LinearColor& color, bool sRGB = true)
    {
        return makeColorRgb(makeColor(color, sRGB));
    }

    /**
     * Converts LinearColorRgb object to ColorRgb.
     * @param rgb   - linear RGB color object to convert.
     * @param sRGB  - (optional) do sRGB conversion. Default is true.
     */
    ColorRgb makeColorRgb(const LinearColorRgb& rgb, bool sRGB = true);

    /**
     * Returns ColorRgb object from RGB color components.
     * @param red   - red color component.
     * @param green - green color component.
     * @param blue  - blue color component.
     */
    constexpr inline ColorRgb makeColorRgb(uint8_t red, uint8_t green, uint8_t blue)
    {
        return ColorRgb{{{{ red, green, blue }}}};
    }

    /**
     * Converts Color object to LinearColor.
     * @param color - non-linear (gamma space) color object to convert.
     * @param sRGB  - (optional) do sRGB conversion. Default is true.
     */
    LinearColor makeLinearColor(const Color& color, bool sRGB = true);

    /**
     * Converts ColorRgb object to LinearColor.
     * @param rgb   - non-linear (gamma space) RGB color object to convert.
     * @param sRGB  - (optional) do sRGB conversion. Default is true.
     * @param alpha - (optional) value of alpha color component. Default is 1.0f.
     */
    LinearColor makeLinearColor(const ColorRgb& rgb, bool sRGB = true, float alpha = LinearColor::max());

    /**
     * Converts LinearColorRgb object to LinearColor.
     * @param rgb   - linear RGB color object to convert.
     * @param alpha - (optional) value of alpha color component. Default is 1.0f.
     */
    constexpr inline LinearColor makeLinearColor(const LinearColorRgb& rgb, float alpha = LinearColor::max())
    {
        return LinearColor{{
            LinearColor::clamp(rgb.red()),
            LinearColor::clamp(rgb.green()),
            LinearColor::clamp(rgb.blue()),
            LinearColor::clamp(alpha)
        }};
    }

    /**
     * Returns LinearColor object from RGBA color components.
     * @note: Color components are clamped before applied.
     *
     * @param red   - red color component.
     * @param green - green color component.
     * @param blue  - blue color component.
     * @param alpha - alpha color component.
     */
    constexpr inline LinearColor makeLinearColor(float red, float green, float blue, float alpha)
    {
        return LinearColor{{
            LinearColor::clamp(red),
            LinearColor::clamp(green),
            LinearColor::clamp(blue),
            LinearColor::clamp(alpha)
        }};
    }

    /**
     * Converts Color object to LinearColorRgb.
     * @param color - non-linear (gamma space) color object to convert.
     * @param sRGB  - (optional) do sRGB conversion. Default is true.
     */
    LinearColorRgb makeLinearColorRgb(const Color& color, bool sRGB = true);

    /**
     * Converts ColorRgb object to LinearColorRgb.
     * @param rgb  - non-linear (gamma space) RGB color object to convert.
     * @param sRGB - (optional) do sRGB conversion. Default is true.
     */
    LinearColorRgb makeLinearColorRgb(const ColorRgb& rgb, bool sRGB = true);

    /**
     * Converts LinearColor object to LinearColorRgb.
     * @param rgba - linear color object to convert.
     */
    constexpr inline LinearColorRgb makeLinearColorRgb(const LinearColor& rgba)
    {
        return LinearColorRgb{{
            LinearColorRgb::clamp(rgba.red()),
            LinearColorRgb::clamp(rgba.green()),
            LinearColorRgb::clamp(rgba.blue())
        }};
    }

    /**
     * Returns LinearColorRgb object from RGB color components.
     * @note: Color components are clamped before applied.
     *
     * @param red   - red color component.
     * @param green - green color component.
     * @param blue  - blue color component.
     */
    constexpr inline LinearColorRgb makeLinearColorRgb(float red, float green, float blue)
    {
        return LinearColorRgb{{
            LinearColorRgb::clamp(red),
            LinearColorRgb::clamp(green),
            LinearColorRgb::clamp(blue)
        }};
    }
}

#endif // LIBIM_COLOR_H