#ifndef LIBIM_MATH_H
#define LIBIM_MATH_H
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <type_traits>
#include <utility>

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

    /** clamp overload for arithmetic types (numbers) for std::clamp because r-value references can cause UB */
    template<class T, typename = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline T clamp(T v, T lo, T hi) {
        return std::clamp(v, lo, hi);
    }

    template<class T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline const T& clamp(const T& v, const T& lo, const T& hi) {
        return std::clamp(v, lo, hi);
    }

    template<class T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline T& clamp(T& v, T& lo, T& hi) {
        return const_cast<T&>(std::clamp(v, lo, hi));
    }

    template<class T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    T&& clamp(T&&, T&&, T&&) = delete;

    /** min overload for arithmetic types (numbers) for std::min because r-value references can cause UB */
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline T min(T a, T b) {
        return std::min({ a, b });
    }

    template<typename T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline const T& min(const T& a, const T& b ) {
        return std::min(a, b);
    }

    template<typename T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline T& min(T& a, T& b ) {
        return const_cast<T&>(std::min(a, b));
    }

    template<typename T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline T&& min(T&&, T&&) = delete;

    /** max overload for arithmetic types (numbers) for std::max because r-value references can cause UB */
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline T max(T a, T b) {
        return std::max({ a, b });
    }

    template<typename T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline const T& max(const T& a, const T& b ) {
        return std::max(a, b);
    }

    template<typename T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline T& max(T& a, T& b ) {
        return const_cast<T&>(std::max(a, b));
    }

    template<typename T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    T&& max(T&&, T&&) = delete;

    /** minmax overload for arithmetic types (numbers) for std::minmax because r-value references can cause UB */
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline std::pair<T, T> minmax(T a, T b) {
        return std::minmax({ a, b });
    }

    template<typename T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline std::pair<const T&,const T&> minmax(const T& a, const T& b ) {
        return std::minmax(a, b);
    }

    template<typename T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    [[nodiscard]] constexpr inline std::pair<T&,T&> minmax(T& a, T& b) {
        auto p = std::minmax(a, b);
        return { const_cast<T&>(p.first), const_cast<T&>(p.second) };
    }

    template<typename T, typename = std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>>>
    std::pair<T&&, T&&> minmax(T&&, T&&) = delete;


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
