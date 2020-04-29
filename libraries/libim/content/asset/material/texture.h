#ifndef LIBIM_TEXTURE_H
#define LIBIM_TEXTURE_H
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "bmp.h"
#include "colorformat.h"
#include "../../../common.h"
#include "../../../io/stream.h"

namespace libim::content::asset{
    class Texture
    {
    public:
        Texture() = default;
        Texture(const Texture& rhs);
        Texture(Texture&& rrhs) noexcept;

        Texture& operator = (const Texture& rhs);
        Texture& operator = (Texture&& rrhs) noexcept;

        virtual ~Texture() = default;

        Texture& setWidth(uint32_t width)
        {
            m_width = width;
            return *this;
        }

        uint32_t width() const
        {
            return m_width;
        }

        Texture& setHeight(uint32_t height)
        {
            m_height = height;
            return *this;
        }

        uint32_t height() const
        {
            return m_height;
        }

        Texture& setRowSize(uint32_t rowSize)
        {
            m_rowSize = rowSize;
            return *this;
        }

        uint32_t rowSize() const
        {
            return m_rowSize;
        }

        Texture& setColorInfo(const ColorFormat& colorInfo)
        {
            m_colorInfo = colorInfo;
            return *this;
        }

        const ColorFormat& colorInfo() const
        {
            return m_colorInfo;
        }

        Texture& setBitmap(const BitmapPtr& bitmap)
        {
            m_bitmap = bitmap;
            return *this;
        }

        Texture& setBitmap(BitmapPtr&& bitmap)
        {
            m_bitmap = std::move(bitmap);
            return *this;
        }

        BitmapPtr bitmap() const
        {
            return m_bitmap;
        }

        Bmp toBmp() const;

    private:
        uint32_t m_width    = 0;
        uint32_t m_height   = 0;
        uint32_t m_rowSize  = 0;
        //uint32_t m_rowWidth = 0; // in jones engine row width is defined as rowSize / Bitdepth in bytes
        ColorFormat m_colorInfo;
        BitmapPtr m_bitmap;
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
