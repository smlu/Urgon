#ifndef LIBIM_GRADIENTCOLOR_H
#define LIBIM_GRADIENTCOLOR_H
#include <libim/math/color.h>
#include <string>

namespace libim::content::text {

    struct GradientColor final
    {
        LinearColor top;
        LinearColor middle;
        LinearColor bottomLeft;
        LinearColor bottomRight;

        GradientColor() = default;
        GradientColor(const std::string_view strcolor); // !< Constructs a gradient color from a string representation in format: "(top/middle/bottomLeft/bottomRight)"

        constexpr inline bool isValid() const noexcept {
            return top.red() != -1.0f && top.green() != -1.f && top.blue() != -1.f && top.alpha() != -1.0f;
        }

        constexpr explicit operator bool() const noexcept {
            return isValid();
        }

        constexpr inline bool isZero() const noexcept {
            return top.isZero() && middle.isZero() && bottomLeft.isZero() && bottomRight.isZero();
        }

        constexpr inline bool operator == (const GradientColor& rhs) const noexcept
        {
            return top == rhs.top && middle == rhs.middle &&
                   bottomLeft == rhs.bottomLeft && bottomRight == rhs.bottomRight;
        };

        constexpr inline bool operator != (const GradientColor& rhs) const noexcept {
            return !(*this == rhs);
        }

        /**
         * Returns a string representation of the gradient color in format: "(top/middle/bottomLeft/bottomRight)"
         * @return A string representation of this object.
         */
        std::string toString() const;
    };
}
#endif // LIBIM_GRADIENTCOLOR_H
