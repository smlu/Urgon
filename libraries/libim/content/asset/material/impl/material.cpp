#include "../material.h"

using namespace libim;
using namespace libim::content::asset;


inline Bitmap::const_iterator libim::content::asset::CopyMipmapFromBuffer(Mipmap& mipmap, const Bitmap& buffer, uint32_t textureCount, uint32_t width, uint32_t height, const ColorFormat& colorInfo)
{
    auto itBitmapBegin = buffer.begin();
    auto itBitmapEnd   = buffer.end();
    for(uint32_t mmIdx = 0; mmIdx < textureCount; mmIdx++) // Mipmap textures
    {
        /* Calculate texture size according to the mipmap index */
        uint32_t texWidth  = width >> mmIdx;
        uint32_t texHeight = height >> mmIdx;

        Texture tex;
        tex.setWidth(texWidth)
           .setHeight(texHeight)
           .setColorInfo(colorInfo)
           .setRowSize(GetRowSize(texHeight, tex.colorInfo().bpp));

        /* Init texture bitmap buffer */
        uint32_t bitmapSize = GetBitmapSize(texWidth, texHeight, tex.colorInfo().bpp);
        auto bitmap = MakeBitmapPtr(bitmapSize);

        /* Copy texture's bitmap from buffer */
        itBitmapEnd = std::next(itBitmapBegin, bitmapSize);
        std::copy(itBitmapBegin, itBitmapEnd, bitmap->begin());

        tex.setBitmap(std::move(bitmap));
        mipmap.emplace_back(std::move(tex));
        itBitmapBegin = itBitmapEnd;
    }

    return itBitmapEnd;
}

Mipmap libim::content::asset::MoveMipmapFromBuffer(Bitmap& buffer, uint32_t textureCount, uint32_t width, uint32_t height, const ColorFormat& colorInfo)
{
    Mipmap mipmap;
    for(uint32_t mmIdx = 0; mmIdx < textureCount; mmIdx++) // Mipmap's textures
    {
        /* Calculate texture's size according to the mipmap index */
        uint32_t texWidth  = width >> mmIdx;
        uint32_t texHeight = height >> mmIdx;

        Texture tex;
        tex.setWidth(texWidth)
           .setHeight(texHeight)
           .setColorInfo(colorInfo)
           .setRowSize(GetRowSize(texHeight, tex.colorInfo().bpp));

        /* Init texture bitmap buffer */
        uint32_t bitmapSize = GetBitmapSize(texWidth, texHeight, tex.colorInfo().bpp);
        auto bitmap = std::make_shared<Bitmap>(bitmapSize);

        /* Copy texture's bitmap from buffer */
        auto itBitmapEnd = std::next(buffer.begin(), bitmapSize);
        std::copy(buffer.begin(), itBitmapEnd, bitmap->begin());

        buffer.erase(buffer.begin(), itBitmapEnd);
        tex.setBitmap(std::move(bitmap));
        mipmap.emplace_back(std::move(tex));
    }

    return mipmap;
}

template<> Mipmap Stream::read<Mipmap, uint32_t, uint32_t, uint32_t, const ColorFormat&>(uint32_t textureCount, uint32_t width, uint32_t height, const ColorFormat& colorInfo) const
{
    Mipmap mipmap;
    for(uint32_t mmIdx = 0; mmIdx < textureCount; mmIdx++) // Mipmap's textures
    {
        /* Calculate texture size according to the mipmap index */
        uint32_t texWidth  = width >> mmIdx;
        uint32_t texHeight = height >> mmIdx;

        /* Read Texture */
        auto tex = this->read<Texture, uint32_t, uint32_t, const ColorFormat&>(texWidth, texHeight, colorInfo);
        mipmap.emplace_back(std::move(tex));
    }

    return mipmap;
}
