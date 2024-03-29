#ifndef LIBIM_UTILS_H
#define LIBIM_UTILS_H
#include <algorithm>
#include <assert.h>
#include <cctype>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <libim/types/safe_cast.h>

#define CONCATENATE_DIRECT(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_DIRECT(s1, s2)

// Anonymous variable
#ifdef _MSC_VER
# define ANOVAR(str) CONCATENATE(str, __COUNTER__)
#else
# define ANOVAR(str) CONCATENATE(str, __LINE__)
#endif

namespace libim::utils {
    namespace detail {
        inline bool compare_char(char c1, char c2)
        {
            if (c1 == c2) {
                return true;
            }
            else if (std::toupper(c1) == std::toupper(c2)) {
                return true;
            }
            return false;
        }

        [[nodiscard]] inline std::string trim(const char* str, std::size_t len)
        {
            std::size_t end = 0;
            while(end++ < len) {
                if (str[end] == '\0') {
                    break;
                }
            }
            return std::string(str, end);
        }

        // Note, d_size must be for 1 greater
        // than src.size() to include null termination
        inline bool strcpy(char* dest, std::size_t d_size, const std::string_view src)
        {
            std::size_t count = src.size() + 1;
            count = std::min(d_size, count);
        #if defined(__STDC_LIB_EXT1__) || defined(_MSC_VER)
            return strncpy_s(dest, d_size, src.data(), count) == 0;
        #else
            const bool success = std::strncpy(dest, src.data(), count - 1) != nullptr;
            dest[count - 1] = 0;
            return success;
        #endif
        }

        template <typename T>
        auto promote_to_printable_integer_type(T i) -> decltype(+i)
        {
            // Make char type printable as number and not as character.
            static_assert (std::is_arithmetic_v<T>);
            return +i;
        }
    } // detail

    /** std::string_view implementation of std::streambuf. */
    struct StringViewStreamBuf : public std::streambuf
    {
        StringViewStreamBuf(const std::string_view strv)
        {
            char* pd(const_cast<char*>(strv.data()));
            setg(pd, pd, pd + strv.size());
        }
    };

    // Helper type for the visitor
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

    template <typename Lambda>
    [[nodiscard]] auto at_scope_exit(Lambda&& callback)
    {
        struct at_exit_scope_t final
        {
            at_exit_scope_t(Lambda&& cb) :
                cb_(std::forward<Lambda>(cb)) {}
            ~at_exit_scope_t() {
               cb_();
            }
        private:
            Lambda cb_;
        };

        return at_exit_scope_t { std::forward<Lambda>(callback) };
    }

    #ifdef AT_SCOPE_EXIT
    #  error macro AT_SCOPE_EXIT already defined
    #endif

    #define AT_SCOPE_EXIT(...) \
        const auto ANOVAR(sexit_) = libim::utils::at_scope_exit(__VA_ARGS__)

    /**
     * Returns wrapped iterable object which iterator is pair of iterator's
     * index number and iterator of iterable object. Wrapping object has 4 methods:
     * begin, end and their const variation.
     * Example:
     *     std::vector<char> chars{ 'a', 'b', 'c' };
     *     for(auto[idx, c] : chars) {
     *         std::cout << "idx: " << idx << " char: " << c << std::endl;
     *     }
     *
     *     Should print: idx: 0 char: a
     *                   idx: 1 char: b
     *                   idx: 2 char: c
     *
     *
     * @param iterable RValue iterable object.
     * @return Wrapped iterable object.
    */
    template <typename T,
              typename TIter = decltype(std::begin(std::declval<T>())),
              typename = decltype(std::end(std::declval<T>()))>
    constexpr auto enumerate(T&& iterable)
    {
        struct iterator
        {
            size_t i;
            TIter iter;
            bool operator != (const iterator & other) const { return iter != other.iter; }
            void operator ++ () { ++i; ++iter; }
            auto operator * () const { return std::tie(std::as_const(i), std::as_const(*iter)); }
            auto operator * () { return std::tie(std::as_const(i), *iter); }
        };

        struct iterable_wrapper
        {
            T iterable;
            auto begin() { return iterator{ 0, std::begin(iterable) }; }
            auto end() { return iterator{ 0, std::end(iterable) }; }
            auto begin() const { return iterator{ 0, std::begin(iterable) }; }
            auto end() const { return iterator{ 0, std::end(iterable) }; }
        };

        return iterable_wrapper{ std::forward<T>(iterable) };
    }

