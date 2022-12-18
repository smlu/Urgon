#ifndef LIBIM_INDEXMAP_H
#define LIBIM_INDEXMAP_H
#include <cstdint>
#include <deque>
#include <functional>
#include <iterator>
#include <list>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include <libim/platform.h>
#include <libim/types/string_map.h>
#include <libim/utils/traits.h>

namespace libim {

    /** A case-insensitive string hash function. */
    struct StringCaseInsensitiveHash
    {
        size_t operator()(std::string_view val) const
        {
            // FNV-1a hash function
            // TODO: Use faster hash function for smaller strings with better avalanche effect.
            //       Check MurmurHash3, CityHash....
            #if defined(LIBIM_PLATFORM_64BIT)
                static constexpr size_t fnvOffsetBasis = 14695981039346656037ULL;
                static constexpr size_t fnvPrime       = 1099511628211ULL;
            #else
                static constexpr size_t fnvOffsetBasis = 2166136261U;
                static constexpr size_t fnvPrime       = 16777619U;
            #endif

            std::size_t hash = fnvOffsetBasis;
            for (auto c : val)
            {
                hash ^= static_cast<std::size_t>(std::tolower(c));
                hash *= fnvPrime;
            }
            return hash;
        }
    };

    /**
     * Hash table which elements are ordered by insertion and mapped to the key.
     * Each element can be retrieved by the key or by the index.
     *
     * @tparam T        - The value type.
     * @tparam KeyT     - The key type. By default std::string.
     * @tparam Hash     - The hash function type. By default, StringCaseInsensitiveHash is used if KeyT is std::string, else std::hash<KeyT>.
     * @tparam KeyEqual - The key equality function type. By default, StringCaseInsensitiveEqual is used if KeyT is std::string, else std::equal_to<KeyT>.
     */
    template<typename T,
        typename KeyT = std::string,
        typename Hash = std::conditional_t<utils::isStdString<KeyT>, StringCaseInsensitiveHash, std::hash<KeyT>>,
        typename KeyEqual = std::conditional_t<utils::isStdString<KeyT>, StringCaseInsensitiveEqual, std::equal_to<KeyT>>
    >
    class IndexMap
    {
        static_assert(std::is_same_v<KeyT, std::string_view> == false,
            "std::string_view is not supported as a key type. Use std::string instead.");

        template<typename, typename>
        class IndexMapIterator;
        template<typename, typename> friend class IndexMapIterator;

        static constexpr bool isStringKey = utils::isStdString<KeyT>;

        // Helper types to store std::string in std::unordered_map as std::string_view
        using MapKey = std::conditional_t<isStringKey,
            std::string_view, std::reference_wrapper<const KeyT>
        >;
        using UnderlyingMapKeyT =
            std::conditional_t<isStringKey, std::string_view, KeyT>;

    public:
        using size_type = std::size_t;
        using Idx = size_type;

        using ContainerElement = std::pair<KeyT, T>;
        using ContainerType = std::list<ContainerElement>;

        using key_type = UnderlyingMapKeyT;
        using key_reference = std::conditional_t<isStringKey, key_type, key_type&>;
        using key_const_reference  = const key_reference;
        using key_rvalue_reference = std::conditional_t<isStringKey, key_type, key_type&&>;

        using value_type = T;
        using reference = value_type&;
        using const_reference =  const value_type&;

        using iterator = IndexMapIterator<T, typename ContainerType::iterator>;
        using const_iterator = IndexMapIterator<T, typename ContainerType::const_iterator>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        IndexMap() = default;
        IndexMap(IndexMap&&) = default;
        IndexMap& operator = (IndexMap&&) noexcept = default;

        IndexMap(const IndexMap& rhs) : data_(rhs.data_)
        {
            reconstruct();
        }

        IndexMap& operator = (const IndexMap& rhs)
        {
            if (&rhs != this)
            {
                data_ = rhs.data_;
                reconstruct();
            }
            return *this;
        }

        IndexMap(std::initializer_list<std::pair<key_type, value_type>> ilist)
        {
            for (auto&& [key, value] : ilist) {
                pushBack(std::move(key), std::move(value));
            }
            return *this;
        }

