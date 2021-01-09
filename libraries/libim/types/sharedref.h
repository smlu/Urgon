#ifndef LIBIM_SHAREDREF_H
#define LIBIM_SHAREDREF_H
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace libim {

    template<typename T>
    class WeakRef;

    /** SharedRef represents owning non-nullable shared pointer **/
    template<typename T>
    class SharedRef
    {
        using DT = std::decay_t<T>;
    public:
        SharedRef()
        {
            static_assert(std::is_default_constructible_v<DT>, "T must be default constructible");
            ptr_ = std::make_shared<DT>();
        }

        SharedRef(const T& v)
        {
            ptr_ = std::make_shared<DT>(v);
        }

        SharedRef(T&& v)
        {
            ptr_ = std::make_shared<DT>(std::move(v));
        }

        // template<typename... Args, typename std::enable_if_t<!std::is_same_v<Args..., SharedRef<DT>>>* = nullptr>
        // explicit SharedRef(Args&& ... args)
        // {
        //     ptr_ = std::make_shared<DT>(std::forward<Args>(args)...);
        // }

        SharedRef(const SharedRef&) noexcept = default;
        SharedRef(SharedRef&&) noexcept = default;

        SharedRef& operator = (const SharedRef&) noexcept = default;
        SharedRef& operator = (SharedRef&&) noexcept = default;

        template<typename U>
        SharedRef(const SharedRef<U>& other)
        {
            ptr_ = other.ptr_;
        }

        template<typename U>
        SharedRef(SharedRef<U>&& other)
        {
            ptr_ = std::move(other.ptr_);
        }

        auto& get() const
        {
            return *ptr_;
        }

        DT& operator*() const
        {
            return *ptr_;
        }

        T* operator->() const
        {
            return ptr_.get();
        }

        void swap(SharedRef& r) noexcept
        {
            std::swap(ptr_, r);
        }

        std::size_t getSharedCount() const
        {
            return ptr_.use_count(); // Note: in multithreaded environment use_count returns an approximation
        }

        bool isUnique() const
        {
            return getSharedCount() == 1;
        }

    private:
        template <typename>
        friend class SharedRef;
        friend class WeakRef<DT>;
        template<typename T, typename ... Args>
        friend SharedRef<T> makeSharedRef(Args&& ...);
        SharedRef(std::shared_ptr<T>&& ptr) :
            ptr_(std::move(ptr))
        {
            assert(bool(ptr_));
        }

        std::shared_ptr<DT> ptr_;
    };

    template<typename T, typename ... Args>
    SharedRef<T> makeSharedRef(Args&& ... args)
    {
        SharedRef<T> ref(
            std::make_shared<T>(std::forward<Args>(args)...)
        );
        return ref;
    }


    /** WeakRef represents non-owning non-nullable shared weak pointer **/
    template<typename T>
    class WeakRef
    {
        using DT = std::decay_t<T>;
    public:
        constexpr WeakRef() noexcept = default;
        WeakRef(const WeakRef<DT>& rhs) noexcept :
            wptr_(rhs.wptr_)
        {}

        WeakRef(WeakRef<DT>&& rhs) noexcept:
            wptr_(std::move(rhs.wptr_))
        {}

        WeakRef(const SharedRef<DT>& sref) noexcept
        {
            wptr_ = sref.ptr_;
        }

        WeakRef& operator = (const WeakRef<DT>& rhs) noexcept
        {
            wptr_ = rhs.wptr_;
        }

        WeakRef& operator = (WeakRef<DT>&& rhs) noexcept
        {
            wptr_ = std::move(rhs.wptr_);
        }

        WeakRef& operator = (const SharedRef<DT>& sref) noexcept
        {
            wptr_ = sref.ptr_;
        }

        bool isValid() const
        {
            return !wptr_.expired();
        }

        explicit operator bool() const
        {
            return isValid();
        }

        SharedRef<DT> pin() const
        {
            return wptr_.lock();
        }

    private:
        std::weak_ptr<DT> wptr_;
    };
}

#endif // LIBIM_SHAREDREF_H
