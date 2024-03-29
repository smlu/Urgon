#ifndef LIBIM_ABSTRACT_VECTOR_H
#define LIBIM_ABSTRACT_VECTOR_H
#include <array>
#include <functional>
#include <sstream>
#include <string>
#include <type_traits>

#include "math.h"
#include "fmath.h"

#include <libim/utils/utils.h>
#include <libim/types/safe_cast.h>


namespace libim {
    struct math_vector_tag {};

    template<typename Tag>
    constexpr bool isMathVectorTag = std::is_base_of_v<math_vector_tag, Tag>;

    template<typename Tag>
    using requireMathVectorTag = std::enable_if_t<isMathVectorTag<Tag>>;

    template<typename T, std::size_t S, typename Tag>
    struct AbstractVector : public std::array<T, S>
    {
        using base_type = std::array<T, S>;
        using tag_type  = Tag;

        static_assert(std::is_arithmetic_v<T>, "T must be arithmetic type!");

        using base_type::array;
        constexpr AbstractVector() : base_type{ 0 } {}
        constexpr AbstractVector(base_type a) : base_type(std::move(a)) {}

        /**
         * Construct a new vector object from string.
         *
         * @param vecstr - String representation of vector. The vector components in string should be deliminated with non-float numeric delims e.g. "(x/y/z/...)" , "x y z ...", "x,y, z,..." etc..
         * @param strict - If true, the string must be in the format of "(x/y/z/...)". If false, the string can be in any format.
         *                 By default false.
         *
         * @throw std::ios_base::failure if string is not in valid format,
         */
        AbstractVector(std::string_view vecstr, bool strict = false); // Definition in text_resource_reader.h

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

        constexpr inline bool isZero() const
        {
            for (const auto& e : *this)
            {
                if (e != T(0)) {
                    return false;
                }
            }

            return true;
        }

        /**
         * Serializes vector to string in format: "(x/y/z/...)"
         * @return Serialized vector.
         */
        std::string toString() const; // Definition in text_resource_writer.h

        template<typename TTag = Tag, typename = requireMathVectorTag<TTag>>
        constexpr inline auto& operator += (const AbstractVector& rhs)
        {
            for (std::size_t i = 0; i < this->size(); i++) {
                this->at(i) += rhs.at(i);
            }
            return *this;
        }

        template<typename TTag = Tag, typename = requireMathVectorTag<TTag>>
        constexpr inline auto& operator -= (const AbstractVector& rhs)
        {
            for (std::size_t i = 0; i < this->size(); i++) {
                this->at(i) -= rhs.at(i);
            }
            return *this;
        }

        template<typename TTag = Tag, typename = requireMathVectorTag<TTag>>
        constexpr inline auto& operator *= (const AbstractVector& rhs)
        {
            for (std::size_t i = 0; i < this->size(); i++) {
                this->at(i) *= rhs.at(i);
            }
            return *this;
        }

        template<typename TTag = Tag, typename = requireMathVectorTag<TTag>>
        constexpr inline auto& operator *= (float scalar)
        {
            for (float& c : *this) {
                c *= scalar;
            }
            return *this;
        }

        template<typename TTag = Tag, typename = requireMathVectorTag<TTag>>
        constexpr inline auto& operator /= (const AbstractVector& rhs)
        {
            for (std::size_t i = 0; i < this->size(); i++) {
                this->at(i) /= rhs.at(i);
            }
            return *this;
        }

        template<typename TTag = Tag, typename = requireMathVectorTag<TTag>>
        constexpr inline auto& operator /= (float scalar)
        {
            const float	invScalar = 1.0f / scalar;
            for (float& c : *this) {
                c *= invScalar;
            }
            return *this;
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

    // Vector trait && concept
    namespace detail {
        template<typename, typename = void>
        struct isVector : std::false_type {};

        template<typename T>
        struct isVector<T,
                std::void_t<
                    typename T::value_type,
                    typename T::size_type,
                    typename T::tag_type,
                    decltype(T::size())>>
            : std::is_base_of<
                AbstractVector<typename T::value_type, T::size(), typename T::tag_type>,
                T
        > {};
    }

    template <typename T>
    constexpr bool isVector = libim::detail::isVector<T>::value;

    template<typename T>
    using requireVector = std::enable_if_t<isVector<T>, T>;

    template<typename MVT>
    using requireMathVector = std::enable_if_t<isVector<MVT> && isMathVectorTag<typename MVT::tag_type>, MVT>;

    // Math operation definitions for math vector type
    template<typename MVT>
    requireMathVector<MVT> operator + (const MVT& l, const MVT& r)
    {
        MVT rv;
        using F = typename MVT::value_type;
        std::transform(l.begin(), l.end(), r.begin(), rv.begin(), std::plus<F>());
        return rv;
    }

    template<typename MVT>
    requireMathVector<MVT> operator - (const MVT& l, const MVT& r)
    {
        MVT rv;
        using F = typename MVT::value_type;
        std::transform(l.begin(), l.end(), r.begin(), rv.begin(), std::minus<F>());
        return rv;
    }

    template<typename MVT>
    requireMathVector<MVT> operator * (typename MVT::value_type scalar, const MVT& v)
    {
        MVT rv;
        using F = typename MVT::value_type;
        std::transform(v.begin(), v.end(), rv.begin(), [scalar](F x) { return scalar * x; });
        return rv;
    }

    template<typename MVT>
    requireMathVector<MVT> operator * (const MVT& v, typename MVT::value_type scalar)
    {
        MVT rv;
        using F = typename MVT::value_type;
        std::transform(v.begin(), v.end(), rv.begin(), [scalar](F x) { return x * scalar; });
        return rv;
    }

    template<typename MVT>
    requireMathVector<MVT> operator / (const MVT& v, typename MVT::value_type scalar)
    {
        MVT rv;
        using F = typename MVT::value_type;
        const F	is = static_cast<F>(1) / scalar;
        std::transform(v.begin(), v.end(), rv.begin(), [is](F x) { return x * is; });
        return rv;
    }

    template<typename MVT>
    requireMathVector<MVT> operator - (const MVT& v)
    {
        MVT rv;
        using F = typename MVT::value_type;
        std::transform(v.cbegin(), v.cend(), rv.begin(), std::negate<F>());
        return rv;
    }
}
#endif // LIBIM_ABSTRACT_VECTOR_H
