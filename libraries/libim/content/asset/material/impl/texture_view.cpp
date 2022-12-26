#include "../texture_view.h"
#include "../texutils.h"
#include "../colorformat.h"
#include <libim/math/math.h>

#include <stdexcept>

using namespace libim;
using namespace libim::content::asset;

Texture TextureView::converted(const ColorFormat& format) const
{
    if (format == *cf_) {
        return Texture(*this);
    }

    auto width = width_, height = height_;
    if (mipLevels_ > 1) { // represent pixdata as 1 big row
        width = safe_cast<uint32_t>(std::distance(itFirst_, itLast_)) / bbs(cf_->bpp); height = 1;
    }

    PixdataPtr ptrPixdata = convertPixdata(itFirst_, itLast_, width, height, *cf_, format);
    return Texture(width_, height_, mipLevels_, format, std::move(ptrPixdata));
}

TextureView TextureView::mipmap(uint32_t lod, std::optional<uint32_t> mipLevels) const
{
    if (lod >= mipLevels_) {
        throw std::out_of_range("lod out of range in TextureView");
    }

    const auto numLevels = min(mipLevels.value_or(mipLevels_ - lod), mipLevels_ - lod);
    auto [mipWidth, mipHeight, itMipBegin, itMipEnd] =
        getMipmapsAtLevel(lod, itFirst_, width_, height_, numLevels, *cf_);

    return TextureView(
        itMipBegin,
        itMipEnd,
        mipWidth,
        mipHeight,
        calcStride(mipWidth, *cf_),
        numLevels,
        *cf_
    );
}