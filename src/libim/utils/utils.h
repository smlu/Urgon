#ifndef LIBIM_UTILS_H
#define LIBIM_UTILS_H
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>
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

    template<typename T, template<typename> class U>
    typename U<T>::iterator copy(typename U<T>::iterator s_begin, std::size_t count, U<T>& dest)
    {
        dest.reserve(count);
        auto s_end = s_begin + count;
        std::copy(s_begin, s_end, std::back_inserter(dest));
        return s_end;
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


    template<std::size_t base = 10, std::size_t width = 0, typename T>
    static std::string to_string(T n)
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

        if constexpr(width != 0)
        {
            ss << std::setw(width)
               << std::setfill('0')
               << std::fixed
               << std::setprecision(width);
        }

        ss << n;
        return ss.str();
    }

    template<std::size_t N>
    std::string trim(const char (&str)[N])
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