        IndexMap& operator = (std::initializer_list<std::pair<key_type, value_type>> ilist)
        {
            clear();
            for (auto&& [key, value] : ilist) {
                pushBack(std::move(key), std::move(value));
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

        /**
         * Returns the pair of key and associated value at specified index.
         *
         * @param idx - The index of the element.
         * @return std::pair of key and associated value.
         * @throw std::out_of_range if the index is out of range.
         */
        const ContainerElement& at(Idx idx) const
        {
            return *index_.at(idx);
        }

        /**
         * Returns a reference to the value at specified key.
         *
         * @param key - The key of the element.
         * @return A reference to the value.
         * @throw std::out_of_range if the key is not found.
         */
        reference value(key_const_reference key)
        {
            return map_.at(key)->second;
        }

        /**
         * Returns a const reference to the value at specified key.
         *
         * @param key - The key of the element.
         * @return A const reference to the value.
         * @throw std::out_of_range if the key is not found.
         */
        const_reference value(key_const_reference key) const
        {
            return map_.at(key)->second;
        }

        /**
         * Returns a reference to the value at specified index.
         *
         * @param idx - The index of the element.
         * @return A reference to the value.
         * @throw std::out_of_range if the index is out of range.
         */
        reference value(Idx idx)
        {
            return index_.at(idx)->second;
        }

        /**
         * Returns a const reference to the value at specified index.
         *
         * @param idx - The index of the element.
         * @return A const reference to the value.
         * @throw std::out_of_range if the index is out of range.
         */
        const_reference value(Idx idx) const
        {
            return index_.at(idx)->second;
        }

        /**
         * Returns a reference to the value at specified index.
         *
         * @param idx - The index of the element.
         * @return A reference to the value.
         * @throw std::out_of_range if the index is out of range.
         */
        reference operator[](Idx idx)
        {
            return index_.at(idx)->second;
        }

        /**
         * Returns a const reference to the value at specified index.
         *
         * @param idx - The index of the element.
         * @return A const reference to the value.
         * @throw std::out_of_range if the index is out of range.
         */
        const_reference operator[](Idx idx) const
        {
            return index_.at(idx)->second;
        }

        /**
         * Returns a reference to the value that is mapped to a key.
         * If the key is not mapped, a new element is created.
         *
         * @param key - The const reference key to mapped element.
         * @return A reference to the mapped element.
         */
        reference operator[](key_const_reference key)
        {
            auto it = map_.find(key);
            if (it == map_.end()) {
                return *emplaceBack(key).first;
            }
            return it->second->second;
        }

        /**
         * Returns a reference to the value that is mapped to a key.
         * If the key is not mapped, a new element is created.
         *
         * @param key - The r-value key to mapped element.
         * @return A reference to the mapped element.
         */
        template<typename = std::enable_if_t<!isStringKey>>
        reference operator[](key_rvalue_reference key)
        {
            auto it = map_.find(key);
            if (it == map_.end()) {
                return *emplaceBack(std::move(key)).first;
            }
            return it->second->second;
        }

        /**
         * Returns a const reference to the value that is mapped to a key.
         *
         * @param key - The key to mapped element.
         * @return A const reference to the mapped element.
         * @throw std::out_of_range if the key is not mapped.
         */
        const_reference operator[](key_const_reference key) const
        {
            auto it = map_.find(key);
            if (it == map_.end()) {
                data_.at(data_.size()); // should throw std::out_of_range
            }
            return it->second;
        }

        /**
         * Returns iterator to the element at specified key or end() iterator if not found.
         *
         * @param key - The key to mapped element.
         * @return Iterator to the element of found key or end() iterator if not found.
         */
        iterator find(key_const_reference key)
        {
            auto it = map_.find(key);
            if (it == map_.end()) {
                return end();
            }
            return it->second;
        }

        /**
         * Returns const iterator to the element at specified key or end() iterator if not found.
         *
         * @param key - The key to mapped element.
         * @return Const iterator to the element of found key or end() iterator if not found.
         */
        const_iterator find(key_const_reference key) const
        {
            auto it = map_.find(key);
            if (it == map_.end()) {
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

        /**
         * Returns the const reference to the key at the given index.
         * @param idx The index of the element.
         * @return The const reference key at the given index.
         *
         * @throw std::out_of_range if the index is out of range.
         */
        template<typename = std::enable_if_t<!isStringKey>>
        const key_type& key(Idx idx) const
        {
            return index_.at(idx)->first;
        }

        template<typename = std::enable_if_t<isStringKey>>
        key_type key(Idx idx) const
        {
            return index_.at(idx)->first;
        }

        /**
         * Constructs value at place in the container and inserts it at the specified position mapped by key.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param pos   - The position iterator to insert the element. If pos is invalid, the element is inserted at the end.
         * @param key   - The key to mapped element.
         * @param args  - The arguments to construct the value.
         * @return std::pair<reference, bool>
         */
        template< class... Args >
        std::pair<reference, bool> emplace(const_iterator pos, key_type key, Args&&... args)
        {
            return insert(pos, std::move(key), T{ std::forward<Args>(args)... });
        }

        /**
         * Inserts a new element at the specified position.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param pos   - The position iterator to insert the element. If pos is invalid, the element is inserted at the end.
         * @param key   - The key to mapped element.
         * @param value - The value to insert.
         * @return std::pair<iterator, bool>
         */
        std::pair<iterator, bool> insert(const_iterator pos, key_type key, const T& value)
        {
            auto iidx = getItrIdx(pos);
            return insert(iidx, std::move(key), value);
        }

        /**
         * Inserts a new element at the specified position.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param pos   - The position iterator to insert the element. If pos is invalid, the element is inserted at the end.
         * @param key   - The key to mapped element.
         * @param value - The R-value reference to value to insert.
         * @return std::pair<iterator, bool>
         */
        std::pair<iterator, bool> insert(const_iterator pos, key_type key, T&& value)
        {
            auto iidx = getItrIdx(pos);
            return insert(iidx, std::move(key), std::move(value));
        }

        /**
         * Inserts a new element at the specified position.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param pos   - The position to insert the element. If pos is invalid, the element is inserted at the end.
         * @param key   - The key to mapped element.
         * @param value - The value to insert.
         * @return std::pair<iterator, bool>
         */
        std::pair<iterator, bool> insert(Idx pos, key_type key, const T& value)
        {
            auto mapIt = map_.find(key);
            if (mapIt != map_.end()) {
                return { mapIt->second, false };
            }

            if (pos > size()) {
                pos = isEmpty() ? 0 : size();
            }

            auto it = data_.end();
            auto iit = index_.begin() + pos;
            if (iit != index_.end()) {
                it = data_.insert(*iit, { moveOrConstructKey(key), value });
            }
            else
            {
                data_.push_back({ moveOrConstructKey(key), value });
                it = --data_.end();
            }

            map_[crefOrKey(it->first)] = it;
            index_.insert(iit, it);
            return { it, true };
        }

        /**
         * Inserts a new element at the specified position.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param pos   - The position to insert the element. If pos is invalid, the element is inserted at the end.
         * @param key   - The key to mapped element.
         * @param value - The R-value reference to value to insert.
         * @return std::pair<iterator, bool>
         */
        std::pair<iterator, bool> insert(Idx pos, key_type key, T&& value)
        {
            auto mapIt = map_.find(key);
            if (mapIt != map_.end()) {
                return { mapIt->second, false };
            }

            if (pos > size()) {
                pos = isEmpty() ? 0 : size();
            }

            auto it  = data_.end();
            auto iit = index_.begin() + pos;
            if (iit != index_.end()) {
                it = data_.insert(*iit, { moveOrConstructKey(key), std::move(value) });
            }
            else
            {
                data_.push_back({ moveOrConstructKey(key), std::move(value) });
                it = --data_.end();
            }

            map_[crefOrKey(it->first)] = it;
            index_.insert(iit, it);
            return { it, true };
        }

        /**
         * Constructs value at place in the container and inserts it at the front mapped by the key.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param key   - The key to mapped element.
         * @param value - The value to insert.
         * @return std::pair<iterator, bool>
         */
        template< class... Args >
        std::pair<iterator, bool> emplaceFront(key_type key, Args&&... args)
        {
            return pushFront(std::move(key), T{ std::forward<Args>(args)... });
        }

        /**
         * Inserts a new element at the front.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param key   - The key to mapped element.
         * @param value - The value to insert.
         * @return std::pair<iterator, bool>
         */
        std::pair<iterator, bool> pushFront(key_type key, const T& value)
        {
            return insert(0, std::move(key), value);
        }

        /**
         * Inserts a new element at the front.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param key   - The key to mapped element.
         * @param value - The R-value reference to value to insert.
         * @return std::pair<iterator, bool>
         */
        std::pair<iterator, bool> pushFront(key_type key, T&& value)
        {
            return insert(0, std::move(key), std::move(value));
        }

        /**
         * Constructs value at place in the container and inserts it at the end, mapped by key.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param key   - The key to mapped element.
         * @param value - The value to insert.
         * @return std::pair<iterator, bool>
         */
        template< class... Args >
        std::pair<iterator, bool> emplaceBack(key_type key, Args&&... args)
        {
            return pushBack(std::move(key), T{ std::forward<Args>(args)... });
        }

        /**
         * Inserts a new element at the end.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param key   - The key to mapped element.
         * @param value - The value to insert.
         * @return std::pair<iterator, bool>
         */
        std::pair<iterator, bool> pushBack(key_type key, const T& value)
        {
            return insert(size(), std::move(key), value);
        }

        /**
         * Inserts a new element at the end.
         * If the key already exists the element is not inserted. In this case the iterator
         * to the mapped element is returned and the bool is set to false.
         *
         * @param key   - The key to mapped element.
         * @param value - The R-value reference to value to insert.
         * @return std::pair<iterator, bool>
         */
        std::pair<iterator, bool> pushBack(key_type key, T&& value)
        {
            return insert(size(), std::move(key), std::move(value));
        }

        /**
         * Removes the element at pos.
         *
         * @param pos       - The position of the element to remove.
         * @return iterator - An iterator to the element that followed the removed element.
         */
        iterator erase(const_iterator pos)
        {
            return eraseByIdx(getItrIdx(pos));
        }

        /**
         * Removes the element at pos.
         *
         * @param pos - The position of the element to remove.
         */
        void erase(Idx pos)
        {
            eraseByIdx(pos);
        }

        /**
         * Removes the element at key.
         *
         * @param key - The key of the element to remove.
         */
        void erase(key_const_reference key)
        {
            auto mapIt = map_.find(key);
            if (mapIt != map_.end())
            {
                auto it = mapIt->second;
                eraseByIdx(getItrIdx(it));
            }
        }

        /**
         * Removes all elements in the container.
         */
        inline void clear() noexcept
        {
            data_.clear();
            index_.clear();
            map_.clear();
        }

        /**
         * Checks if the element at given key exists.
         * @param key - The key to check.
         * @return True if the element exists, false otherwise.
         */
        inline bool contains(key_const_reference key) const
        {
            return map_.count(key) > 0;
        }

        /**
         * Checks if the container is empty.
         * @return True if the container is empty, false otherwise.
         */
        inline bool isEmpty() const
        {
            return data_.empty();
        }

        /**
         * Returns the number of elements in the container.
         * @return The number of elements in the container.
         */
        inline size_type size() const
        {
            return data_.size();
        }

        /**
         * Returns the underlying container which can be iterated over to access the key-value pairs.
         * @return The underlying container.
         */
        inline const ContainerType& container() const // !< returns list of ContainerElements
        {
            return data_;
        }

        /**
         * Reserves capacity for n elements.
         * @param n - The number of elements to reserve capacity for.
         */
        void reserve(size_type n)
        {
            map_.reserve(n);
        }

        /**
         * Swaps the contents of the container with those of other.
         * @param other - The container to swap contents with.
         */
        void swap(IndexMap& other) noexcept
        {
            data_.swap(other.data_);
            index_.swap(other.index_);
            map_.swap(other.map_);
        }

    private:
        static inline auto crefOrKey(key_const_reference key)
        {
            if constexpr (isStringKey) return key;
            else return std::cref(key);
        }

        static inline auto moveOrConstructKey(key_rvalue_reference key)
        {
            if constexpr (isStringKey) return std::string(key);
            else return std::move(key);
        }

        Idx getItrIdx(typename ContainerType::const_iterator itr) const
        {
            return std::distance(data_.begin(), itr);
        }

        iterator eraseByIdx(Idx idx)
        {
            auto it = end();
            if (idx < index_.size())
            {
                it = index_.at(idx);
                map_.erase(it.it_->first);
                index_.erase(index_.begin() + idx);
                it = data_.erase(it.it_);
            }

            return it;
        }

        void reconstruct()
        {
            map_.clear();
            index_.clear();
            map_.reserve(data_.size());
            for(auto it = data_.begin(); it != data_.end(); it++)
            {
                map_[crefOrKey(it->first)] = it;
                index_.push_back(it);
            }
        }

    private:
        // TODO: When moved to C++20 refactor MapType to directly support std::string_view
        using IndexType = std::deque<typename ContainerType::iterator>;
        using MapType = std::unordered_map<
            MapKey,
            typename ContainerType::iterator,
            Hash,
            KeyEqual
        >;

        ContainerType data_;
        IndexType index_;
        MapType map_;
    };


    template<typename T,typename KeyT, typename Hash, typename KeyEqual>
    template <typename ValueT, typename ContainerIterator>
    class IndexMap<T, KeyT, Hash, KeyEqual>::IndexMapIterator
    {
        using IterTraits = std::iterator_traits<ContainerIterator>;
        ContainerIterator it_;

        operator ContainerIterator() const
        {
            return it_;
        }

        friend class IndexMap<T, KeyT>;
        IndexMapIterator(ContainerIterator it) : it_(it) {}

    public:
        using iterator_category = typename IterTraits::iterator_category;
        using value_type = ValueT;
        using difference_type = typename IterTraits::difference_type;
        using pointer = std::conditional_t<std::is_const_v<typename IterTraits::pointer>, const value_type*, value_type*>;
        using reference = std::conditional_t<std::is_const_v<typename IterTraits::reference>, const value_type, value_type>&;

        template<typename U, typename W>
        IndexMapIterator(IndexMapIterator<U, W> other) : it_(other.it_)
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

        inline IndexMapIterator operator+ (difference_type n) const
        {
            return IndexMapIterator(it_ + n);
        }

        IndexMapIterator& operator++ ()
        {
            ++it_;
            return *this;
        }

        IndexMapIterator operator++ (int)
        {
            return IndexMapIterator(it_++);
        }

        IndexMapIterator& operator+= (difference_type n)
        {
            it_ += n;
            return *this;
        }

        IndexMapIterator operator- (difference_type n) const
        {
            return IndexMapIterator(it_ - n);
        }

        IndexMapIterator& operator-- ()
        {
            --it_;
            return *this;
        }

        IndexMapIterator operator-- (int)
        {
            return IndexMapIterator(it_--);
        }

        IndexMapIterator& operator-= (difference_type n)
        {
            it_ -= n;
            return *this;
        }

        bool operator == (const IndexMapIterator& rhs) const
        {
            return it_ == rhs.it_;
        }

        bool operator != (const IndexMapIterator& rhs) const
        {
            return it_ != rhs.it_;
        }

        bool operator < (const IndexMapIterator& rhs) const
        {
            return it_ < rhs.it_;
        }

        bool operator > (const IndexMapIterator& rhs) const
        {
            return it_ > rhs.it_;
        }

        bool operator <= (const IndexMapIterator& rhs) const
        {
            return it_ <= rhs.it_;
        }

        bool operator >= (const IndexMapIterator& rhs) const
        {
            return it_ >= rhs.it_;
        }
    };
}
#endif // LIBIM_INDEXMAP_H
