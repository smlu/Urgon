#ifndef LIBIM_SOUNDCACHE_H
#define LIBIM_SOUNDCACHE_H
#include <libim/common.h>
#include <libim/log/log.h>
#include <libim/math/math.h>

#include <string.h>
#include <type_traits>

namespace libim::content::audio {
    struct SoundCache
    {
        constexpr static std::size_t kMaxStringLen = 128U;

        SoundCache() = default;
        SoundCache(ByteArray data, std::size_t usedSize) :
            data_(std::move(data)),
            usedSize_(usedSize)
        {}
        SoundCache(ByteArray data) :
            SoundCache(std::move(data), data.size())
        {}

        /**
         * Returns the size of used data in the cache.
         * @return The size of data.
         */
        std::size_t size() const
        {
            return usedSize_;
        }

        /**
         * Returns a span of the data chunk in the cache.
         *
         * @param offset - The offset to the beginning of the data.
         * @param size   - The size of the data.
         * @return A ByteView of the data chunk.
         *
         * @throw std::out_of_range - If the requested data is out of range.
         */
        ByteView getDataView(std::size_t offset, std::size_t size) const
        {
            if (offset + size > usedSize_) {
                throw std::out_of_range("SoundCache::getDataView: Requested data offset is out of range.");
            }
            const byte_t* pBegin = data_.data() + offset;
            return ByteView(pBegin, pBegin + size);
        }

        /**
         * Returns null-terminated string from the cache data.
         * @param offset - The offset to the beginning of the string.
         * @param len    - The length of the string.
         * @return A std::string_view of the string.
         *
         * @throw std::out_of_range - If the requested string is out of range.
         */
        std::string_view getString(std::size_t offset, std::size_t len) const
        {
            if (offset + len > usedSize_) {
                throw std::out_of_range("SoundCache::getString: Requested string offset and size is out of range.");
            }
            const char* pBegin = reinterpret_cast<const char*>(data_.data()) + offset;
            return std::string_view(pBegin, len);
        }

        /**
         * Returns null-terminated string from the cache data.
         * Max string length is 128.
         *
         * @param offset - The offset to the beginning of the string.
         * @return A std::string_view of the string.
         *
         * @throw std::out_of_range - If the requested string is out of range.
         */
        std::string_view getString(std::size_t offset) const
        {
            if (offset >= usedSize_) {
                throw std::out_of_range("SoundCache::getString: Requested string offset is out of range.");
            }
            const char* pBegin = reinterpret_cast<const char*>(data_.data()) + offset;
        #if defined(__STDC_LIB_EXT1__) || defined(_MSC_VER)
            std::size_t sLen = strnlen_s(pBegin, kMaxStringLen);
        #else
            std::size_t sLen = strnlen(pBegin, kMaxStringLen);
        #endif
            if (offset + sLen > usedSize_) {
                sLen = usedSize_ - offset; // TODO: maybe some warning?
            }
            return getString(offset, sLen);
        }

        /**
         * Reads data from the cache data into buffer.
         *
         * @param offset - The offset to the beginning of data to read.
         * @param size   - The size of the data to be read.
         * @return A ByteArray containing the data.
         *
         * @throw std::out_of_range - If the requested data is out of range.
         */
        ByteArray read(std::size_t offset, std::size_t size) const
        {
            ByteArray data(size);
            if (read(offset, data.data(), size)) {
                return data;
            }
            throw std::out_of_range("SoundCache::read: Requested data is out of range.");
        }

        /**
         * Reads data from the cache data into buffer.
         *
         * @param offset - The offset to the beginning of data to read.
         * @param data   - The buffer to store the data.
         * @param size   - The size of the data to be read.
         * @return True if the data was read successfully i.e. offset and size is in range, false otherwise.
         */
        bool read(std::size_t offset, byte_t* data, std::size_t size) const
        {
            if (offset + size > usedSize_) {
                return false;
            }
            std::memcpy(data, data_.data() + offset, size);
            return true;
        }

        /**
         * Reads data from the cache data into std::span buffer.
         *
         * @param offset - The offset to the beginning of data to read.
         * @param data   - The buffer to store the data.
         * @return True if the data was read successfully i.e. offset and size is in range, false otherwise.
         */
        bool read(std::size_t offset, MutableByteView data) const
        {
            return read(offset, data.data(), data.size());
        }

