#ifndef LIBIM_FUNCTION_TRAITS_H
#define LIBIM_FUNCTION_TRAITS_H
#include <tuple>
#include <functional>

namespace libim::utils {
    namespace detail {
       template<class Ret, class... Args>
       struct function_traits_helper
       {
            static constexpr auto arity = sizeof...(Args);
            using return_type = Ret;

            template<std::size_t I>
            struct arg {
                using type = typename std::tuple_element<I,
                    std::tuple<Args...>
                >::type;
            };

            template<std::size_t I>
            using arg_t = typename arg<I>::type;
        };
    }

    template <class... Lambdas>
    struct overload : Lambdas... {
      explicit overload(Lambdas... lambdas) : Lambdas(std::move(lambdas))... {}

      using Lambdas::operator()...;
    };

    template <class... Lambdas>
    overload<Lambdas...> make_overload(Lambdas... lambdas) {
      return overload<Lambdas...>(std::move(lambdas)...);
    }

    template<class Ld>
    struct function_traits: function_traits<decltype(&Ld::template operator())>
    {};

    template<class Ret, class... Args>
    struct function_traits<Ret(Args...)> :
        detail::function_traits_helper<Ret, Args...>
    {};

    template<class Ret, typename ... Args>
    struct function_traits<Ret(*)(Args...)> :
        detail::function_traits_helper<Ret, Args...>
    {};

    template<class Ret, class Cls, class... Args>
    struct function_traits<Ret(Cls::*)(Args...)> :
        function_traits<Ret(Args...)>
    {};

    template<class Ret, class Cls, class... Args>
    struct function_traits<Ret(Cls::*)(Args...) const> :
        function_traits<Ret(Args...)>
    {};
}
#endif // LIBIM_FUNCTION_TRAITS_H
