#ifndef LIBIM_OPTREF_H
#define LIBIM_OPTREF_H
#include <functional>
#include <optional>

namespace libim {
    template<typename T>
    struct OptionalRef : std::optional<std::reference_wrapper<T>>
    {
        using base = std::optional<std::reference_wrapper<T>>;
        using std::optional<std::reference_wrapper<T>>::optional;

        constexpr bool hasValue() const noexcept {
            return this->has_value();
        }

        constexpr auto value() const & {
            return base::value().get();
        }

        constexpr auto operator->() const {
            return &base::value().get();
        }

        constexpr auto operator*() const & {
            return this->value();
        }

        template<class U>
        constexpr auto valueOr(U&& defaultValue) const& {
            return this->value_or(defaultValue);
        }

    private:
        constexpr bool has_value() const noexcept {
            return base::has_value();
        }

        template<class U>
        constexpr auto value_or(U&& defaultValue) const& {
            return bool(*this) ? **this : static_cast<T>(std::forward<U>(defaultValue));
        }

        constexpr T&& value() && = delete;
        constexpr const T&& value() const && = delete;
        template<class U>
        constexpr auto valueOr(U&& defaultValue) && = delete;
        template<class U>
        constexpr auto value_or(U&& defaultValue) && = delete;
        constexpr const T&& operator*() const&& = delete;
        constexpr T&& operator*() && = delete;
    };
}
#endif // LIBIM_OPTREF_H
