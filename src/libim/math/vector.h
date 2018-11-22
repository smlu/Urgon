#ifndef LIBIM_VECTOR_H
#define LIBIM_VECTOR_H
#include "abstract_vector.h"
#include <utility>

namespace libim {
    struct position_vector_tag {};

    template<typename T, std::size_t N>
    struct Vector : public AbstractVector<T, N, position_vector_tag>
    {};
}
#endif // LIBIM_VECTOR_H
