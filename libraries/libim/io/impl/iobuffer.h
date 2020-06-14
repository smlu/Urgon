#ifndef LIBIM_IOBUFFER_H
#define LIBIM_IOBUFFER_H
#include <array>
#include <cstdint>
#include <libim/common.h>
#include <libim/types/safe_cast.h>

namespace libim {
    template<std::size_t BufferSize>
    struct IOBuffer final : public std::array<byte_t, BufferSize>
    {
        using Base_ = std::array<byte_t, BufferSize>;

        IOBuffer()
        {
            reset();
        }

        std::size_t write(const byte_t* data, std::size_t size)
        {
            std::size_t nWrite = safe_cast<std::size_t>(std::distance(pos_, Base_::end()));
            if(size < nWrite) {
                nWrite = size;
            }
            pos_ = std::copy(data, data + nWrite, pos_);
            return nWrite;
        }

        std::size_t size() const noexcept
        {
            return safe_cast<std::size_t>(
                std::distance(Base_::cbegin(), typename Base_::const_iterator(pos_))
            );
        }

        std::size_t capacity() const noexcept
        {
            return Base_::max_size();
        }

        bool hasData() const
        {
            return pos_ != Base_::begin();
        }

        void reset()
        {
            pos_ = Base_::begin();
        }

    private:
        typename Base_::iterator pos_;
    };
}
#endif //