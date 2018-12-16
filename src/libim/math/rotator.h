#ifndef LIBIM_ROTATOR_H
#define LIBIM_ROTATOR_H
#include "abstract_vector.h"

namespace libim {

    struct rotation_vector_tag {};

    template<typename T>
    struct Rotator :
        public AbstractVector<T, 3, rotation_vector_tag>
    {
        using _PT = AbstractVector<T, 3, rotation_vector_tag>;
        using _PT::AbstractVector;

        constexpr inline Rotator(T pich, T roll, T yaw) noexcept :
            _PT{ pich, roll, yaw }
        {}

        explicit constexpr inline Rotator(std::array<T, 3> a) noexcept :
            _PT{std::move(a)}
        {}

        inline T roll() const
        {
            return this->at(0);
        }

        inline void setRoll(T r)
        {
            set(0, r);
        }


        inline T yaw() const
        {
            return this->at(1);
        }

        inline void setYaw(T p)
        {
            set(1, p);
        }

        inline T pitch() const
        {
            return this->at(2);
        }

        inline void setPitch(T p)
        {
            set(2, p);
        }
    };

    using FRotator = Rotator<float>;
}
#endif // LIBIM_ROTATOR_H
