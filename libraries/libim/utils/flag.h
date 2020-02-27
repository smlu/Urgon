#ifndef LIBIM_FLAG_H
#define LIB_IMFLAG_H
#include "utils.h"

#include <utility>
#include <type_traits>

namespace libim::utils {
    template <typename T>
    class Flag {
    protected:
        static_assert (std::is_enum_v<T>, "T must be enum type");
        using UT = underlying_type_t<T>;

    public:
        Flag() = default;
        Flag(T f) : value(static_cast<UT>(f)) {}
        Flag(UT v) : value(v) {}

        Flag(std::initializer_list<T> initl)
        {
            value = 0;
            for(T val : initl) {
                value |= static_cast<UT>(val);
            }
        }

        explicit operator T() const
        {
            return T(value);
        }

        inline bool operator = (Flag val) const
        {
            return value = val.value;
        }

        inline bool operator & (T val) const
        {
            return value & static_cast<UT>(val);
        }

        inline bool operator & (Flag val) const
        {
            return value & val.value;
        }

        inline Flag operator | (T val) const
        {
            return Flag(value | static_cast<UT>(val));
        }

        inline Flag operator | (Flag val) const
        {
            return Flag(value | val.value);
        }

        inline Flag const &operator |= (T val)
        {
            value |= static_cast<UT>(val);
            return *this;
        }

        inline Flag const &operator |= (Flag val)
        {
            value |= val.value;
            return *this;
        }

        inline Flag operator - (T val) const
        {
            return Flag(value & ~static_cast<UT>(val));
        }

        inline Flag operator - (Flag val) const
        {
            return Flag(value & ~val.value);
        }

        inline Flag const &operator -= (T val)
        {
            value = value & ~static_cast<UT>(val);
            return *this;
        }

        inline Flag const &operator -= (Flag val)
        {
            value = value & ~val.value;
            return *this;
        }

        inline bool operator == (Flag val) const
        {
            return value == val.value;
        }

        inline bool operator != (Flag val) const
        {
            return value != val.value;
        }

    private:
        UT value = 0;
    };
}

#endif // LIBIM_FLAG_H
