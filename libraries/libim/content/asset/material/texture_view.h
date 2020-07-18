#ifndef LIBIM_TEXTURE_VIEW_H
#define LIBIM_TEXTURE_VIEW_H
#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>

#include "colorformat.h"
#include "texture.h"
#include "texutils.h"

#include <libim/common.h>
#include <libim/io/stream.h>
#include <libim/math/math.h>

namespace libim::content::asset{
    class TextureView final
    {
        friend class Texture;
        TextureView(Pixdata::const_iterator itFirst, Pixdata::const_iterator itLast, uint32_t width, uint32_t height, uint32_t stride, uint32_t mipLevels, const ColorFormat& cf) :
            width_(width),
            height_(height),
            stride_(stride),
            mipLevels_(mipLevels),
            cf_(&cf),
            itFirst_(itFirst),
            itLast_(itLast)
        {}

    public:
        TextureView(const Texture& tex):
            TextureView(
                tex.pixdata()->cbegin(),
                tex.pixdata()->cend(),
                tex.width(),
                tex.height(),
                tex.stride(),
                tex.mipLevels(),
                tex.format()
            )
        {}

        TextureView(Texture&&) noexcept = delete;
        TextureView(const TextureView&) noexcept = default;
        TextureView& operator=(const TextureView&) noexcept = default;
        TextureView& operator=(Texture&&) noexcept = delete;

        /** Returns top texture width */
        uint32_t width() const
        {
            return width_;
        }

        /** Returns top texture height */
        uint32_t height() const
        {
            return height_;
        }

        /** Returns pixeldata row stride size */
        uint32_t stride() const
        {
            return stride_;
        }

        /** Returns number of mipmap levels stored in texture view. */
        uint32_t mipLevels() const
        {
            return mipLevels_;
        }

        const ColorFormat& format() const
        {
            return *cf_;
        }

        bool isEmpty() const
        {
            return begin() == end();
        }

        /**
         * Returns size in bytes of Pixdata of top mipmap texture at LOD idx 0.
         * To get size of pixel data for any other LOD mipmap call function mipmap.
         */
        std::size_t size() const
        {
            return calcPixdataSize(width_, height_, *cf_);
        }

        /** Returns size of whole mipmap. e.g.: size of all LOD pixel data. */
        std::size_t mipSize() const
        {
            return safe_cast<std::size_t>(
                std::distance(itFirst_, itLast_)
            );
        }

        /** Returns true if this Texture contains full mipmap chain. */
        bool isFullMipmapChain() const
        {
            auto maxLevels = 1 +
                static_cast<uint32_t>(std::floor(std::log2(max(height_, width_))));
            return maxLevels == mipLevels_;
        }

        /**
         * Returns Pixdata start iterator of top mipmap texture at LOD idx 0.
         * To get Pixdata start iterator for any other LOD mipmap call function mipmap.
         */
        Pixdata::const_iterator begin() const
        {
            return itFirst_;
        }

        /**
         * Returns Pixdata end iterator of top mipmap texture at LOD idx 0.
         * To get Pixdata end iterator for any other LOD mipmap call function mipmap.
         */
        Pixdata::const_iterator end() const
        {
            return std::next(itFirst_, size());
        }

        /** Returns Pixdata iterator to the end of last mipmap texture pixel data. */
        Pixdata::const_iterator mipEnd() const
        {
            return itLast_;
        }

        /**
         * Returns deeply copied new PixdataPtr for pixel data at LOD 0.
         * To get PixdataPtr of any other LOD mipmap call function mipmap.
         * @return new PixdataPtr of top mipmap pixel data
         */
        PixdataPtr pixdata() const
        {
            return makePixdataPtr(itFirst_, std::next(itFirst_, size()));
        }

        /**
         * Returns TextureView of mipmap at LOD idx.
         * @param lod - index of mipmap. Should start at index 1 as index 0 is current top texture.
         * @param mipLevels - number of mipmaps at higher LOD index to extract with mipmap at lod index.
         *                    e.g.: lod + n mipmaps.
         *                    By default all mipmaps after lod index are extracted.
         *                    If mipLevels is greater then remaining mipmaps after lod index then
         *                    those remaining mipmaps are extracted.
         * @return TextureView
         * @throw std::out_of_range if lod index is invalid.
         */
        TextureView mipmap(uint32_t lod, std::optional<uint32_t> mipLevels = std::nullopt) const;

        /**
         * Returns Texture converted to the given color format.
         * Texture has the same MipMip levels as this TextView.
         * @param format - color format to convert new Texture to.
         * @return converted copy of the Texture.
         * @throw can throw std::runtime_error and std::overflow_error
         */
        Texture converted(const ColorFormat& format) const;

    private:
        uint32_t width_;
        uint32_t height_;
        uint32_t stride_;
        uint32_t mipLevels_;
        const ColorFormat* cf_;
        Pixdata::const_iterator itFirst_;
        Pixdata::const_iterator itLast_;
    };
}

#endif // LIBIM_TEXTURE_VIEW_H