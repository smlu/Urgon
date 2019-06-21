#ifndef LIBIM_SHAREDREF_H
#define LIBIM_SHAREDREF_H
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace libim::utils {

    template<typename T>
    class WeakRef;

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

        SharedRef(std::shared_ptr<DT> ptr)
        {
            if(!ptr) {
                std::runtime_error("SharedRef: ptr is null");
            }
            ptr_ = ptr;
        }

        SharedRef(const SharedRef&) noexcept = default;
        SharedRef(SharedRef&&) noexcept = default;

        SharedRef& operator = (const SharedRef&) noexcept = default;
        SharedRef& operator = (SharedRef&&) noexcept = default;

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
            return *ptr_;
        }

        void swap(std::shared_ptr<DT>& r) noexcept
        {
            if(r) {
                std::swap(ptr_, r);
            }
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
        friend class WeakRef<DT>;
        std::shared_ptr<DT> ptr_;
    };





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
