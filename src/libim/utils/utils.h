#ifndef LIBIM_UTILS_H
#define LIBIM_UTILS_H
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <string>
#include <sstream>
#include <string_view>
#include <type_traits>

namespace libim::utils {
    namespace detail {
        inline bool compare_char(char c1, char c2)
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

    template<typename T>
    typename T::iterator copy(typename T::iterator s_begin, std::size_t count, T& dest)
    {
        dest.reserve(count);
        auto s_end = s_begin + count;
        std::copy(s_begin, s_end, std::back_inserter(dest));
        return s_end;
    }

    /* Case insensitive comparison of two strings */
    [[nodiscard]] inline bool iequal(const std::string& s1, const std::string& s2)
    {
        return ((s1.size() == s2.size()) &&
            std::equal(s1.begin(), s1.end(), s2.begin(), &detail::compare_char)
        );
    }

    [[nodiscard]] inline bool iequal(std::string_view s1, std::string_view s2)
    {
        return ((s1.size() == s2.size()) &&
            std::equal(s1.begin(), s1.end(), s2.begin(), &detail::compare_char)
        );
    }

    template<typename T>
    [[nodiscard]] inline constexpr auto to_underlying(T t)
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
    [[nodiscard]] inline std::size_t numdigits(T i)
    {
        static_assert (std::is_integral_v<T> && std::is_unsigned_v<T>, "T must be unsigned integral type");
        return i > 0 ? static_cast<std::size_t>(std::log10(i)) + 1 : 1;
    }


    template<std::size_t base = 10, std::size_t precision = 0, typename T>
    [[nodiscard]] static std::string to_string(T n)
    {
        static_assert(base == 8 || base == 10 || base == 16, "invalid encoding base");
        static_assert(std::is_arithmetic_v<T>, "T is not a arithmetic type");
        static_assert(!std::is_floating_point_v<T> || base == 10,
            "floating point can be only represented in base 10"
        );

        std::stringstream ss;
        ss.exceptions(std::ios::failbit);


        if constexpr(base == 8) {
            ss << std::oct << std::showbase;
        }
        else if constexpr (base == 10) {
            ss << std::dec;
        }
        else
        {
            ss << "0x"
               << std::uppercase
               << std::hex;
        }

        if constexpr(precision != 0)
        {
            ss << std::setw(precision)
               << std::setfill('0')
               << std::fixed
               << std::setprecision(precision);
        }

        ss << n;
        return ss.str();
    }

    template<std::size_t N>
    bool strcpy(char (&dest)[N], std::string_view src)
    {
        if(N < src.size()) {
            return false;
        }

#if defined(__STDC_LIB_EXT1__) || defined(_MSC_VER)
        return strncpy_s(dest, N, src.data(), src.size()) == 0;
#else
        return std::strncpy(dest, src.data(), src.size()) != nullptr;
#endif
    }

    template<std::size_t N>
    [[nodiscard]] std::string trim(const char (&str)[N])
    {
        std::size_t end = 0;
        while(end++ < N) {
            if(str[end] == '\0') {
                break;
            }
        }

        return std::string(str, end);
    }
}

#endif // LIBIM_UTILS_H
