#ifndef LIBIM_TRAITS_H
#define LIBIM_TRAITS_H
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

    // Triats

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

}
#endif // LIBIM_TRAITS_H
