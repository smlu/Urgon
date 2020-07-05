#ifndef LIBIM_TRAITS_H
#define LIBIM_TRAITS_H
#include <libim/types/flags.h>
#include <libim/types/typemask.h>
#include <libim/math/abstract_vector.h>

#include <array>
#include <type_traits>

namespace libim::utils {

    namespace detail {

        template <class T, class=void>
        struct has_capacity : std::false_type{};

        template <class C>
        struct has_capacity<C,
                std::void_t<decltype(std::declval<C>().capacity())>>
            : std::true_type{};

        template <class T, class=void>
        struct has_push_back : std::false_type{};

        template <class C>
        struct has_push_back<C,
                std::void_t<decltype(std::declval<C>().push_back(typename C::value_type{}))>>
            : std::true_type{};


        template <class T, class=void>
        struct has_no_pos_insert : std::false_type{};

        template <class C>
        struct has_no_pos_insert<C,
                std::void_t<decltype(std::declval<C>().insert(typename C::value_type{}))>>
            : std::true_type{};


        template <class T, class=void>
        struct has_reserve : std::false_type{};

        template <class C>
        struct has_reserve<C,
                std::void_t<decltype(std::declval<C>().reserve(typename C::size_type{}))>>
            : std::true_type{};

        template<typename>
        struct is_flags : std::false_type {};

        template<typename T>
        struct is_flags<Flags<T>> : std::true_type {};

        template<typename>
        struct is_std_array : std::false_type {};

        template<typename T, std::size_t N>
        struct is_std_array<std::array<T, N>> : std::true_type {};

        template<typename>
        struct is_numeric_std_array : std::false_type {};

        template<typename T, std::size_t N>
        struct is_numeric_std_array<std::array<T, N>> : std::is_arithmetic<T> {};

        template<typename>
        struct is_typemask : std::false_type {};

        template<typename T>
        struct is_typemask<TypeMask<T>> : std::true_type {};

        template<typename>
        struct array_size;

        template<typename T, size_t N>
        struct array_size<std::array<T,N>> {
            static constexpr size_t size = N;
        };

        template<typename T, size_t N>
        struct array_size<T[N]> {
            static size_t const size = N;
        };
    }


    // Poor man's concepts

    template <class Container>
    using requires_container = std::void_t<
        decltype(std::declval<Container const>().size()),
        typename Container::const_iterator,
        typename Container::iterator,
        typename Container::size_type,
        typename Container::value_type
    >;

    /* Triats */

    // Does C have member function 'capacity'
    template<typename C>
    constexpr bool has_mf_capacity = detail::has_capacity<C>::value;

    // Does C have member function 'push_back'
    template<typename C>
    constexpr bool has_mf_push_back = detail::has_push_back<C>::value;

    // Does C have no positional member function 'insert'
    template<typename C>
    constexpr bool has_no_pos_mf_insert = detail::has_no_pos_insert<C>::value;

    // Does C have member function 'reserve'
    template<typename C>
    constexpr bool has_mf_reserve = detail::has_reserve<C>::value;

    // Is T of type std::array<T,N>
    template<typename T>
    constexpr bool isStdArray = detail::is_std_array<T>::value;

    // Is T numeric std::array<T, N> type
    template<typename T>
    constexpr bool isNumericStdArray = detail::is_numeric_std_array<T>::value;

    // Is T of type Flags
    template<typename T>
    constexpr bool isFlags = detail::is_flags<T>::value;

    // Is T of type TypeMask
    template<typename T>
    constexpr bool isTypeMask = detail::is_typemask<T>::value;

    // is T of type enum or Flags or TypeMask
    template<typename T>
    constexpr bool isEnum = std::is_enum_v<T> || isFlags<T> || isTypeMask<T>;

    /* Utility type triats */
    template<typename T>
    constexpr std::size_t arraySize = detail::array_size<T>::size;
}
#endif // LIBIM_TRAITS_H
