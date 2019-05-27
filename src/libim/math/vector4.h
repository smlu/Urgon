#ifndef LIBIM_VECTOR4_H
#define LIBIM_VECTOR4_H
#include "vector.h"

namespace libim {
    template<typename T>
    struct Vector4 : public Vector<T, 4>
    {
        using Vector<T, 4>::Vector;
        constexpr inline Vector4(T x, T y, T z, T w) noexcept :
            Vector<T, 4>({ x, y, z, w })
        {}

        explicit constexpr inline Vector4(std::array<T, 4> a) noexcept :
            Vector<T, 4>{std::move(a)}
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

        void setY(T y)
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

        constexpr inline T w() const
        {
            return this->at(3);
        }

        constexpr inline void setW(T w)
        {
            set(3, w);
        }
    };

    struct Vector4f : public Vector4<float> {
        using Vector4<float>::Vector4;
    };

    struct Vector4i : public Vector4<int32_t> {
        using Vector4<int32_t>::Vector4;
    };

    struct Vector4u : public Vector4<uint32_t> {
        using Vector4<uint32_t>::Vector4;
    };
}
#endif // LIBIM_VECTOR4_H
