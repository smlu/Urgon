#ifndef LIBIM_MATH_SIZE_H
#define LIBIM_MATH_SIZE_H
#include "abstract_vector.h"

namespace libim {
    struct size_vector_tag { };

    template<typename T, size_t N>
    struct Size final : AbstractVector<T, N, size_vector_tag>
    {
       T volume(Size const& sz)
       {
           T vol = T(1);
           for(auto const &dim : sz) {
               vol *= dim;
           }

           return vol;
        }
    };

    template <typename T, typename ...A>
    constexpr Size<T, sizeof...(A) + 1> makeSize(T v0, A ...v) {
       return Size<T, sizeof...(A) + 1>{v0, v...};
    }
}
#endif // LIBIM_MATH_SIZE_H
