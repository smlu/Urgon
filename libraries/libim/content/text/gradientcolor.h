#ifndef LIBIM_GRADIENTCOLOR_H
#define LIBIM_GRADIENTCOLOR_H
#include "../../math/color.h"
#include <string>

namespace libim::content::text {

    struct GradientColor final
    {
        LinearColor top;
        LinearColor middle;
        LinearColor bottomLeft;
        LinearColor bottomRight;

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

        std::string toString() const
        {
            auto topstr = top.toString();
            *(--topstr.end()) = '/';

            auto middlestr = middle.toString().substr(1);
            *(--middlestr.end()) = '/';

            auto bleftstr = bottomLeft.toString().substr(1);
            *(--bleftstr.end()) = '/';

            auto brightstr = bottomRight.toString().substr(1);
            return topstr + middlestr + bleftstr + brightstr;
        }
    };

}
#endif // LIBIM_GRADIENTCOLOR_H
