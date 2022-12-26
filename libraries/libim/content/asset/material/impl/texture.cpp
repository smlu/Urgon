#include "../texture.h"
#include "../texture_view.h"
#include "../texutils.h"

#include <libim/math/math.h>
#include <libim/types/safe_cast.h>

#include <algorithm>
#include <stdexcept>

using namespace libim;
using namespace libim::content::asset;


Texture::Texture(uint32_t width, uint32_t height, uint32_t mipLevels, ColorFormat format, PixdataPtr ptrPixdata):
    width_(width),
    height_(height),
    mipLevels_(mipLevels),
    cf_(std::move(format)),
    ptrPixdata_(std::move(ptrPixdata))
{
    if (width == 0 || height == 0) {
        throw TextureError("Invalid texture size");
    }

    stride_ = calcStride(width_, cf_);
    if (calcMipmapSize(width, height, mipLevels, cf_) != ptrPixdata_->size()) { //sanity check
        throw TextureError("Invalid pixdata size for given texture info");
    }
}

Texture::Texture(const TextureView& tv)
{
    auto ptrPixdata = makePixdataPtr(tv.itFirst_, tv.itLast_);
    auto tex = Texture(tv.width_, tv.height_, tv.mipLevels_, *tv.cf_, std::move(ptrPixdata));
    this->operator=(std::move(tex));
}

Texture::Texture(const Texture& rhs) :
    width_(rhs.width_),
    height_(rhs.height_),
    stride_(rhs.stride_),
    mipLevels_(rhs.mipLevels_),
    cf_(rhs.cf_),
    ptrPixdata_(rhs.ptrPixdata_)
{}

Texture::Texture(Texture&& rrhs) noexcept:
    width_(rrhs.width_),
    height_(rrhs.height_),
    stride_(rrhs.stride_),
    mipLevels_(rrhs.mipLevels_),
    cf_(std::move(rrhs.cf_)),
    ptrPixdata_(std::move(rrhs.ptrPixdata_))
{
    rrhs.width_     = 0;
    rrhs.height_    = 0;
    rrhs.stride_    = 0;
    rrhs.mipLevels_ = 0;
}

Texture& Texture::operator = (const TextureView& tv)
{
    if (tv.itFirst_ != ptrPixdata_->begin() || tv.itLast_ != ptrPixdata_->end())
    {
        auto ptrPixdata = makePixdataPtr(tv.itFirst_, tv.itLast_);
        auto tex = Texture(tv.width_, tv.height_, tv.mipLevels_, *tv.cf_, std::move(ptrPixdata));
        this->operator=(std::move(tex));
    }
    return *this;
}

Texture& Texture::operator = (const Texture& rhs)
{
    if (&rhs != this)
    {
        width_       = rhs.width_;
        height_      = rhs.height_;
        stride_      = rhs.stride_;
        mipLevels_   = rhs.mipLevels_;
        cf_          = rhs.cf_;
        ptrPixdata_  = rhs.ptrPixdata_;
    }

    return *this;
}

Texture& Texture::operator = (Texture&& rrhs) noexcept
{
    if(&rrhs != this)
    {
        width_       = rrhs.width_;
        height_      = rrhs.height_;
        stride_      = rrhs.stride_;
        mipLevels_   = rrhs.mipLevels_;
        cf_          = std::move(rrhs.cf_);
        ptrPixdata_  = std::move(rrhs.ptrPixdata_);

        rrhs.width_     = 0;
        rrhs.height_    = 0;
        rrhs.stride_    = 0;
        rrhs.mipLevels_ = 0;
    }

    return *this;
}

TextureView Texture::mipmap(uint32_t lod, std::optional<uint32_t> mipLevels) const
{
    return TextureView(*this).mipmap(lod, mipLevels);
}

Texture Texture::clone() const
{
    PixdataPtr ptrPixdata = makePixdataPtr(ptrPixdata_->begin(), ptrPixdata_->end());
    return Texture(width_, height_, mipLevels_, cf_, std::move(ptrPixdata));
}

Texture& Texture::convert(const ColorFormat& format)
{
    if (format != cf_)
    {
        auto width = width_, height = height_;
        if(mipLevels_ > 1) { // represent pixdata as 1 big row
            width = safe_cast<uint32_t>(ptrPixdata_->size()) / bbs(cf_.bpp); height = 1;
        }

        ptrPixdata_ = convertPixdata(ptrPixdata_, width, height, cf_, format);
        cf_         = format;
        stride_     = calcStride(width_, cf_);
    }
    return *this;
}

Texture Texture::converted(const ColorFormat& format) const
{
    auto tex = clone();
    return tex.convert(format);
}

