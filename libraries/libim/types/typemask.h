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
    class TypeMask {
    protected:
        static_assert (std::is_enum_v<T>, "T must be enum type");
        using UT = utils::underlying_type_t<T>;

    public:
        TypeMask() = default;
        TypeMask(T f) : value(toFlag(f)) {}
        explicit TypeMask(UT v) : value(v) {}

        TypeMask(std::initializer_list<T> initl)
        {
            value = 0;
            for(T val : initl) {
                value |= toFlag(val);
            }
        }

        explicit operator T() const
        {
            return T(value);
        }

        inline const TypeMask& operator = (T val)
        {
            value = toFlag(val);
            return *this;
        }

        inline bool operator & (T val) const
        {
            return value & toFlag(val);
        }

        inline bool operator & (TypeMask val) const
        {
            return value & val.value;
        }

        inline TypeMask operator | (T val) const
        {
            return TypeMask(value | toFlag(val));
        }

        inline TypeMask operator | (TypeMask val) const
        {
            return TypeMask(value | val.value);
        }

        inline const TypeMask& operator |= (T val)
        {
            value |= toFlag(val);
            return *this;
        }

        inline const TypeMask& operator |= (TypeMask val)
        {
            value |= val.value;
            return *this;
        }

        inline TypeMask operator - (T val) const
        {
            return TypeMask(value & ~toFlag(val));
        }

        inline TypeMask operator - (TypeMask val) const
        {
            return TypeMask(value & ~val.value);
        }

        inline const TypeMask& operator -= (T val)
        {
            value = value & ~toFlag(val);
            return *this;
        }

        inline const TypeMask& operator -= (TypeMask val)
        {
            value = value & ~val.value;
            return *this;
        }

        inline bool operator == (TypeMask val) const
        {
            return value == val.value;
        }

        inline bool operator != (TypeMask val) const
        {
            return value != val.value;
        }

    private:
        static UT toFlag(T t) {
            return 1 << static_cast<UT>(t);
        }

    private:
        UT value = 0;
    };
}
#endif // LIBIM_TYPEMASK_H
