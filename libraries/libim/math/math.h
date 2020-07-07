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
            // Ref: https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
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


    template<typename T, typename = std::enable_if<std::is_floating_point_v<T>>>
    static constexpr T epsilon = std::numeric_limits<T>::epsilon();

    // Compares two floats and returns true if equal
    inline constexpr bool cmpf(float f1, float f2, float e = epsilon<float>) {
        return detail::cmpf_impl(f1, f2, e);
    }

    // Compares two doubles and returns true if equal
    inline constexpr bool cmpf(double f1, double f2, double e = epsilon<double>) {
        return detail::cmpf_impl(f1, f2, e);
    }

    // Compares two long doubles and returns true if equal
    inline constexpr bool cmpf(long double f1, long double f2, long double e = epsilon<long double>) {
        return detail::cmpf_impl(f1, f2, e);
    }
}

#endif // LIBIM_MATH_H