uint32_t Texture::maxMipLevels() const {
    return calcMaxMipmapLevels(width_, height_);
}

template<> Texture Stream::read<Texture, uint32_t, uint32_t, uint32_t, const ColorFormat&>
    (uint32_t width, uint32_t height, uint32_t mipLevels, const ColorFormat& format) const
{
    /* Read pixdata data from stream */
    const uint32_t pixdataSize = calcMipmapSize(width, height, mipLevels, format);
    auto pixdata = read<PixdataPtr>(pixdataSize);
    return Texture(width, height, mipLevels, format, std::move(pixdata));
}

void makeMipmapsFromPixdata(Pixdata::const_iterator itSrc, uint32_t srcWidth, uint32_t srcHeight, Pixdata::iterator itDest, uint32_t mipLevels, const ColorFormat& cf, bool sRGB)
{
    uint32_t destWidth = srcWidth >> 1; uint32_t destHeight = srcHeight >> 1;
    while (mipLevels --> 1 && srcWidth && srcHeight && destWidth && destHeight)
    {
        boxFilterScale(itSrc, srcWidth, srcHeight, itDest, destWidth, destHeight, cf, sRGB);

        std::advance(itSrc, calcPixdataSize(srcWidth, srcHeight, cf));
        srcWidth  >>= 1;
        srcHeight >>= 1;

        std::advance(itDest, calcPixdataSize(destWidth, destHeight, cf));
        destWidth  >>= 1;
        destHeight >>= 1;
    }
}

Texture& Texture::scale(uint32_t width, uint32_t height, bool sRGB)
{
    if (width_ != width || height_ != height) {
        *this = scaled(width, height, sRGB);
    }
    return *this;
}

Texture Texture::scaled(uint32_t width, uint32_t height, bool sRGB) const
{
    auto tex = Texture(mipmap(/*lod=*/0, /*mipLevels=*/1));
    if (tex.format().bpp < RGB24.bpp) { // Image is better scaled at 8 bits per channel
        tex = tex.converted(RGBA32);
    }

    auto ptrPixdata = makePixdataPtr(calcMipmapSize(width, height, mipLevels_, tex.format()));
    boxFilterScale(tex.ptrPixdata_->begin(), width_, height_, ptrPixdata->begin(), width, height, tex.format(), sRGB);
    makeMipmapsFromPixdata(ptrPixdata->begin(),
        width, width,
        std::next(ptrPixdata->begin(), calcPixdataSize(width, height, tex.format())),
        mipLevels_,
        tex.format(),
        sRGB
    );

    tex.width_      = width;
    tex.height_     = height;
    tex.mipLevels_  = mipLevels_;
    tex.ptrPixdata_ = ptrPixdata;
    if (cf_ != tex.format()) {
        tex = tex.converted(cf_);
    }
    return tex;
}

Texture& Texture::generateMipmaps(std::optional<uint32_t> optMipLevels, std::optional<ColorFormat> optFormat, bool sRGB)
{
    *this = makeMipmap(optMipLevels, optFormat, sRGB);
    return *this;
}

Texture Texture::makeMipmap(std::optional<uint32_t> levels, std::optional<ColorFormat> optFormat, bool sRGB) const
{
    auto tex             = Texture(mipmap(/*lod=*/0, /*mipLevels=*/1));
    const auto maxLevels = maxMipLevels();
    auto mipLevels       = clamp<uint32_t>(levels.value_or(maxLevels), 1, maxLevels);
    if (mipLevels > 1)
    {
        if (tex.format().bpp < RGB24.bpp) { // Image is better downsampled at 8 bits per channel
            tex.convert(RGBA32);
        }

        auto ptrPixdata = makePixdataPtr(calcMipmapSize(width_, height_, mipLevels, tex.format()));
        auto itSrc      = ptrPixdata->begin();
        auto itDest     = std::copy(tex.ptrPixdata_->begin(), tex.ptrPixdata_->end(), itSrc);
        makeMipmapsFromPixdata(itSrc, width_, height_, itDest, mipLevels, tex.format(), sRGB);

        tex.ptrPixdata_ = ptrPixdata;
        tex.mipLevels_  = mipLevels;
    }

    const auto cfd  = optFormat.value_or(cf_);
    if (cfd != tex.format()) {
        tex.convert(cfd);
    }
    return tex;
}

Texture& Texture::clearMipmaps()
{
    if (mipLevels_ > 1)
    {
        ptrPixdata_->resize(calcPixdataSize(width_, height_, cf_));
        ptrPixdata_->shrink_to_fit(); // forcefully invalidate iterators, also TextViews based on this texture will be invalidated
        mipLevels_ = 1;
    }
    return *this;
}