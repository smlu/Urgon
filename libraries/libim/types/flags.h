#ifndef LIBIM_FLAG_H
#define LIBIM_FLAG_H
#include <libim/utils/utils.h>
#include <utility>
#include <type_traits>

namespace libim {
    template <typename T>
    class Flags final {
        static_assert (std::is_enum_v<T>, "T must be enum type");
        using UT = utils::underlying_type_t<T>;

    public:
        using FlagType = T;

        constexpr Flags() = default;
        constexpr Flags(T f) : value_(f) {}
        constexpr Flags(UT v) : value_(static_cast<T>(v)) {}

        constexpr Flags(std::initializer_list<T> initList)
        {
            value_ = 0;
            for(T v : initList) {
                *this |= v;
            }
        }

        explicit constexpr operator T() const
        {
            return value_;
        }

        inline constexpr bool operator & (T val) const
        {
            return uv(value_) & uv(val);
        }

        inline constexpr bool operator & (Flags val) const
        {
            return uv(*this) & uv(val);
        }

        inline constexpr Flags operator | (T val) const
        {
            return uv(value_) | uv(val);
        }

        inline constexpr Flags operator | (Flags val) const
        {
            return uv(*this) | uv(val);
        }

        inline constexpr Flags& operator |= (T val)
        {
            *this = uv(value_) | uv(val);
            return *this;
        }

        inline constexpr Flags& operator |= (Flags val)
        {
            *this = uv(*this) | uv(val);
            return *this;
        }

        inline constexpr Flags operator - (T val) const
        {
            return uv(value_) & ~uv(val);
        }

        inline constexpr Flags operator - (Flags val) const
        {
            return uv(*this) & ~uv(val);
        }

        inline constexpr Flags& operator -= (T val)
        {
            *this = uv(value_) & ~uv(val);
            return *this;
        }

        inline constexpr Flags& operator -= (Flags val)
        {
            *this = uv(*this) & ~uv(val);
            return *this;
        }

        inline constexpr bool operator == (Flags val) const
        {
            return value_ == val.value_;
        }

        inline constexpr bool operator != (Flags val) const
        {
            return value_ != val.value_;
        }

        inline constexpr T value() const
        {
            return value_;
        }

    private:
        static constexpr UT uv(Flags f) {
            return utils::to_underlying(f.value_);
        }

        static constexpr UT uv(T v) {
            return utils::to_underlying(v);
        }

    private:
        T value_;
    };

    namespace utils {
        template<typename T>
        [[nodiscard]] inline constexpr auto to_underlying(Flags<T> f) {
            return to_underlying(f.value());
        }

        template<typename T>
        struct underlying_type<Flags<T>> {
            using type = std::underlying_type_t<T>;
        };
    }
}

#endif // LIBIM_FLAG_H
