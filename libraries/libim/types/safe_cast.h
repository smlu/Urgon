#ifndef LIBIM_SAFE_CAST_H
#define LIBIM_SAFE_CAST_H
#include <limits>
#include <typeinfo>
#include <type_traits>

namespace libim {

    class bad_safe_cast : std::bad_cast
    {
         const char* msg_;
    public:
        bad_safe_cast(const char* msg) : msg_(msg) {}
        virtual const char* what() const noexcept override {
            return msg_;
        }
    };

    namespace details {
        /** safe_cast function helper template struct */
        template <bool IsFromSigned, bool IsToSigned>
        struct safe_cast_impl;

        /** safe_cast_impl specialization for casting from an unsigned type into an unsigned type */
        template <>
        struct safe_cast_impl<false, false>
        {
            template <typename T1, typename T2>
            constexpr static inline T1 cast(T2 value)
            {
                return value >= std::numeric_limits<T1>::min() && value <= std::numeric_limits<T1>::max() ?
                    static_cast<T1>(value) : throw bad_safe_cast("Number to cast exceeds numeric limits");
            }
        };

        /** safe_cast_impl specialization for casting from an unsigned type into a signed type */
        template <>
        struct safe_cast_impl<false, true>
        {
            template <typename T1, typename T2>
            constexpr static inline T1 cast(T2 value)
            {
                return static_cast<T1>(value) >= std::numeric_limits<T1>::min() && value <= static_cast<T2>(std::numeric_limits<T1>::max()) ?
                    static_cast<T1>(value) : throw bad_safe_cast("Number to cast exceeds numeric limits");
            }
        };

        /** safe_cast_impl specialization for casting from a signed type into an unsigned type */
        template <>
        struct safe_cast_impl<true, false>
        {
            template <typename T1, typename T2>
            constexpr static inline T1 cast(T2 value)
            {
                // assuring a positive input, we can safely cast it into its unsigned type and check the numeric limits.
                using UnsignedFrom = typename std::make_unsigned<T2>::type;
                return value >= 0 &&
                    static_cast<UnsignedFrom>(value) >= std::numeric_limits<T1>::min() &&
                    static_cast<UnsignedFrom>(value) <= std::numeric_limits<T1>::max() ?
                    static_cast<T1>(value) : throw bad_safe_cast("Number to cast exceeds numeric limits");
            }
        };

        /** safe_cast_impl specialization for casting from a signed type into a signed type */
        template <>
        struct safe_cast_impl<true, true>
        {
            template <typename T1, typename T2>
            constexpr static inline T1 cast(T2 value)
            {
                return value >= std::numeric_limits<T1>::min() && value <= std::numeric_limits<T1>::max() ?
                    static_cast<T1>(value) : throw bad_safe_cast("Number to cast exceeds numeric limits");
            }
        };
    } // details

    /**
    * Returns casted integral number of type T2 to type T1.
    *
    * @param value - number to cast.
    * @return Casted input number.
    * @throw bad_safe_cast if type T2 cannot be casted to type T1 due to overflow.
    */
    template <typename T1, typename T2>
    [[nodiscard]] constexpr inline auto safe_cast(T2 value) ->
        typename std::enable_if<
            std::is_integral<T2>::value &&
            std::is_integral<T1>::value, T1
        >::type
    {
        return details::safe_cast_impl<
            std::numeric_limits<T2>::is_signed,
            std::numeric_limits<T1>::is_signed
        >::template cast<T1>(value);
    }
}
#endif // LIBIM_SAFE_CAST_H
