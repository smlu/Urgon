#ifndef LIBIM_TEXTURE_H
#define LIBIM_TEXTURE_H
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "colorformat.h"

#include <libim/common.h>
#include <libim/io/stream.h>

namespace libim::content::asset{
    using Pixdata    = ByteArray; // ByteArray alias which represents uncompressed but possible encoded pixel data
    using PixdataPtr = std::shared_ptr<Pixdata>;

    class TextureView;

    class TextureError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    /** Represents 2D top-down MipMap texture image. */
    class Texture
    {
    public:
        Texture() = default;

        /**
         * Constructs new Texture.
         * @note pixel data row should not be padded.
         *
         * @param width      - texture width at lod 0
         * @param height     - texture height at lod 0
         * @param mipLevels  - number of MipMap levels in ptrPixdata.
         * @param format     - color format.
         * @param ptrPixdata - shared pointer to the pixel date of texture.
         *                     The size of ptrPixdata must be the same as calculated MipMap size via function calcMipmapSize.
         * @throw TextureError if invalid texture size (width or height is 0) and if wrong color format for given ptrPixdata arg.
        */
        Texture(uint32_t width, uint32_t height, uint32_t mipLevels, ColorFormat format, PixdataPtr ptrPixdata);

        /**
         * Constructs new Texture from TextureView.
         *
         * @param tv - const reference to TextureView.
         */
        explicit Texture(const TextureView& tv);

        Texture(const Texture& rhs);
        Texture(Texture&& rrhs) noexcept;              //!< @note if there is any TextureView object of rrhs it will be invalidated.

        Texture& operator = (const TextureView& tv);   //!< @note if tv is sub view of this texture it will be invalidated. Also, if there is any TextureView object of this texture it will be invalidated.
        Texture& operator = (const Texture& rhs);      //!< @note if there is any TextureView object of this texture it will be invalidated.
        Texture& operator = (Texture&& rrhs) noexcept; //!< @note if there is any TextureView object of this texture or rrhs it will be invalidated.

        virtual ~Texture() = default;

        /** Returns width of top image at LOD 0. */
        uint32_t width() const
        {
            return width_;
        }

        /** Returns height of top image at LOD 0. */
        uint32_t height() const
        {
            return height_;
        }

        /**
         * Returns pixdata row stride.
         * e.g.: BPP * width
         */
        uint32_t stride() const
        {
            return stride_;
        }

        /**
         * Returns number of LOD levels in MipMap.
         * @return number of LOD levels.
         */
        uint32_t mipLevels() const
        {
            return mipLevels_;
        }

        const ColorFormat& format() const
        {
            return cf_;
        }

        bool isEmpty() const
        {
            return !ptrPixdata_ || ptrPixdata_->empty();
        }

        /**
         * Returns whole MipMap pixel data shared pointer.
         * @return PixdataPtr
        */
        [[nodiscard]] PixdataPtr pixdata() const
        {
            return ptrPixdata_;
        }

        /**
         * Returns TextureView of mipmap at LOD idx.
         * @param lod - index of mipmap. Should start at index 1 as index 0 is current top texture (LOD 0).
         * @param mipLevels - number of mipmaps at higher LOD index to extract with mipmap at lod index.
         *                    e.g.: lod + n mipmaps.
         *                    By default all mipmaps after lod index are extracted.
         *                    If mipLevels is greater then remaining mipmaps after lod index then
         *                    those remaining mipmaps are extracted.
         * @return TextureView of mipmap texture at lod idx.
         * @throw std::out_of_range if lod index is invalid.
         */
        [[nodiscard]] TextureView mipmap(uint32_t lod, std::optional<uint32_t> mipLevels = std::nullopt) const;

        /**
         * Returns clone of this texture.
         * @return Texture.
         */
        [[nodiscard]] Texture clone() const;

        /**
         * Converts this texture converted to the given color format.
         * If format is the same as what is returned by format() then no conversion is done.
         *
         * @param format - color format to convert new Texture to.
         * @return reference to this.
         * @throw can throw std::runtime_error and std::overflow_error
         */
        Texture& convert(const ColorFormat& format);

