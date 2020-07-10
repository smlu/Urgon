#ifndef LIBIM_BOX_H
#define LIBIM_BOX_H
#include <algorithm>
#include <cstdint>
#include <vector>
#include <tuple>

#include "math.h"
#include "size.h"
#include "vector.h"

namespace libim {
    template <typename T, std::size_t N> class Box
    {
        template <std::size_t m, std::size_t S, typename G>
        friend std::tuple<T, T> getRange(const Box<G, S>&);

        template <std::size_t m,  size_t S, typename G>
        friend T getSize(const Box<G, S>&);

    public:
        using value_type = T;

        Vector<T, N> v0, v1;

        constexpr Box() {}
        constexpr Box(const Vector<T, N>& v0, const Vector<T, N>& v1) :
            v0(v0), v1(v1)
        {}

        Box(const Vector<T, N>& v0, const Size<T, N>& sz) : v0(v0) {
            std::transform(sz.begin(), sz.end(), v0.begin(), v1.begin(), std::plus<T>());
        }

        constexpr bool isZero() const
        {
            return v0.isZero() && v1.isZero();
        }

        bool overlaps(const Box& b) const
        {
            for(auto i0 = v0.begin(), i1 = v1.begin(), j0 = b.v0.begin(), j1 = b.v1.begin();
                    i0 != v0.end();
                    ++i0, ++i1, ++j0, ++j1)
            {
                if((*i0 < *j0 && *i1 < *j0) || (*i0 > *j1 && *i1 > *j1)) {
                    return false;
                }
            }

            return true;
        }

        bool contains(const Vector<T, N>& v) const
        {
            auto it = v0.begin();
            auto jt = v1.begin();
            auto rt = v.begin();

            for(; it != v0.end(); ++it, ++jt, ++rt) {
                if(*rt < *it || *rt > *jt) {
                    return false;
                }
            }

            return true;
        }

        Box operator& (const Box& b) const
        {
            Vector<T, N> nv0, nv1;

            auto v0_it  = v0.begin();
            auto v1_it  = v1.begin();

            auto b0_it  = b.v0.begin();
            auto b1_it  = b.v1.begin();

            auto nv0_it = nv0.begin();
            auto nv1_it = nv1.begin();

            for(; v0_it != v0.end(); ++v0_it, ++v1_it, ++b0_it, ++b1_it, ++nv0_it, ++nv1_it)
            {
                *nv0_it = max(*v0_it, *b0_it);
                *nv1_it = min(*v1_it, *b1_it);
            }

            return Box(nv0, nv1);
        }

        Box operator+(const Vector<T, N>& v) const {
            return box(v0 + v, v1 + v);
        }

        Box operator-(const Vector<T, N>& v) const {
            return box(v0 - v, v1 - v);
        }

        bool operator==(const Box& b) const {
            return v0 == b.v0 && v1 == b.v1;
        }

        bool operator!=(const Box& b) const {
            return v0 != b.v0 || v1 != b.v1;
        }
    };


    struct Box3f : public Box<float, 3> {
        using Box<float, 3>::Box;
    };


    template <typename T, std::size_t N>
    inline constexpr Box<T, N> makeBox(const Vector<T, N>& v0, const Vector<T, N>& v1) {
        return Box<T, N>(v0, v1);
    }

    template <typename T, size_t N>
    inline Box<T, N> makeBox(const Vector<T, N>& v0, const Size<T, N>& sz) {
        return Box<T, N>(v0, sz);
    }

    template <size_t m,  size_t n, typename T>
    std::tuple<T, T> getRange(const Box<T, n>& box)
    {
        static_assert(m < n, "box dimension out of bounds");
        return std::make_tuple(std::get<m>(box.v0), std::get<m>(box.v1));
    }

    template <size_t m, size_t n, typename T>
    T getSize(const Box<T, n>& box)
    {
        static_assert(m < n, "box dimension out of bounds");
        return std::get<m>(box.v1) - std::get<m>(box.v0);
    }


    // Box trait
    namespace detail {
        template<typename, typename = void>
        struct isBox : std::false_type {};

        template<typename T>
        struct isBox<T,
                std::void_t<
                    typename T::value_type,
                    decltype(std::declval<T>().v0),
                    decltype(std::declval<T>().v1)>>
            : std::is_base_of<
                Box<typename T::value_type, decltype(std::declval<T>().v0)::size()>,
                T
        > {};
    }

    template <typename T>
    constexpr bool isBox = detail::isBox<T>::value;
}

#endif // LIBIM_BOX_H
