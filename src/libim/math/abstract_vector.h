#ifndef LIBIM_ABSTRACT_VECTOR_H
#define LIBIM_ABSTRACT_VECTOR_H
#include <array>
#include <type_traits>

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
}
#endif // LIBIM_ABSTRACT_VECTOR_H
