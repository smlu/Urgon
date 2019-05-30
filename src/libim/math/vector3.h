#ifndef LIBIM_VECTOR3_H
#define LIBIM_VECTOR3_H
#include "vector.h"

namespace libim {
    template<typename T>
    struct Vector3 : public Vector<T, 3>
    {
        using Vector<T, 3>::Vector;
        constexpr inline Vector3(T x, T y, T z) noexcept :
            Vector<T, 3>({ x, y, z })
        {}

        explicit constexpr inline Vector3(std::array<T, 3> a) noexcept :
            Vector<T, 3>{ std::move(a) }
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

        constexpr inline T z() const
        {
            return this->at(2);
        }

        constexpr inline void setZ(T z)
        {
            set(2, z);
        }
    };

    struct Vector3f : public Vector3<float> {
        using Vector3<float>::Vector3;
    };

    struct Vector3i : public Vector3<int32_t> {
        using Vector3<int32_t>::Vector3;
    };

    struct Vector3u : public Vector3<uint32_t> {
        using Vector3<uint32_t>::Vector3;
    };
}
#endif // LIBIM_VECTOR3_H
