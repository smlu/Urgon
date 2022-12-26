#ifndef LIBIM_STRING_MAP_H
#define LIBIM_STRING_MAP_H
#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string_view>
#include <type_traits>

namespace libim{
    struct StringCaseInsensitiveLess
    {
        bool operator()(const std::string_view lhs, const std::string_view rhs) const
        {
            return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                [](const char& lhs, const char& rhs) {
                    return std::tolower(lhs) < std::tolower(rhs);
            });
        }
    };

    struct StringCaseInsensitiveEqual
    {
        bool operator()(const std::string_view lhs, const std::string_view rhs) const
        {
            return lhs.size() == rhs.size() &&
                std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                    [](const char& lhs, const char& rhs) {
                        return std::tolower(lhs) == std::tolower(rhs);
                });
        }
    };

    /**
     * A std::map alias for string which uses a case-insensitive string for key.
     *
     * @tparam StrT   - The key string type. Must be convertible to std::string_view.
     * @tparam ValueT - The value type.
     * @tparam PredT  - The predicate type. By default, CaseInsensitiveLess is used.
     */
    template<typename StrT, typename ValueT, typename PredT = StringCaseInsensitiveLess,
        typename = std::enable_if_t<std::is_convertible_v<StrT, std::string_view>>>
    using StringMap = std::map<StrT, ValueT, PredT>;

    /**
     * A std::set alias for string which uses a case-insensitive string for key.
     *
     * @tparam KeyT   - The key type. Must be convertible to std::string_view.
     * @tparam PredT  - The predicate type. By default, CaseInsensitiveLess is used.
     */
    template<typename KeyT = std::string, typename PredT = StringCaseInsensitiveLess,
        typename = std::enable_if_t<std::is_convertible_v<KeyT, std::string_view>>>
    using StringSet = std::set<KeyT, PredT>;
}
#endif // LIBIM_STRING_MAP_H