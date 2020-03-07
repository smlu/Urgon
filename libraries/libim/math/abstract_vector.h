#ifndef LIBIM_ABSTRACT_VECTOR_H
#define LIBIM_ABSTRACT_VECTOR_H
#include <array>
#include <sstream>
#include <string>
#include <type_traits>

#include "math.h"
#include <libim/utils/utils.h>
#include <libim/types/safe_cast.h>


namespace libim {

    template<typename T, std::size_t S, typename Tag>
    struct AbstractVector : public std::array<T, S>
    {
        using base_type = std::array<T, S>;
        using tag_type  = Tag;

        static_assert(std::is_arithmetic_v<T>, "T must be arithmetic type!");

        using base_type::array;
        constexpr AbstractVector() : base_type{ 0 } {}
        constexpr AbstractVector(base_type a) : base_type(std::move(a)) {}

        constexpr void set(std::size_t idx, T v)
        {
            this->at(idx) = v;
        }

        constexpr static inline
        typename base_type::size_type
        size()
        {
            return S;
        }

        constexpr inline bool isZero() const
        {
            for (const auto& e : *this)
            {
                if (e != T(0)) {
                    return false;
                }
            }

            return true;
        }

        std::string toString() const
        {
            std::string ss;
            ss.reserve(
                S * 10  + /* S * max float num char len */
                (S - 1) + /* S - 1 fwd. slashes */
                2         /* 2 parentheses */
            );

            ss.push_back('(');
            for(auto e : *this)
            {
                ss.append(utils::to_string<10, 6>(e));
                ss.push_back('/');
            }
            ss.back() = ')';
            return ss;
        }

        static AbstractVector fromString(std::string_view str)
        {
            std::istringstream istream;
            istream.exceptions(std::ios::failbit);
            istream.rdbuf()->pubsetbuf(
                const_cast<char*>(str.data()),
                safe_cast<std::streamsize>(str.size())
            );

            char ch;
            istream >> ch;
            if( ch != '(' ) {
                throw std::ios_base::failure("Found invalid char while converting string to AbstractVector, expected char '('");
            }

            AbstractVector res;
            for(auto[ idx, e ] : utils::enumerate(res))
            {
                istream >> e;
                istream >> ch;

                if( idx >= (res.size() - 1) )
                {
                    if( ch != ')' ) {
                        throw std::ios_base::failure("Found invalid char while converting string to AbstractVector, expected char ')'");
                    }
                }
                else if( ch != '/' ) {
                    throw std::ios_base::failure("Found invalid char while converting string to AbstractVector, expected char '/'");
                }
            }

            return res;
        }

    };

    template<typename T, std::size_t S, typename Tag>
    inline constexpr bool operator == (const AbstractVector<T, S, Tag>& v1, const AbstractVector<T, S, Tag>& v2)
    {
        if(v1.size() != v2.size()) {
            return false;
        }

        auto cmp = [](T e1, T e2) -> bool {
            if constexpr (std::is_floating_point_v<T>) {
                return cmpf(e1, e2);
            } else {
                return e1 == e2;
            }
        };

        for(std::size_t i = 0; i < v1.size(); i++)
        {
            if(!cmp(v1[i], v2[i])) {
                return false;
            }
        }

        return true;
    }

    template<typename T, std::size_t S, typename Tag>
    inline constexpr bool operator != (const AbstractVector<T, S, Tag>& v1, const AbstractVector<T, S, Tag>& v2)
    {
        return !(v1 == v2);
    }


    // Vector trait
    namespace detail {
        template<typename, typename = void>
        struct isVector : std::false_type {};

        template<typename T>
        struct isVector<T,
                std::void_t<
                    typename T::value_type,
                    typename T::size_type,
                    typename T::tag_type,
                    decltype(T::size())>>
            : std::is_base_of<
                AbstractVector<typename T::value_type, T::size(), typename T::tag_type>,
                T
        > {};
    }

    template <typename T>
    constexpr bool isVector = detail::isVector<T>::value;
}
#endif // LIBIM_ABSTRACT_VECTOR_H
