#ifndef LIBIM_TYPEMASK_H
#define LIBIM_TYPEMASK_H
#include <libim/utils/utils.h>
#include <utility>
#include <type_traits>

namespace libim {
    /**
     * Class represents enum type enumerators as bit flags.
    *  e.g. enumerator A = 5 is convert to flag (0x1 << 5) aka 0x20
    */
    template <typename T>
    class TypeMask final {
        static_assert (std::is_enum_v<T>, "T must be enum type");
        using UT = utils::underlying_type_t<T>;

    public:
        constexpr TypeMask() = default;
        constexpr TypeMask(T f) : value_(toFlag(f)) {}
        constexpr explicit TypeMask(UT v) : value_(v) {}

        constexpr TypeMask(std::initializer_list<T> initList)
        {
            value_ = 0;
            for(T v : initList) {
                value_ |= toFlag(v);
            }
        }

        explicit constexpr operator T() const
        {
            return T(value_);
        }

        inline constexpr TypeMask& operator = (T val)
        {
            value_ = toFlag(val);
            return *this;
        }

        inline constexpr bool operator & (T val) const
        {
            return value_ & toFlag(val);
        }

        inline constexpr bool operator & (TypeMask val) const
        {
            return value_ & val.value_;
        }

        inline constexpr TypeMask operator | (T val) const
        {
            return TypeMask(value_ | toFlag(val));
        }

        inline constexpr TypeMask operator | (TypeMask val) const
        {
            return TypeMask(value_ | val.value_);
        }

        inline constexpr TypeMask& operator |= (T val)
        {
            value_ |= toFlag(val);
            return *this;
        }

        inline constexpr TypeMask& operator |= (TypeMask val)
        {
            value_ |= val.value_;
            return *this;
        }

        inline constexpr TypeMask operator - (T val) const
        {
            return TypeMask(value_ & ~toFlag(val));
        }

        inline constexpr TypeMask operator - (TypeMask val) const
        {
            return TypeMask(value_ & ~val.value_);
        }

        inline constexpr TypeMask& operator -= (T val)
        {
            value_ = value_ & ~toFlag(val);
            return *this;
        }

        inline constexpr TypeMask& operator -= (TypeMask val)
        {
            value_ = value_ & ~val.value_;
            return *this;
        }

        inline constexpr bool operator == (TypeMask val) const
        {
            return value_ == val.value_;
        }

        inline constexpr bool operator != (TypeMask val) const
        {
            return value_ != val.value_;
        }

        constexpr T value() const
        {
            return T(value_);
        }

    private:
        static UT toFlag(T t) {
            return 1 << static_cast<UT>(t);
        }

    private:
        UT value_ = 0;
    };


    namespace utils {
        template<typename T>
        [[nodiscard]] inline constexpr auto to_underlying(TypeMask<T> tm) {
            return to_underlying(tm.value());
        }

        template<typename T>
        struct underlying_type<TypeMask<T>> {
            using type = std::underlying_type_t<T>;
        };
    }
}
#endif // LIBIM_TYPEMASK_H
