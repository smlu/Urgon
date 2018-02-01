#ifndef TEXTURE_H
#define TEXTURE_H
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "bmp.h"
#include "colorformat.h"
#include "common.h"
#include "io/stream.h"

class Texture
{
public:
    Texture() = default;
    Texture(const Texture& rhs) :
        m_width(rhs.m_width),
        m_height(rhs.m_height),
        m_rowSize(rhs.m_rowSize),
        /*m_rowWidth(rhs.m_rowWidth),*/
        m_colorInfo(rhs.m_colorInfo),
        m_bitmap(rhs.m_bitmap)
    {}

    Texture(Texture&& rrhs) noexcept:
        m_width(rrhs.m_width),
        m_height(rrhs.m_height),
        m_rowSize(rrhs.m_rowSize),
        /*m_rowWidth(rrhs.m_rowWidth),*/
        m_colorInfo(std::move(rrhs.m_colorInfo)),
        m_bitmap(std::move(rrhs.m_bitmap))
    {
        rrhs.m_width    = 0;
        rrhs.m_height   = 0;
        rrhs.m_rowSize  = 0;
        //rrhs.m_rowWidth = 0;
    }

    Texture& operator = (const Texture& rhs)
    {
        if(&rhs != this)
        {
            m_width = rhs.m_width;
            m_height =rhs.m_height;
            m_rowSize   = rhs.m_rowSize;
            //m_rowWidth  = rhs.m_rowWidth;
            m_colorInfo = rhs.m_colorInfo;
            m_bitmap = rhs.m_bitmap;
        }

        return *this;
    }

    Texture& operator = (Texture&& rrhs) noexcept
    {
        if(&rrhs != this)
        {
            m_width  = rrhs.m_width;
            m_height = rrhs.m_height;
            m_rowSize   = rrhs.m_rowSize;
            //m_rowWidth  = rrhs.m_rowWidth;
            m_colorInfo = std::move(rrhs.m_colorInfo);
            m_bitmap = std::move(rrhs.m_bitmap);

            rrhs.m_width    = 0;
            rrhs.m_height   = 0;
            rrhs.m_rowSize  = 0;
            //rrhs.m_rowWidth = 0;
        }

        return *this;
    }

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

    Bmp toBmp() const
    {
        uint32_t matBitdataSize = GetBitmapSize(width(), height(), m_colorInfo.bpp);

        Bmp bmp;
        bmp.header.type    = BMP_TYPE;
        bmp.header.offBits = sizeof(BitmapFileHeader) + sizeof(BitmapV5Header);
        bmp.header.size    = bmp.header.offBits + matBitdataSize;

        bmp.info.size        = sizeof(BitmapV5Header);
        bmp.info.width       = width();
        bmp.info.height      = - (height()); // flip image
        bmp.info.planes      = 1;
        bmp.info.bitCount    = m_colorInfo.bpp;
        bmp.info.compression = BI_BITFIELDS; //m_colorInfo.colorMode ? BI_BITFIELDS : BI_ALPHABITFIELDS;
        bmp.info.sizeImage   = matBitdataSize;
        bmp.info.redMask     = RGBMask(m_colorInfo.redBPP  , m_colorInfo.RedShl);
        bmp.info.greenMask   = RGBMask(m_colorInfo.greenBPP, m_colorInfo.GreenShl);
        bmp.info.blueMask    = RGBMask(m_colorInfo.blueBPP , m_colorInfo.BlueShl);
        bmp.info.alphaMask   = RGBMask(m_colorInfo.alphaBPP, m_colorInfo.AlphaShl);

        bmp.pixelData = m_bitmap;
        return bmp;
    }

private:
    uint32_t m_width    = 0;
    uint32_t m_height   = 0;
    uint32_t m_rowSize  = 0;
    //uint32_t m_rowWidth = 0; // in jones engine row width is defined as rowSize / Bitdepth in bytes
    ColorFormat m_colorInfo;
    BitmapPtr m_bitmap;
};


/* Stream template specialization */
template<> inline Texture Stream::read<Texture, uint32_t, uint32_t, const ColorFormat&>(uint32_t width, uint32_t height, const ColorFormat& colorInfo) const
{
    Texture tex;
    tex.setWidth(width)
       .setHeight(height)
       .setColorInfo(colorInfo)
       .setRowSize(GetRowSize(height, tex.colorInfo().bpp));

    /* Read bitmap data from stream */
    uint32_t bitmapSize = GetBitmapSize(tex.width(), tex.height(), tex.colorInfo().bpp);
    tex.setBitmap(read<BitmapPtr>(bitmapSize));

    return tex;
}

#endif // TEXTURE_H