    /**
     * Const version of enumerate function.
     * @see enumerate
    */
    template<typename T>
    constexpr auto cenumerate(T&& iterable)
    {
        return enumerate<std::add_const_t<T>,
                decltype(std::cbegin(std::declval<T>()))>(
                    std::forward<std::add_const_t<T>>(iterable)
        );
    }

    /**
     * Copies n elements from source iterator to the back of destination container.
     *
     * @param srcIt Source iterator.
     * @param count Number of elements to copy.
     * @param dest Destination container.
     * @return An instance of SourceIt at srcIt + count.
    */
    template<typename SourceIt, typename T>
    SourceIt copy(SourceIt srcIt, const std::size_t count, T& dest)
    {
        using dt = typename std::iterator_traits<SourceIt>::difference_type;
        dest.reserve(count);
        auto srcEnd = std::next(srcIt, safe_cast<dt>(count));
        std::copy(srcIt, srcEnd, std::back_inserter(dest));
        return srcEnd;
    }

    /** Case insensitive comparison of two strings */
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


    /** Returns instance of enumerator casted to it's underlying type. */
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


    /* Returns number of digits in the unsigned number. */
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
            "floating point can only be represented in base 10"
        );

        std::stringstream ss;
        ss.exceptions(std::ios::failbit);

        if constexpr (base == 8) {
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

        if constexpr (precision != 0)
        {
            ss << std::setw(precision)
               << std::setfill('0')
               << std::fixed
               << std::setprecision(precision);
        }

        ss << detail::promote_to_printable_integer_type(n);
        return ss.str();
    }

    template<std::size_t N>
    inline bool strcpy(char (&dest)[N], const std::string_view src)
    {
        return detail::strcpy(dest, std::min(N, src.size() + 1), src); // +1 for null terminator
    }

    template<std::size_t N>
    inline bool strcpy(std::array<char, N>& dest, const std::string_view src)
    {
        return detail::strcpy(dest.data(), std::min(N, src.size() + 1), src); // +1 for null terminator
    }

    template<std::size_t N>
    [[nodiscard]] inline std::string trim(const char (&str)[N])
    {
        return detail::trim(str, N);
    }

    template<std::size_t N>
    [[nodiscard]] inline std::string trim(const std::array<char, N>& str)
    {
        return detail::trim(str.data(), N);
    }

    /**
     * Transforms string to all lower case characters.
     *
     * @param str A reference to a string to convert to lower case.
     */
    inline void to_lower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](char c) {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        });
    }

    /* Checks if string s ends with x */
    [[nodiscard]] inline bool ends_with(const std::string& s, std::string_view x)
    {
        return s.size() >= x.size() && s.compare(s.size() - x.size(), s.npos, x) == 0;
    }

    /** Case insensitive check if string s ends with x */
    [[nodiscard]] inline bool iends_with(const std::string& s, std::string_view x)
    {
        auto it = x.begin();
        using it_diff_t = typename std::iterator_traits<decltype(s.begin())>::difference_type;
        return s.size() >= x.size() &&
            std::all_of(std::next(s.begin(), safe_cast<it_diff_t>(s.size() - x.size())), s.end(),
                [&it](const char & c){
                    return ::tolower(c) == ::tolower(*(it++));
            });
    }

    /**
     * Splits input string view by delim string and returns
     *
     * @param str    - input string to split.
     * @param delims - optional string of deliminator chars by which to split input string.
     *                 By default input string is splitted by whitespace character.
     * @param optMax - optional the maximum number of splits.
     * @return std::vector of std::string_view elements
     * */
    [[maybe_unused]][[nodiscard]] static std::vector<std::string_view>
    split(std::string_view str,
        std::string_view delims = "\t\n\v\f\r ",
        std::optional<std::size_t> optMax = std::nullopt)
    {
        assert(!optMax || optMax.value() > 0);
        std::vector<std::string_view> result;
        auto first = str.begin();
        while (first != str.end() && optMax.value_or(1))
        {
            const auto second =
                std::find_first_of(first, str.cend(), delims.cbegin(), delims.cend());
            if (first != second)
            {
                result.emplace_back(
                    str.substr(
                        safe_cast<std::size_t>(std::distance(str.cbegin(), first)),
                        safe_cast<std::size_t>(std::distance(first, second))
                ));
                first = second; // Needed for if statement before return.
            }

            if (second == str.end()) break;
            if (optMax) optMax = optMax.value() - 1;
            first = std::next(second);
        }

        // Add remaining string to the list
        // if optMax and not whole string was parsed
        if (first != str.end()) {
            result.emplace_back(
                str.substr(
                    safe_cast<std::size_t>(std::distance(str.cbegin(), first)),
                    safe_cast<std::size_t>(std::distance(first, str.cend()))
            ));
        }

        return result;
    }

    /**
     * Writes string to string stream.
     * @tparam StringStream - type of string stream to write to.
     * @param ss - string stream to write to.
     * @param format - format string.
     */
    template<typename StringStream>
    inline void ssprintf(StringStream& ss, const char* format)
    {
        ss << format;
    }

    /**
     * Formats string and writes it to string stream.
     *
     * @tparam StringStream - type of string stream to write to.
     * @tparam First        - type of first argument.
     * @tparam Rest         - types of other arguments.
     *
     * @param ss    - string stream to write to.
     * @param format - format string.
     * @param first  - first argument.
     */
    template<typename StringStream, typename First, typename... Rest>
    static void ssprintf(StringStream& ss, const char* format, First first, Rest&&... rest) // recursive variadic function
    {
        for ( ; *format != '\0'; format++ )
        {
            if ( *format == '%' )
            {
                ss << first;
                ssprintf(ss, format + 1, rest...);
                return;
            }
            ss << *format;
        }
    }

    /**
     * Formats string and writes it to string stream.
     *
     * @tparam StringStream - type of string stream to write to.
     * @tparam Args         - types of arguments.
     *
     * @param ss     - string stream to write to.
     * @param format - format string.
     * @param args   - arguments.
     */
    template<typename StringStream, typename... Args>
    inline void ssprintf(StringStream& ss, std::string_view format, Args&&... args) // recursive variadic function
    {
        ssprintf(ss, format.data(), std::forward<Args>(args)...);
    }

    /**
     * Formats string and returns std::basic_string.
     *
     * @tparam CharT - type of string character. By default it is char.
     * @tparam Args  - types of arguments.
     *
     * @param format - format string.
     * @param args   - arguments.
     * @return std::basic_string<CharT> - formatted string.
     */
    template<typename CharT = char, typename... Args>
    inline std::basic_string<CharT> format(std::string_view format, Args&&... args)
    {
        std::ostringstream oss;
        ssprintf(oss, format, std::forward<Args>(args)...);
        return oss.str();
    }

    /**
     * Converts a string to a number.
     *
     * @tparam T The type of the number to convert to.
     *
     * @param strnum - The string to convert to number.
     * @param num    - The number to store the result in.
     * @param base   - The base to use for the conversion i.e.: std::dec, std::hex, std::oct.
     * @result True if the conversion was successful, false otherwise.
     */
    template<typename T, typename BaseF = std::ios_base&(*)(std::ios_base&)>
    bool to_number(const std::string_view strnum, T& num, const BaseF& base = std::dec)
    {
        static_assert(std::is_arithmetic_v<T>, "T is not a arithmetic type");
        StringViewStreamBuf osrb(strnum);
        std::istream is(&osrb);
        is >> base >> num;
        return !is.fail();
    }

    /**
     * Converts a string to a number.
     *
     * @tparam T The type of the number to convert to.
     *
     * @param strnum - The string to convert to number.
     * @param base   - The base to use for the conversion i.e.: std::dec, std::hex, std::oct.
     * @return The number if the conversion was successful.
     * @throw std::ios_base::failure if the conversion was not successful.
     */
    template<typename T, typename DT = std::decay_t<T>, typename BaseF = std::ios_base&(*)(std::ios_base&)>
    DT to_number(const std::string_view strnum, BaseF base = std::dec)
    {
        DT num;
        if(!to_number(strnum, num, base))
        {
            throw std::ios_base::failure("invalid numeric conversion from string");
        }
        return num;
    }

    /**
     * Checks if the predicate is true, if not throws an exception.
     * @tparam ExceptionT - type of exception to throw. By default it is std::runtime_error.
     * @tparam Args       - types of arguments.
     * @param pred        - predicate to check.
     * @param message     - message to throw.
     * @param args        - arguments to format message.
     */
    template<typename ExceptionT = std::runtime_error, typename... Args>
    inline void check(bool pred, const char* message,  Args&&... args)
    {
        if(!pred)
        {
            using namespace libim::utils;
            auto msg = utils::format(message, std::forward<Args>(args)...);
            throw ExceptionT(msg);
        }
    }
}

#endif // LIBIM_UTILS_H
