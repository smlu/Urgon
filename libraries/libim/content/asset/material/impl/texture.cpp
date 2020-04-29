#include "../texture.h"

using namespace libim;
using namespace libim::content::asset;


Texture::Texture(const Texture& rhs) :
    m_width(rhs.m_width),
    m_height(rhs.m_height),
    m_rowSize(rhs.m_rowSize),
    /*m_rowWidth(rhs.m_rowWidth),*/
    m_colorInfo(rhs.m_colorInfo),
    m_bitmap(rhs.m_bitmap)
{}

Texture::Texture(Texture&& rrhs) noexcept:
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

Texture& Texture::operator = (const Texture& rhs)
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

Texture& Texture::operator = (Texture&& rrhs) noexcept
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

Bmp Texture::toBmp() const
{
    uint32_t matBitdataSize = getBitmapSize(width(), height(), m_colorInfo.bpp);

    Bmp bmp;
    bmp.header.type    = BMP_TYPE;
    bmp.header.offBits = sizeof(BitmapFileHeader) + sizeof(BitmapV5Header);
    bmp.header.size    = bmp.header.offBits + matBitdataSize;

    bmp.info.size        = sizeof(BitmapV5Header);
    bmp.info.width       = width();
    bmp.info.height      = - static_cast<int32_t>(height()); // flip image
    bmp.info.planes      = 1;
    bmp.info.bitCount    = m_colorInfo.bpp;
    bmp.info.compression = BI_BITFIELDS; //m_colorInfo.colorMode ? BI_BITFIELDS : BI_ALPHABITFIELDS;
    bmp.info.sizeImage   = matBitdataSize;
    bmp.info.redMask     = rgbMask(m_colorInfo.redBPP  , m_colorInfo.redShl);
    bmp.info.greenMask   = rgbMask(m_colorInfo.greenBPP, m_colorInfo.greenShl);
    bmp.info.blueMask    = rgbMask(m_colorInfo.blueBPP , m_colorInfo.blueShl);
    bmp.info.alphaMask   = rgbMask(m_colorInfo.alphaBPP, m_colorInfo.alphaShl);

    bmp.pixelData = m_bitmap;
    return bmp;
}

template<> Texture Stream::read<Texture, uint32_t, uint32_t, const ColorFormat&>(uint32_t width, uint32_t height, const ColorFormat& colorInfo) const
{
    Texture tex;
    tex.setWidth(width)
       .setHeight(height)
       .setColorInfo(colorInfo)
       .setRowSize(GetRowSize(height, tex.colorInfo().bpp));

    /* Read bitmap data from stream */
    uint32_t bitmapSize = getBitmapSize(tex.width(), tex.height(), tex.colorInfo().bpp);
    tex.setBitmap(read<BitmapPtr>(bitmapSize));

    return tex;
}
