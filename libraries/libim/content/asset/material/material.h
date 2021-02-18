#ifndef LIBIM_MATERIAL_H
#define LIBIM_MATERIAL_H
#include <cstdint>
 #include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../asset.h"
#include "texture.h"

#include <libim/common.h>
#include <libim/io/stream.h>

namespace libim::content::asset {

    struct MaterialError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    /**
     * Material is an image with celluloid textures aka cells.
     * All textures have the same width, height and color format.
     */
    class Material final : public Asset
    {
    public:
        using Asset::Asset;

        Material(const Material&) = default;
        Material(Material&&) noexcept = default;
        Material& operator = (const Material&) = default;
        Material& operator = (Material&&) noexcept = default;
        virtual ~Material() override = default;

        /**
         * Returns texture width of material cells.
         * If material is empty 0 is returned.
         */
        uint32_t width() const
        {
            return isEmpty() ? 0 : cells_.at(0).width();
        }

        /**
         * Returns texture hight of material cells.
         * If material is empty 0 is returned.
         */
        uint32_t height() const
        {
            return isEmpty() ? 0 : cells_.at(0).height();
        }

        /**
         * Returns texture color format of material cells.
         * @return const reference to the ColorFormat.
         * @throw MaterialError if material is empty.
         */
        const ColorFormat& format() const
        {
            if (isEmpty()) {
                throw MaterialError("Empty material");
            }
            return cells_.at(0).format();
        }

        /** Returns true if material doesn't contain any cel. */
        bool isEmpty() const
        {
            return cells_.empty();
        }

        /**
         * Returns number of material cells.
         * @return std::size_t of number of material cells.
         */
        std::size_t count() const
        {
            return cells_.size();
        }

        /**
         * Returns cel texture.
         *
         * @param optCelIdx - optional index of cel to return texture of.
         *                    If not set the default cel texture is returned.
         * @return const reference to cel texture.
         * @throw std::out_of_range if optCelIdx is invalid.
         */
        const Texture& cel(std::optional<std::size_t> optCelIdx = std::nullopt) const
        {
            return cells_.at(optCelIdx.value_or(celIdx_));
        }

        /**
         * Returns list of all aterial celluloid textures.
         * @return const reference to std::vector<Texture>
         */
        const std::vector<Texture>& cells() const
        {
            return cells_;
        }

        /**
         * Adds celluloid texture to material.
         *
         * @param cel - celluloid texture to add.
         * @return reference to this
         * @throw MaterialError if cel texture width, height, ColorFormat
         *        or number of MipMap levels is not the same as this material.
         */
        Material& addCel(Texture cel);

        /**
         * Sets material celluloid textures.
         *
         * @param cells - std::vector of textures.
         * @return reference to this
         * @throw MaterialError if any texture has different width, height or ColorFormat
         *  or if the first cel in list is empty.
         */
        Material& setCells(std::vector<Texture> cells);

        /**
         * Sets default cel index.
         * Default cel index is used when member function cel() is called
         * without passing an optional cel index argument.
         *
         * @param idx - cel index.
         * @return reference to this
         * @throw MaterialError if idx is out of range.
         */
        Material& setDefaultCel(std::size_t idx)
        {
            if (idx >= count()) {
                throw MaterialError("Can't set default material cel, invalid index");
            }
            celIdx_ = idx;
            return *this;
        }

    private:
        bool isValidCel(const Texture& cel);

    private:
        std::size_t celIdx_ = 0; // default cel
        std::vector<Texture> cells_;
    };

    /**
     * Loads Material as MAT file format from input stream.
     * @param istream - input stream to read Material from
     * @return Material
     * @throw StreamError   - if invalid MAT file in the istream
     *        MaterialError - if textures stored in MAT file are invalid i.e. either not the same size or don't have the same mip level.
     */
    Material matLoad(const InputStream& istream);

    /**
     * Loads Material from MAT file format.
     * @param istream - input rvalue stream to read Material from
     * @return Material
     * @throw StreamError   - if invalid MAT file in the istream
     *        MaterialError - if textures stored in MAT file are invalid i.e. either not the same size or don't have the same mip level.
     */
    Material matLoad(InputStream&& istream);

    /**
     * Writes Material as MAT file format to output stream.
     * @param mat - Material to write to ostream
     * @param ostream - output stream reference to write Material to
     * @return true if writting took place, otherwise false
     * @throw StreamError if error has occur while writting Material to stream.
     */
    bool matWrite(const Material& mat, OutputStream&& ostream);

    /**
     * Writes Material as MAT file format to output stream.
     * @param mat - Material to write to ostream
     * @param ostream - output rvalue stream reference to write Material to
     * @return true if writting took place, otherwise false
     * @throw StreamError if error has occur while writting Material to stream.
     */
    bool matWrite(const Material& mat, OutputStream& ostream);
}

#endif // LIBIM_MATERIAL_H