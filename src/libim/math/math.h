#ifndef LIBIM_MATH_H
#define LIBIM_MATH_H
#include <cmath>
#include <cfloat>
#include <type_traits>

namespace libim {

    namespace detail {
        template<typename F>
        inline constexpr bool cmpf_impl(F f1, F f2, F maxRelDiff)
        {
            static_assert (std::is_floating_point_v<F>, "F must be a floating point");

            // Check if the numbers are really close -- needed
            // when comparing numbers near zero.
            const F diff = std::fabs(f1 - f2);
            if (diff <= maxRelDiff) {
                return true;
            }

            f1 = std::fabs(f1);
            f2 = std::fabs(f2);
            const F largest = (f2 > f1) ? f2 : f1;

            if (diff <= largest * maxRelDiff) {
                return true;
            }
            return false;
        }
    }

    inline constexpr bool cmpf(float f1, float f2, float e = FLT_EPSILON) {
        return detail::cmpf_impl(f1, f2, e);
    }

    inline constexpr bool cmpf(double f1, double f2, double e = DBL_EPSILON) {
        return detail::cmpf_impl(f1, f2, e);
    }

    inline constexpr bool cmpf(long double f1, long double f2, long double e = LDBL_EPSILON) {
        return detail::cmpf_impl(f1, f2, e);
    }

}

#endif // LIBIM_MATH_H
