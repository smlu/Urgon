#ifndef LIBIM_BOX_H
#define LIBIM_BOX_H
#include <algorithm>
#include <cstdint>
#include <vector>
#include <tuple>

#include <libim/math/math.h>
#include <libim/math/size.h>
#include <libim/math/vector.h>

namespace libim::content::asset {
    template <typename T, std::size_t N> class Box
    {
        template <std::size_t m, std::size_t S, typename G>
        friend std::tuple<T, T> getRange(const Box<G, S>&);

        template <std::size_t m,  size_t S, typename G>
        friend T getSize(const Box<G, S>&);

    public:
        using value_type = T;

        Vector<T, N> min, max;

        constexpr Box() {}
        constexpr Box(const Vector<T, N>& min, const Vector<T, N>& max) :
            min(min), max(max)
        {}

        Box(const Vector<T, N>& min, const Size<T, N>& sz) : min(min) {
            std::transform(sz.begin(), sz.end(), min.begin(), max.begin(), std::plus<T>());
        }

        constexpr bool isZero() const
        {
            return min.isZero() && max.isZero();
        }

        bool overlaps(const Box& b) const
        {
            for(auto i0 = min.begin(), i1 = max.begin(), j0 = b.min.begin(), j1 = b.max.begin();
                    i0 != min.end();
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
            auto it = min.begin();
            auto jt = max.begin();
            auto rt = v.begin();

            for(; it != min.end(); ++it, ++jt, ++rt) {
                if(*rt < *it || *rt > *jt) {
                    return false;
                }
            }

            return true;
        }

        Box operator& (const Box& b) const
        {
            Vector<T, N> nv0, nv1;

            auto v0_it  = min.begin();
            auto v1_it  = max.begin();

            auto b0_it  = b.min.begin();
            auto b1_it  = b.max.begin();

            auto nv0_it = nv0.begin();
            auto nv1_it = nv1.begin();

            for(; v0_it != min.end(); ++v0_it, ++v1_it, ++b0_it, ++b1_it, ++nv0_it, ++nv1_it)
            {
                *nv0_it = max(*v0_it, *b0_it);
                *nv1_it = min(*v1_it, *b1_it);
            }

            return Box(nv0, nv1);
        }

        Box operator+(const Vector<T, N>& v) const {
            return box(min + v, max + v);
        }

        Box operator-(const Vector<T, N>& v) const {
            return box(min - v, max - v);
        }

        bool operator==(const Box& b) const {
            return min == b.min && max == b.max;
        }

        bool operator!=(const Box& b) const {
            return min != b.min || max != b.max;
        }
    };


    struct Box3f : public Box<float, 3> {
        using Box<float, 3>::Box;
    };


    template <typename T, std::size_t N>
    inline constexpr Box<T, N> makeBox(const Vector<T, N>& min, const Vector<T, N>& max) {
        return Box<T, N>(min, max);
    }

    template <typename T, size_t N>
    inline Box<T, N> makeBox(const Vector<T, N>& min, const Size<T, N>& sz) {
        return Box<T, N>(min, sz);
    }

    template <size_t m,  size_t n, typename T>
    std::tuple<T, T> getRange(const Box<T, n>& box)
    {
        static_assert(m < n, "box dimension out of bounds");
        return std::make_tuple(std::get<m>(box.min), std::get<m>(box.max));
    }

    template <size_t m, size_t n, typename T>
    T getSize(const Box<T, n>& box)
    {
        static_assert(m < n, "box dimension out of bounds");
        return std::get<m>(box.max) - std::get<m>(box.min);
    }


    // Box trait
    namespace detail {
        template<typename, typename = void>
        struct isBox : std::false_type {};

        template<typename T>
        struct isBox<T,
                std::void_t<
                    typename T::value_type,
                    decltype(std::declval<T>().min),
                    decltype(std::declval<T>().max)>>
            : std::is_base_of<
                Box<typename T::value_type, decltype(std::declval<T>().min)::size()>,
                T
        > {};
    }

    template <typename T>
    constexpr bool isBox = detail::isBox<T>::value;
}
#endif // LIBIM_BOX_H
