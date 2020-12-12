#ifndef LIBIM_HASHMAP_H
#define LIBIM_HASHMAP_H
#include <cstdint>
#include <deque>
#include <functional>
#include <iterator>
#include <list>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace libim {

    /** Sequence ordered list which elements are indexed and mapped to the key. */
    template<typename T, typename KeyT = std::string>
    class HashMap
    {
        template<typename, typename>
        class HashMapIterator;
        template<typename, typename> friend class HashMapIterator;

    public:
        using size_type = std::size_t;
        using Idx = size_type;

        using ContainerElement = std::pair<KeyT, T>;
        using ContainerType = std::list<ContainerElement>;

        using key_type = KeyT;
        using key_reference = key_type&;
        using key_const_reference =  const key_type&;
        using key_rvalue = key_type&&;
        using value_type = T;
        using reference = value_type&;
        using const_reference =  const value_type&;

        using iterator = HashMapIterator<T, typename ContainerType::iterator>;
        using const_iterator = HashMapIterator<T, typename ContainerType::const_iterator>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        HashMap() = default;
        HashMap(HashMap&&) = default;
        HashMap& operator = (HashMap&&) noexcept = default;

        HashMap(const HashMap& rhs) : data_(rhs.data_)
        {
            reconstruct_map();
        }

        HashMap& operator = (const HashMap& rhs)
        {
            if(&rhs != this)
            {
                data_ = rhs.data_;
                reconstruct_map();
            }
            return *this;
        }

        iterator begin() noexcept
        {
            return iterator(std::begin(data_));
        }

        const_iterator begin() const noexcept
        {
            return const_iterator(std::cbegin(data_));
        }

        const_iterator cbegin() const noexcept
        {
            return const_iterator(std::cbegin(data_));
        }

        iterator rbegin() noexcept
        {
            return iterator(std::rbegin(data_));
        }

        const_iterator rbegin() const noexcept
        {
            return const_iterator(std::crbegin(data_));
        }

        const_iterator crbegin() const noexcept
        {
            return const_iterator(std::crbegin(data_));
        }

        iterator end() noexcept
        {
            return iterator(std::end(data_));
        }

        const_iterator end() const noexcept
        {
            return const_iterator(std::end(data_));
        }
        const_iterator cend() const noexcept
        {
            return const_iterator(std::end(data_));
        }

        iterator rend() noexcept
        {
            return iterator(std::rend(data_));
        }

        const_iterator rend() const noexcept
        {
            return const_iterator(std::crend(data_));
        }

        const_iterator crend() const noexcept
        {
            return const_iterator(std::crend(data_));
        }

        reference at(key_const_reference key)
        {
            return map_.at(key)->second;
        }

        const_reference at(key_const_reference key) const
        {
            return map_.at(key)->second;
        }

        reference at(Idx idx)
        {
            return index_.at(idx)->second;
        }

        const_reference at(Idx idx) const
        {
            return index_.at(idx)->second;
        }

        reference operator[](Idx idx)
        {
            return index_.at(idx)->second;
        }

        const_reference operator[](Idx idx) const
        {
            return index_.at(idx)->second;
        }

        reference operator[](key_const_reference key)
        {
            auto it = map_.find(key);
            if(it == map_.end()) {
                return *emplaceBack(key).first;
            }
            return it->second->second;
        }

        reference operator[](key_rvalue key)
        {
            auto it = map_.find(key);
            if(it == map_.end()) {
                return *emplaceBack(std::move(key)).first;
            }
            return it->second->second;
        }

        const_reference operator[](key_const_reference key) const
        {
            auto it = map_.find(key);
            if(it == map_.end()) {
                data_.at(data_.size()); // should throw std::out_of_range
            }
            return it->second;
        }

        iterator find(key_const_reference key)
        {
            auto it = map_.find(key);
            if(it == map_.end()) {
                return end();
            }
            return it->second;
        }

        const_iterator find(key_const_reference key) const
        {
            auto it = map_.find(key);
            if(it == map_.end()) {
                return end();
            }
            return iterator(it->second);
        }

        reference front()
        {
            return data_.front().data;
        }

        const_reference front() const
        {
            return data_.front().data;
        }

        reference back()
        {
            return data_.back().data;
        }

        const_reference back() const
        {
            return data_.back().data;
        }

        template< class... Args >
        std::pair<reference, bool> emplace(const_iterator pos, key_type key, Args&&... args)
        {
            return insert(pos, std::move(key), T{ std::forward<Args>(args)... });
        }

        std::pair<iterator, bool> insert(const_iterator pos, key_type key, const T& value)
        {
            auto iidx = get_itr_idx(pos);
            return insert(iidx, std::move(key), value);
        }

        std::pair<iterator, bool> insert(const_iterator pos, key_type key, T&& value)
        {
            auto iidx = get_itr_idx(pos);
            return insert(iidx, std::move(key), std::move(value));
        }

        std::pair<iterator, bool> insert(Idx pos, key_type key, const T& value)
        {
            auto mapIt = map_.find(key);
            if(mapIt != map_.end()) {
                return { mapIt->second, false };
            }

            if( pos > size()) {
                pos = isEmpty() ? 0 : size();
            }

            auto it = data_.end();
            auto iit = index_.begin() + pos;
            if( iit != index_.end()) {
                it = data_.insert(*iit, { std::move(key), value });
            } else
            {
                data_.push_back({ std::move(key), value });
                it = --data_.end();
            }

            map_[std::cref(it->first)] = it;
            index_.insert(iit, it);
            return { it, true };
        }

        std::pair<iterator, bool> insert(Idx pos, key_type key, T&& value)
        {
            auto mapIt = map_.find(key);
            if(mapIt != map_.end()) {
                return { mapIt->second, false };
            }

            if( pos > size()) {
                pos = isEmpty() ? 0 : size();
            }

            auto it = data_.end();
            auto iit = index_.begin() + pos;
            if( iit != index_.end()) {
                it = data_.insert(*iit, { std::move(key), std::move(value) });
            } else
            {
                data_.push_back({ std::move(key), std::move(value) });
                it = --data_.end();
            }

            map_[std::cref(it->first)] = it;
            index_.insert(iit, it);
            return { it, true };
        }

        template< class... Args >
        std::pair<iterator, bool> emplaceFront(key_type key, Args&&... args)
        {
            return pushFront(std::move(key), T{ std::forward<Args>(args)... });
        }

        std::pair<iterator, bool> pushFront(key_type key, const T& value)
        {
            return insert(0, std::move(key), value);
        }

        std::pair<iterator, bool> pushFront(key_type key, T&& value)
        {
            return insert(0, std::move(key), std::move(value));
        }

        template< class... Args >
        std::pair<iterator, bool> emplaceBack(key_type key, Args&&... args)
        {
            return pushBack(std::move(key), T{ std::forward<Args>(args)... });
        }

        std::pair<iterator, bool> pushBack(key_type key, const T& value)
        {
            return insert(size(), std::move(key), value);
        }

        std::pair<iterator, bool> pushBack(key_type key, T&& value)
        {
            return insert(size(), std::move(key), std::move(value));
        }

        iterator erase(const_iterator pos)
        {
            return erase_by_idx(get_itr_idx(pos));
        }

        void erase(Idx pos)
        {
            erase_by_idx(pos);
        }

        void erase(key_const_reference key)
        {
            auto mapIt = map_.find(key);
            if(mapIt != map_.end())
            {
                auto it = mapIt->second;
                erase_by_idx(get_itr_idx(it));
            }
        }

        inline void clear() noexcept
        {
            data_.clear();
            index_.clear();
            map_.clear();
        }

        inline bool contains(key_const_reference key) const
        {
            return map_.count(key) > 0;
        }

        inline bool isEmpty() const
        {
            return data_.empty();
        }

        inline size_type size() const
        {
            return data_.size();
        }

        inline const ContainerType& container() const // !< returns list of ContainerElements
        {
            return data_;
        }

        void reserve(size_type n)
        {
            map_.reserve(n);
        }

        void swap(HashMap& other) noexcept
        {
            data_.swap(other.data_);
            index_.swap(other.index_);
            map_.swap(other.map_);
        }

    private:
        Idx get_itr_idx(typename ContainerType::const_iterator itr) const
        {
            return std::distance(data_.begin(), itr);
        }

        iterator erase_by_idx(Idx idx)
        {
            auto it = end();
            if(idx < index_.size())
            {
                it = index_.at(idx);
                map_.erase(it.it_->first);
                index_.erase(index_.begin() + idx);
                it = data_.erase(it.it_);
            }

            return it;
        }

        void reconstruct_map()
        {
            map_.clear();
            index_.clear();
            map_.reserve(data_.size());
            for(auto it = data_.begin(); it != data_.end(); it++)
            {
                map_[std::cref(it->first)] = it;
                index_.push_back(it);
            }
        }

    private:
        using IndexType = std::deque<typename ContainerType::iterator>;
        using MapType = std::unordered_map<
            std::reference_wrapper<const KeyT>,
            typename ContainerType::iterator,
            std::hash<KeyT>,
            std::equal_to<KeyT>
        >;

        ContainerType data_;
        IndexType index_;
        MapType map_;
    };


    template<typename T,typename KeyT>
    template <typename ValueT, typename ContainerIterator>
    class HashMap<T, KeyT>::HashMapIterator
    {
        using IterTriats = std::iterator_traits<ContainerIterator>;
        ContainerIterator it_;

        operator ContainerIterator() const
        {
            return it_;
        }

        friend class HashMap<T, KeyT>;
        HashMapIterator(ContainerIterator it) : it_(it) {}

    public:
        using iterator_category = typename IterTriats::iterator_category;
        using value_type = ValueT;
        using difference_type = typename IterTriats::difference_type;
        using pointer = std::conditional_t<std::is_const_v<typename IterTriats::pointer>, const value_type*, value_type*>;
        using reference = std::conditional_t<std::is_const_v<typename IterTriats::reference>, const value_type, value_type>&;

        template<typename U, typename W>
        HashMapIterator(HashMapIterator<U, W> other) : it_(other.it_)
        {}

        reference operator* () const
        {
            return const_cast<reference>(it_->second);
        }

        pointer operator-> () const
        {
            return const_cast<pointer>(&it_->second);
        }

        reference operator[] (difference_type n) const
        {
            return it_.operator[](n)->second;
        }

        inline HashMapIterator operator+ (difference_type n) const
        {
            return HashMapIterator(it_ + n);
        }

        HashMapIterator& operator++ ()
        {
            ++it_;
            return *this;
        }

        HashMapIterator operator++ (int)
        {
            return HashMapIterator(it_++);
        }

        HashMapIterator& operator+= (difference_type n)
        {
            it_ += n;
            return *this;
        }

        HashMapIterator operator- (difference_type n) const
        {
            return HashMapIterator(it_ - n);
        }

        HashMapIterator& operator-- ()
        {
            --it_;
            return *this;
        }

        HashMapIterator operator-- (int)
        {
            return HashMapIterator(it_--);
        }

        HashMapIterator& operator-= (difference_type n)
        {
            it_ -= n;
            return *this;
        }

        bool operator == (const HashMapIterator& rhs) const
        {
            return it_ == rhs.it_;
        }

        bool operator != (const HashMapIterator& rhs) const
        {
            return it_ != rhs.it_;
        }

        bool operator < (const HashMapIterator& rhs) const
        {
            return it_ < rhs.it_;
        }

        bool operator > (const HashMapIterator& rhs) const
        {
            return it_ > rhs.it_;
        }

        bool operator <= (const HashMapIterator& rhs) const
        {
            return it_ <= rhs.it_;
        }

        bool operator >= (const HashMapIterator& rhs) const
        {
            return it_ >= rhs.it_;
        }
    };
}
#endif // LIBIM_HASHMAP_H
