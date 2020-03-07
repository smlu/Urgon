#ifndef LIBIM_ROTATOR_H
#define LIBIM_ROTATOR_H
#include "abstract_vector.h"

namespace libim {

    struct rotation_vector_tag {};

    template<typename T>
    struct Rotator :
        public AbstractVector<T, 3, rotation_vector_tag>
    {
        using Base_ = AbstractVector<T, 3, rotation_vector_tag>;
        using  AbstractVector<T, 3, rotation_vector_tag>::AbstractVector;

        constexpr inline Rotator(T pich, T roll, T yaw) noexcept :
            Base_{ pich, roll, yaw }
        {}

        explicit constexpr inline Rotator(std::array<T, 3> a) noexcept :
            Base_{ std::move(a) }
        {}

        inline T pitch() const
        {
            return this->at(0);
        }

        inline void setPitch(T p)
        {
            set(0, p);
        }

        inline T yaw() const
        {
            return this->at(1);
        }

        inline void setYaw(T p)
        {
            set(1, p);
        }
        
        inline T roll() const
        {
            return this->at(2);
        }

        inline void setRoll(T r)
        {
            set(2, r);
        }
    };

    using FRotator = Rotator<float>;
}
#endif // LIBIM_ROTATOR_H
