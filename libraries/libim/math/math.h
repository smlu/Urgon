#ifndef LIBIM_MATH_H
#define LIBIM_MATH_H
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <utility>

namespace libim {

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
}

#endif // LIBIM_MATH_H