        /**
         * Returns copy of this texture converted to the given color format.
         * If format is the same as this then clone Texture of this is returned.
         *
         * @note Any TextureView based on this texture or Pixdata iterator
         *       returned by pixdata().begin() / pixdata().end() is invalidated.
         *
         * @param format - color format to convert new Texture to.
         * @return converted copy of the Texture.
         * @throw can throw std::runtime_error and std::overflow_error
         */
        [[nodiscard]] Texture converted(const ColorFormat& format) const;

        /** Returns the maximum numper of MipMap levels this texture can have */
        uint32_t maxMipLevels() const;

        /**
         * Generates new MipMap chain for this Texture.
         * MipMaps are generated using box filter (pixel averaging) algorithm for down scaling.
         * @note Any TextureView based on this texture or Pixdata iterator
         *       returned by pixdata().begin() / pixdata().end() is invalidated.
         *
         * @param optMipLevels - (optional) number of MipMaps to generate.
         *                       If optMipLevels is 0 or 1 no MipMap chain is generated instead
         *                       MipMap chain is cleared to image at LOD 0 (same as calling clearMipmaps()).
         * @param optFormat    - (optional) color format to convert returned MipMap texture to.
         * @param sRGB         - (optional) do sRGB color space conversion when scaling down mipmap images.
         *                       Default is true.
         * @return reference to this.
         * @throw can throw std::runtime_error and std::overflow_error
         */
        Texture& generateMipmaps(std::optional<uint32_t> optMipLevels, std::optional<ColorFormat> optFormat = std::nullopt, bool sRGB = true);

        /**
         * Makes new MipMap Texture from this with optMipLevels.
         * MipMaps are generated using box filter (pixel averaging) algorithm for down scaling.
         *
         * @param optMipLevels - (optional) number of MipMaps to generate.
         *                       Default is what returns maxMipLevels().
         *                       If optMipLevels is 0 or 1 no MipMap chain is generated instead
         *                       Texture with image at LOD 0 is returned.
         * @param optFormat    - (optional) color format to convert returned MipMap texture to.
         * @param sRGB         - (optional) do sRGB color space conversion when scaling down mipmap images.
         *                       Default is true.
         * @return new MipMap Texture
         * @throw can throw std::runtime_error and std::overflow_error
         */
        [[nodiscard]] Texture makeMipmap(std::optional<uint32_t> optMipLevels, std::optional<ColorFormat> optFormat = std::nullopt, bool sRGB = true) const;

        /**
         * Removes MipMap chain form this Texture leaving only top image at LOD 0.
         * @note Any TextureView based on this texture or Pixdata iterator
         *       returned by pixdata().begin() / pixdata().end() is invalidated.
         * @return reference to this.
         */
        Texture& clearMipmaps();

        /**
         * Scales this Texture to new size using box-filtering algorithm.
         * @note Any TextureView based on this texture or Pixdata iterator
         *       returned by pixdata().begin() / pixdata().end() is invalidated.
         *
         * @param width  - new texture width.
         * @param height - new texture height.
         * @param sRGB   - (optional) do sRGB color space conversion when scaling image.
         *                 Default is true.
         * @return reference to this.
         * @throw can throw std::runtime_error and std::overflow_error
         */
        Texture& scale(uint32_t width, uint32_t height, bool sRGB = true);

        /**
         * Returns clone Texture scaled to the new size using box-filtering algorithm.
         *
         * @param width  - new texture width.
         * @param height - new texture height.
         * @param sRGB   - (optional) do sRGB color space conversion when scaling image.
         *                 Default is true.
         * @return scaled Texture.
         * @throw can throw std::runtime_error and std::overflow_error
         */
        [[nodiscard]] Texture scaled(uint32_t width, uint32_t height, bool sRGB = true) const;

    private:
        uint32_t width_     = 0;
        uint32_t height_    = 0;
        uint32_t stride_    = 0;
        uint32_t mipLevels_ = 0;
        ColorFormat cf_;
        PixdataPtr ptrPixdata_;
    };
}

namespace libim {
    /** Stream template specialization for reading
     *  raw texture pixeldata from Stream
     */
    template<> content::asset::Texture
    Stream::read<
        content::asset::Texture,
        uint32_t,
        uint32_t,
        uint32_t,
        const content::asset::ColorFormat&>
    (
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels,
        const content::asset::ColorFormat& format
    ) const;
}

#endif // LIBIM_TEXTURE_H