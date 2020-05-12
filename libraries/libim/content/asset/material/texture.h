#ifndef LIBIM_TEXTURE_H
#define LIBIM_TEXTURE_H
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "bmp.h"
#include "colorformat.h"
#include <libim/common.h>
#include <libim/io/stream.h>

namespace libim::content::asset{
    class Texture
    {
    public:
        Texture() = default;
        Texture(uint32_t width, uint32_t height, ColorFormat format, BitmapPtr ptrBitmap); // !< Throws std::runtime_error if invalid size and format for given ptrBitmap arg
        Texture(const Texture& rhs);
        Texture(Texture&& rrhs) noexcept;

        Texture& operator = (const Texture& rhs);
        Texture& operator = (Texture&& rrhs) noexcept;

        virtual ~Texture() = default;

        uint32_t width() const
        {
            return width_;
        }

        uint32_t height() const
        {
            return height_;
        }

        uint32_t rowSize() const
        {
            return rowSize_;
        }

        const ColorFormat& colorInfo() const
        {
            return ci_;
        }

        BitmapPtr bitmap() const
        {
            return bitmap_;
        }

        Bmp toBmp() const;

    private:
        uint32_t width_    = 0;
        uint32_t height_   = 0;
        uint32_t rowSize_  = 0;
        //uint32_t m_rowWidth = 0; // in jones engine row width is defined as rowSize / Bitdepth in bytes
        ColorFormat ci_;
        BitmapPtr bitmap_;
    };
}

namespace libim {
    /* Stream template specialization */
    template<>
    content::asset::Texture
    Stream::read<
        content::asset::Texture,
        uint32_t,
        uint32_t,
        const content::asset::ColorFormat&
    >
    (uint32_t width, uint32_t height, const content::asset::ColorFormat& colorInfo) const;
}

#endif // LIBIM_TEXTURE_H
