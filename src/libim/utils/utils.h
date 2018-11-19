#ifndef LIBIM_UTILS_H
#define LIBIM_UTILS_H
#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>
#include <type_traits>

namespace libim::utils {
    namespace detail {
        inline bool compareChar(char c1, char c2)
        {
            if(c1 == c2) {
                return true;
            }
            else if (std::toupper(c1) == std::toupper(c2)) {
                return true;
            }
            return false;
        }
    }

    /* Case insensitive comparison of two strings */
    inline bool iequal(const std::string& s1, const std::string& s2)
    {
        return ((s1.size() == s2.size()) &&
            std::equal(s1.begin(), s1.end(), s2.begin(), &detail::compareChar)
        );
    }

    inline bool iequal(std::string_view s1, std::string_view s2)
    {
        return ((s1.size() == s2.size()) &&
            std::equal(s1.begin(), s1.end(), s2.begin(), &detail::compareChar)
        );
    }

    template<typename T>
    inline constexpr auto to_underlying(T t)
    {
        static_assert(std::is_enum_v<T>, "T must be enum type");
        using U = std::underlying_type_t<T>;
        return static_cast<U>(t);
    }

    template<typename T, typename = void>
    struct underlying_type { using type = T; };

    template<typename T>
    struct underlying_type<T, std::enable_if_t<std::is_enum_v<T>>> {
        using type = std::underlying_type_t<T>;
    };
    template<typename T>
    using underlying_type_t = typename underlying_type<T>::type;

    template<typename T>
    inline std::size_t numdigits(T i)
    {
        static_assert (std::is_integral_v<T> && std::is_unsigned_v<T>, "T must be unsigned integral type");
        return i > 0 ? static_cast<std::size_t>(std::log10(i)) + 1 : 1;
    }
}

#endif // LIBIM_UTILS_H
