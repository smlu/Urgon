#ifndef LIBIM_MATERIAL_H
#define LIBIM_MATERIAL_H
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "texture.h"
#include "../asset.h"
#include <libim/common.h>
#include <libim/io/stream.h>

namespace libim::content::asset {
    struct Mipmap : public std::vector<Texture>{};

    class Material final : public Asset
    {
    public:
        using Asset::Asset;

        Material(const InputStream& istream)
        {
            deserialize(istream);
        }

        Material(InputStream&& istream)
        {
            deserialize(std::move(istream));
        }

        Material(const Material&) = default;
        Material(Material&&) noexcept = default;
        Material& operator = (const Material&) = default;
        Material& operator = (Material&&) noexcept = default;
        virtual ~Material() override = default;


        Material& deserialize(const InputStream& istream);
        Material& deserialize(InputStream&& istream);
        bool serialize(OutputStream&& ostream) const;
        bool serialize(OutputStream& ostream) const;

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
        uint32_t m_width;
        uint32_t m_height;
        ColorFormat m_colorFormat;
        std::vector<Mipmap> m_mipmaps;
    };


    Bitmap::const_iterator
    copyMipmapFromBuffer(
        Mipmap& mipmap,
        const Bitmap& buffer,
        uint32_t textureCount,
        uint32_t width,
        uint32_t height,
        const ColorFormat& colorInfo
    );

    Mipmap moveMipmapFromBuffer(
        Bitmap& buffer,
        uint32_t textureCount,
        uint32_t width,
        uint32_t height,
        const ColorFormat& colorInfo
    );
}

namespace libim {
    template<> content::asset::Mipmap
    Stream::read<
        content::asset::Mipmap,
        uint32_t,
        uint32_t,
        uint32_t,
        const content::asset::ColorFormat&>
    (
        uint32_t textureCount,
        uint32_t width,
        uint32_t height,
        const content::asset::ColorFormat& colorInfo
    ) const;
}

#endif // LIBIM_MATERIAL_H
