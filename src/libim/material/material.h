#ifndef LIBIM_MATERIAL_H
#define LIBIM_MATERIAL_H
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "texture.h"
#include "common.h"
#include "../io/stream.h"


struct Mipmap : public std::vector<Texture>{};

class Material
{
public:
    Material() = default;
    Material(const std::string& name) : m_name(name){}
    Material(const Material&) = default;
    Material(Material&&) noexcept = default;
    Material& operator = (const Material&) = default;
    Material& operator = (Material&&) noexcept = default;

    Material& setName(const std::string& name)
    {
        m_name = name;
        return *this;
    }

    const std::string& name() const
    {
        return m_name;
    }

    Material& setWidth(uint32_t width)
    {
        m_width = width;
        return *this;
    }

    uint32_t width() const
    {
        return m_width;
    }

    Material& setHeight(uint32_t height)
    {
        m_height = height;
        return *this;
    }

    uint32_t height() const
    {
        return m_height;
    }

    Material& setSize(uint32_t width, uint32_t height)
    {
        m_width = width;
        m_height = height;
        return *this;
    }

    Material& setColorFormat(const ColorFormat& colorFormat)
    {
        m_colorFormat = colorFormat;
        return *this;
    }

    const ColorFormat& colorFormat() const
    {
        return m_colorFormat;
    }

    Material& setMipmaps(const std::vector<Mipmap>& mipmaps)
    {
        m_mipmaps = mipmaps;
        return *this;
    }

    Material& setMipmaps(std::vector<Mipmap> && mipmaps) noexcept
    {
        m_mipmaps = std::move(mipmaps);
        return *this;
    }

    Material& addMipmap(const Mipmap& mipmap)
    {
        m_mipmaps.push_back(mipmap);
        return *this;
    }

    Material& addMipmap(Mipmap&& mipmap)
    {
        m_mipmaps.emplace_back(std::move(mipmap));
        return *this;
    }

    const std::vector<Mipmap>& mipmaps() const
    {
        return m_mipmaps;
    }

private:
    std::string m_name;
    uint32_t m_width;
    uint32_t m_height;
    ColorFormat m_colorFormat;
    std::vector<Mipmap> m_mipmaps;
};


inline Bitmap::const_iterator CopyMipmapFromBuffer(Mipmap& mipmap, const Bitmap& buffer, uint32_t textureCount, uint32_t width, uint32_t height, const ColorFormat& colorInfo)
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

inline Mipmap MoveMipmapFromBuffer(Bitmap& buffer, uint32_t textureCount, uint32_t width, uint32_t height, const ColorFormat& colorInfo)
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

template<> inline Mipmap Stream::read<Mipmap, uint32_t, uint32_t, uint32_t, const ColorFormat&>(uint32_t textureCount, uint32_t width, uint32_t height, const ColorFormat& colorInfo) const
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

#endif // LIBIM_MATERIAL_H
