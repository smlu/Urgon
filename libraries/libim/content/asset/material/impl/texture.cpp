#include "../texture.h"
#include <stdexcept>

using namespace libim;
using namespace libim::content::asset;


Texture::Texture(uint32_t width, uint32_t height, ColorFormat format, BitmapPtr ptrBitmap) :
    width_(width),
    height_(height),
    ci_(std::move(format)),
    bitmap_(std::move(ptrBitmap))
{
    rowSize_ = getRowSize(width_, ci_.bpp);
    if(rowSize_ * height_ > bitmap_->size()) { //sanity check
        throw std::runtime_error("Invalid Texture size and color format");
    }
}

Texture::Texture(const Texture& rhs) :
    width_(rhs.width_),
    height_(rhs.height_),
    rowSize_(rhs.rowSize_),
    ci_(rhs.ci_),
    bitmap_(rhs.bitmap_)
{}

Texture::Texture(Texture&& rrhs) noexcept:
    width_(rrhs.width_),
    height_(rrhs.height_),
    rowSize_(rrhs.rowSize_),
    ci_(std::move(rrhs.ci_)),
    bitmap_(std::move(rrhs.bitmap_))
{
    rrhs.width_    = 0;
    rrhs.height_   = 0;
    rrhs.rowSize_  = 0;
}

Texture& Texture::operator = (const Texture& rhs)
{
    if(&rhs != this)
    {
        width_   = rhs.width_;
        height_  = rhs.height_;
        rowSize_ = rhs.rowSize_;
        ci_      = rhs.ci_;
        bitmap_  = rhs.bitmap_;
    }

    return *this;
}

Texture& Texture::operator = (Texture&& rrhs) noexcept
{
    if(&rrhs != this)
    {
        width_   = rrhs.width_;
        height_  = rrhs.height_;
        rowSize_ = rrhs.rowSize_;
        ci_      = std::move(rrhs.ci_);
        bitmap_  = std::move(rrhs.bitmap_);

        rrhs.width_    = 0;
        rrhs.height_   = 0;
        rrhs.rowSize_  = 0;
    }

    return *this;
}

Bmp Texture::toBmp() const
{
    uint32_t bitdataSize = getBitmapSize(width(), height(), ci_.bpp);

    Bmp bmp;
    bmp.header.type      = BMP_TYPE;
    bmp.header.offBits   = sizeof(BitmapFileHeader) + sizeof(BitmapV5Header);
    bmp.header.size      = bmp.header.offBits + bitdataSize;

    bmp.info.size        = sizeof(BitmapV5Header);
    bmp.info.width       = width();
    bmp.info.height      = - static_cast<int32_t>(height()); // flip image
    bmp.info.planes      = 1;
    bmp.info.bitCount    = ci_.bpp;
    bmp.info.compression = BI_BITFIELDS; //ci_.colorMode ? BI_BITFIELDS : BI_ALPHABITFIELDS;
    bmp.info.sizeImage   = bitdataSize;
    bmp.info.redMask     = rgbMask(ci_.redBPP  , ci_.redShl);
    bmp.info.greenMask   = rgbMask(ci_.greenBPP, ci_.greenShl);
    bmp.info.blueMask    = rgbMask(ci_.blueBPP , ci_.blueShl);
    bmp.info.alphaMask   = rgbMask(ci_.alphaBPP, ci_.alphaShl);

    bmp.pixelData        = bitmap_;
    return bmp;
}

template<> Texture Stream::read<Texture, uint32_t, uint32_t, const ColorFormat&>(uint32_t width, uint32_t height, const ColorFormat& colorInfo) const
{
    /* Read bitmap data from stream */
    const uint32_t bitmapSize = getBitmapSize(width, height, colorInfo.bpp);
    auto bitmap = read<BitmapPtr>(bitmapSize);
    return Texture(width, height, colorInfo, std::move(bitmap));
}
