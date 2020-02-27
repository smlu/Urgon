#ifndef LIBIM_VECTOR2_H
#define LIBIM_VECTOR2_H
#include "vector.h"

namespace libim {
    template<typename T>
    struct Vector2 : public Vector<T, 2>
    {
        using Vector<T, 2>::Vector;
        constexpr inline Vector2(T x, T y) noexcept :
            Vector<T, 2>({ x, y })
        {}

        explicit constexpr inline Vector2(std::array<T,2> a) noexcept :
            Vector<T, 2>{std::move(a)}
        {}

        constexpr inline T x() const
        {
            return this->at(0);
        }

        constexpr inline void setX(T x)
        {
            set(0, x);
        }

        constexpr inline T y() const
        {
            return this->at(1);
        }

        constexpr inline void setY(T y)
        {
            set(0, y);
        }
    };

    struct Vector2f : public Vector2<float> {
        using Vector2<float>::Vector2;
    };

    struct Point : public Vector2<int32_t> {
        using Vector2<int32_t>::Vector2;
    };
}
#endif // LIBIM_VECTOR2_H