        /**
         * Writes data to the cache data.
         * Function makes sure that the buffer is large enough to hold the requested size.
         * If the buffer is not large enough, it will grow the buffer to fit the requested size.
         *
         * @tparam WriteFunc - The type of the write function: std::size_t(byte_t* data, std::size_t dataSize)
         *                     Function must return number of bytes written.
         *
         * @param size  - The size of the data to be stored.
         * @param write - The write function that will be called to write the data.
         *
         * @return The offset to beginning of written data.
         */
        template<typename WriteFunc>
        [[nodiscard]] std::size_t write(std::size_t size, WriteFunc&& write)
        {
            static_assert(std::is_invocable_r_v<std::size_t, WriteFunc, byte_t*, std::size_t>,
                "write function must be invocable with arguments: std::size_t(byte_t* data, std::size_t dataSize)"
            );

            alignUsedSize(); // align to get 4 bytes aligned offset
            auto offset    = usedSize_;
            auto endOffset = offset + size;
            if (endOffset >= data_.size()) {
                grow(endOffset - data_.size());
            }

            std::size_t nUsed = write(data_.data() + offset, size);
            usedSize_ += libim::min(nUsed, data_.size() - usedSize_); // This is to prevent accidental overflow
            return offset;
        }

        /**
         * Writes data to the cache data.
         * Function makes sure that the buffer is large enough to hold the requested size.
         * If the buffer is not large enough, it will grow the buffer to fit the requested size.
         *
         * @param data - The data to be written.
         * @return The offset to beginning of written data.
         */
        [[nodiscard]] std::size_t write(const ByteView data)
        {
            return write(data.size(), [&data](byte_t* pOut, std::size_t size) {
                std::memcpy(pOut, data.data(), size);
                return size;
            });
        }

        /**
         * Writes string to the cache data.
         * Function makes sure that the buffer is large enough to hold the requested size.
         * If string is not null terminated, it will add null terminator.
         *
         * @param string - The string to be written.
         * @return The offset to beginning of written data.
         */
        [[nodiscard]] std::size_t write(const std::string_view string)
        {
            if (string.empty()) {
                throw std::runtime_error("SoundCache::write: Cannot write empty string.");
            }

            auto requiredSize = string.size();
            if (requiredSize > kMaxStringLen)
            {
                LOG_WARNING("SoundCache::write: String is too long, truncating to %d characters.", kMaxStringLen);
                requiredSize = kMaxStringLen;
            }

            if (string[requiredSize - 1] != '\0'
                && requiredSize < kMaxStringLen) {
                requiredSize += 1; // for null terminator
            }

            return write(requiredSize, [&string](byte_t* pOut, std::size_t size) {
                std::memcpy(pOut, reinterpret_cast<const byte_t*>(string.data()), string.size());
                pOut[size - 1] = '\0'; // ensure null termination
                return size;
            });
        }

        /**
         * Shrinks the data buffer to the size of used data.
         *
         * Note, this will invalidate any pointer laying beyond used data offset.
        */
        void shrink()
        {
            data_.resize(usedSize_);
        }

        /**
         * Resets the used size to 0.
         */
        void reset()
        {
            usedSize_ = 0;
        }

        /**
         * Clears the data buffer and resets the used size to 0.
         */
        void clear()
        {
            data_.clear();
            usedSize_ = 0;
        }

    protected:
        constexpr static std::size_t cacheBlockSize = 0x400000;// 4MB

        void alignUsedSize()
        {
            /* Align used cache size to 4 bytes block */
            if (usedSize_ == 0) {
                usedSize_ = 1;
            }
            for (; (usedSize_ & 3) != 0; ++usedSize_);
        }

        void grow(std::size_t size)
        {
            std::size_t newSize = data_.size();
            do {
                newSize += cacheBlockSize;
            } while (newSize < (data_.size() + size));

            data_.resize(newSize);
        }

       /**
        * Sets the size of used cache data.
        *
        * [FOOTGUN WARNING]
        * Calling this function can corrupt the data if the size
        * eats into data which is still used and the write function is called.
        *
        * @param size - The new size of used data.
        */
        void setUsed(std::size_t size)
        {
            usedSize_ = libim::min(size, data_.size());
        }

       /**
        * Decreases the size of used cache data.
        *
        * [FOOTGUN WARNING]
        * Calling this function can corrupt the data if the size
        * eats into data which is still used and the write function is called.
        *
        * @param size - The size to increase by.
        */
        void decreaseUsed(std::size_t size)
        {
            usedSize_ -= libim::min(size, usedSize_);
        }

    private:
        std::size_t usedSize_ = 0;
        ByteArray data_;
    };
}
#endif // LIBIM_SOUNDCACHE_H
