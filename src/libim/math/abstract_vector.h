#ifndef LIBIM_ABSTRACT_VECTOR_H
#define LIBIM_ABSTRACT_VECTOR_H
#include <array>
#include <type_traits>
#include "math.h"


namespace libim {

    template<typename T, std::size_t S, typename Tag>
    struct AbstractVector : public std::array<T, S>
    {
        using base_type = std::array<T, S>;

        static_assert(std::is_arithmetic_v<T>, "T must be arithmetic type!");
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
}
#endif // LIBIM_ABSTRACT_VECTOR_H
