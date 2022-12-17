#ifndef LIBIM_FIXED_STRING_H
#define LIBIM_FIXED_STRING_H
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <stdexcept>

#include <libim/math/math.h>
#include <libim/utils/utils.h>

namespace libim {

    /**
     * FixedString represents fixed size string aka char[Size].
     * It is a wrapper around std::array<char, Size>.
     *
     * @tparam N Size of the string
     * @tparam nullTermination If true, the last element of the string will be set to 0. Default: true.
     */
    template<std::size_t N, bool nullTermination = true>
    struct FixedString : std::array<char, N>
    {
        static_assert(nullTermination ? N >= 2 : N >= 1, "FixedString size must be at least 1, or 2 if nullTermination is true");

        using base_type = std::array<char, N>;
        using std::array<char, N>::array;

        constexpr bool empty() const noexcept = delete;

        constexpr FixedString() : base_type{ 0 } {}
        constexpr FixedString(base_type a) : base_type(std::move(a))
        {
            if constexpr(nullTermination) {
                this->operator[](libim::min(N - 1, this->size())) = 0;
            }
        }

        constexpr FixedString(std::string_view str) : base_type{ 0 }
        {
            if(str.size() > N) {
                throw std::invalid_argument("Invalid str size to init FixedString");
            }
            std::copy(str.begin(), str.end(), this->begin());
            if constexpr(nullTermination)  {
                this->operator[](libim::min(N - 1, this->size())) = 0;
            }

        }

        template<std::size_t C>
        constexpr FixedString(const char (&str)[C]) : base_type{ 0 }
        {
            static_assert(C <= N, "Invalid string literal size to init FixedString");
            std::copy(str, str + C, this->begin()); // Note: constexpr since C++20
            if constexpr(nullTermination) {
                this->operator[](libim::min(N - 1, this->size())) = 0;
            }
        }

        std::string toStdString() const
        {
            return utils::trim(*this);
        }

        operator std::string() const
        {
            return toStdString();
        }

        constexpr operator std::string_view() const // Warning: It is dangerous to implicitly return string_view!
        {
            return std::string_view(this->data(), this->size());
        }

        constexpr bool operator == (const FixedString& rstr) const
        {
            for (std::size_t i = 0; i < N; i++)
            {
                if (this->at(i) != rstr.at(i)) {
                    return false;
                }
                else if (this->at(i) == 0) {
                    return true;
                }
            }
            return true;
        }

        constexpr typename base_type::size_type size() const noexcept
        {
            /* Note the size can't be cached because it changes the interface struct which is then not plain char[] anymore */
            std::size_t size = 0;
            const char* pNullCh = std::char_traits<char>::find(this->data(), N, '\0');
            if (pNullCh) {
                size = pNullCh - this->data();
            };
            return size;
        }

        constexpr bool isEmpty() const
        {
            return N == 0 || this->at(0) == 0; // N == no use due to static check
        }
    };

    template<std::size_t N, bool nullTermination>
    std::ostream& operator<<(std::ostream& os, const FixedString<N, nullTermination>& cstr)
    {
        os << std::string_view(cstr.data(), cstr.size());
        return os;
    }
}
#endif