#ifndef LIBIM_STRING_MAP_H
#define LIBIM_STRING_MAP_H
#include <algorithm>
#include <cctype>
#include <map>
#include <string_view>
#include <type_traits>

namespace libim{
    struct CaseInsensitiveComparator
    {
        bool operator()(const std::string_view lhs, const std::string_view rhs) const
        {
            return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                [](const char& lhs, const char& rhs) {
                    return std::tolower(lhs) < std::tolower(rhs);
            });
        }
    };

    /**
     * A std::map alias that uses a case-insensitive string comparator for key.
     *
     * @tparam StrT   - The key string type. Must be convertible to std::string_view.
     * @tparam ValueT - The value type.
     */
    template<typename StrT, typename ValueT,
        typename = std::enable_if_t<std::is_convertible_v<StrT, std::string_view>>>
    using StringMap = std::map<StrT, ValueT, CaseInsensitiveComparator>;
}
#endif // LIBIM_STRING_MAP_H