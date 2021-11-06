#ifndef LIBIM_VECTOR3_H
#define LIBIM_VECTOR3_H
#include "vector.h"

#include <numeric>
#include <vector>
#include <type_traits>

namespace libim {

    // Class represents 3D vector (x,y,z)
    template<typename T>
    struct Vector3 : public Vector<T, 3>
    {
        using Vector<T, 3>::Vector;
        constexpr inline Vector3(T x, T y, T z) noexcept :
            Vector<T, 3>{{ x, y, z }}
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
            set(1, y);
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

    // Calculate vertex normal over face normals of adjacent faces that contains the vertex.
    // Vertex normal is calculated by averiging face normals (unweighted averaging).
    template<typename V3T, typename = std::enable_if_t<std::is_base_of_v<Vector3<typename V3T::value_type>, V3T>>>
    static V3T unweightedVertexNormal(const std::vector<V3T>& faceNormals)
    {
        V3T vn;
        if (!faceNormals.empty()) {
            vn = std::accumulate(faceNormals.begin(), faceNormals.end(), vn) / faceNormals.size();
        }
        return vn;
    }
}
#endif // LIBIM_VECTOR3_H
