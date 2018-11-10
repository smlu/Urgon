#ifndef LIBIM_UTILS_H
#define LIBIM_UTILS_H
#include <algorithm>
#include <string>
#include <string_view>

namespace libim::utils {
    namespace detail {
        bool compareChar(char c1, char c2)
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

    bool iequal(const std::string& s1, const std::string& s2)
    {
        return ((s1.size() == s2.size()) &&
            std::equal(s1.begin(), s1.end(), s2.begin(), &detail::compareChar)
        );
    }

    bool iequal(std::string_view s1, std::string_view s2)
    {
        return ((s1.size() == s2.size()) &&
            std::equal(s1.begin(), s1.end(), s2.begin(), &detail::compareChar)
        );
    }
}

#endif // LIBIM_UTILS_H
