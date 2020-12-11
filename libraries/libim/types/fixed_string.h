#ifndef LIBIM_FIXED_STRING_H
#define LIBIM_FIXED_STRING_H
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <stdexcept>

#include <libim/utils/utils.h>

namespace libim {

    /** FixedString represents fixed size string aka char[Size] */
    template<std::size_t N>
    struct FixedString : std::array<char, N>
    {
        using base_type = std::array<char, N>;
        using std::array<char, N>::array;

        constexpr FixedString() : base_type{ 0 } {}
        constexpr FixedString(base_type a) : base_type(std::move(a))
        {
            this->operator[](N-1) = 0;
        }

        constexpr FixedString(std::string_view str) : base_type{ 0 }
        {
            if(str.size() > N) {
                throw std::invalid_argument("Invalid str size to init FixedString");
            }
            std::copy(str.begin(), str.end(), this->begin());
            this->operator[](N-1) = 0;

        }

        template<std::size_t C>
        constexpr FixedString(const char (&str)[C]) : base_type{ 0 }
        {
            static_assert(C <= N, "Invalid string literal size to init FixedString");
            std::copy(str, str + C, this->begin()); // Note: since C++20 constexpr
            this->operator[](N-1) = 0;
        }

        std::string toStdString() const
        {
            return utils::trim(*this);
        }

        operator std::string() const
        {
            return toStdString();
        }

        operator std::string_view() const // Warning: It is dangerous to implicitly return string_view!
        {
            auto endIt = std::find(this->begin(), this->end(), '\0');
            return std::string_view(this->data(), std::distance(this->begin(), endIt));
        }

        bool operator == (const FixedString& rstr) const
        {
            for (std::size_t i = 0; i < N; i++)
            {
                if(this->at(i) != rstr.at(i)) {
                    return false;
                }
                else if(this->at(i) == 0) {
                    return true;
                }
            }
            return true;
        }

        constexpr bool isEmpty() const {
            return N == 0 || this->at(0) == 0;
        }
    };

    template<std::size_t N>
    std::ostream& operator<<(std::ostream& os, const FixedString<N>& cstr)
    {
        os << cstr.data();
        return os;
    }
}
#endif