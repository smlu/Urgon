#ifndef LIBIM_TEXUTILS_H
#define LIBIM_TEXUTILS_H
#include "colorformat.h"
#include "texture.h"
#include <libim/common.h>
#include <libim/io/stream.h>
#include <libim/math/color.h>
#include <libim/math/math.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <tuple>
#include <type_traits>

namespace libim::content::asset {

    /**
     * Loads Texture from PNG (Portable Network Graphics) file format stream.
     *
     * @param istream - input PNG file format stream.
     * @return Texture object.
     * @throw StreamError if invalid PNG file format stream or TextureError if texture can't be created.
     */
    Texture pngLoad(const InputStream& istream);

    /**
     * Writes Texture to stream as PNG (Portable Network Graphics) file format.
     *
     * @param ostream - reference to output stream.
     * @param tex     - const reference to TextureView object to write to stream.
     */
    void pngWrite(OutputStream& ostream, const TextureView& tex);

    /**
     * Writes Texture to stream as PNG (Portable Network Graphics) file format.
     *
     * @param ostream - r-value reference to output stream.
     * @param tex     - const reference to TextureView object to write to stream.
     * @throw StreamError if tex is empty or tex can't be written to stream.
     */
    inline void pngWrite(OutputStream&& ostream, const TextureView& tex) {
        pngWrite(ostream, tex);
    }

    /**
     * Loads Texture from BMP (Bitmap) file format stream.
     *
     * @param istream - input BMP file format stream.
     * @return Texture object.
     * @throw StreamError if invalid PNG file format stream or TextureError if texture can't be created.
     */
    Texture bmpLoad(const InputStream& istream);

    /**
     * Writes Texture to stream as BMP (Bitmap) file format.
     *
     * @param ostream - reference to output stream.
     * @param tex     - const reference to TextureView object to write to stream.
     * @throw StreamError if tex is empty or tex can't be written to stream.
     */
    void bmpWrite(OutputStream& ostream, const TextureView& tex);

     /**
     * Writes Texture to stream as BMP (Bitmap) file format.
     *
     * @param ostream - r-value reference to output stream.
     * @param tex     - const reference to TextureView object to write to stream.
     * @throw StreamError if tex is empty or tex can't be written to stream.
     */
    inline void bmpWrite(OutputStream&& ostream, const TextureView& tex) {
        bmpWrite(ostream, tex);
    }

    /**
     * Returns PixdataPtr object of specific size.
     *
     * @param size - byte size of new PixdataPtr object.
     * @return PixdataPtr
     */
    inline PixdataPtr makePixdataPtr(std::size_t size) {
        return std::make_shared<Pixdata>(size);
    }

    /**
     * Returns PixdataPtr object with copied data from another Pixdata object.
     *
     * @param itFirst - const iterator to the begining of Pixdata to copy data from.
     * @param itLast  - const iterator to the end of Pixdata to copy data from.
     * @return PixdataPtr
     */
    inline PixdataPtr makePixdataPtr(Pixdata::const_iterator itFirst, Pixdata::const_iterator itLast) {
        return std::make_shared<Pixdata>(itFirst, itLast);
    }

    /**
     * Returns image row stride.
     * Stride is calculated by multiplaying image width with color depth in bytes (BPP).
     *
     * @param width - image width.
     * @param ci    - image color format.
     * @return image row stride size in bytes.
     */
    inline constexpr uint32_t calcStride(uint32_t width, const ColorFormat& ci) {
        return width * bbs(ci.bpp);
    }

    /**
     * Returns maximum number of LOD levels MipMap can have at given width and height.
     *
     * @param width  - image width.
     * @param height - image height.
     * @return max number of LOD levels.
     */
    inline uint32_t calcMaxMipmapLevels(uint32_t width, uint32_t height) {
        return static_cast<uint32_t>(std::floor(std::log2(max(width, height)))) + 1;
    }

    /**
     * Calculates size of image's pixel data.
     *
     * @param width  - image width.
     * @param height - image height.
     * @param ci     - const reference to color format of image.
     * @return image Pixdata size.
     */
    inline constexpr uint32_t calcPixdataSize(uint32_t width, uint32_t height, const ColorFormat& ci) {
        return height * width * bbs(ci.bpp);
    }

    /**
     * Calculates whole MipMap's pixel data size.
     *
     * @param width     - MipMap width.
     * @param height    - MipMap height.
     * @param mipLevels - number of LOD levels in MipMap.
     * @param ci        - const reference to color format of MipMap.
     * @return image MipMap Pixdata size.
     */
    inline constexpr uint32_t calcMipmapSize(uint32_t width, uint32_t height, std::size_t mipLevels, const ColorFormat& ci)
    {
        uint32_t size = 0;
        while (mipLevels --> 0 && width && height)
        {
            size  += calcPixdataSize(width, height, ci);
            width  = width  >> 1;
            height = height >> 1;
        }
        return size;
    }

    /**
     * Returns range of mipmaps from Pixdata iterator starting at lod.
     *
     * @param lod       - first LOD level in itFirst to return begin iterator.
     * @param itFirst   - start const Pixdata iterator to get mipmaps from.
     * @param width     - width of mipmap texture at itFirst.
     * @param height    - height of mipmap texture at itFirst.
     * @param mipLevels - num mipmap levels to return end mipmap iterator range.
     * @param cf        - Color format of pixel data in itFirst.
     * @return std::tuple(firstMipHight, firstMipHeight, itMipBegin, itMipEnd)
     */
    [[nodiscard]] inline auto getMipmapsAtLevel(uint32_t lod, Pixdata::const_iterator itFirst, uint32_t width, uint32_t height, uint32_t mipLevels, const ColorFormat& cf)
        -> std::tuple<uint32_t /*mipWidth*/, uint32_t /*mipHeight*/, Pixdata::const_iterator /*itMipBegin*/, Pixdata::const_iterator /*itMipBegin*/>
    {
        const auto scale     = 1 / std::pow(2, lod);
        const auto mipWidth  = static_cast<uint32_t>(width  * scale);
        const auto mipHeight = static_cast<uint32_t>(height * scale);

        auto itMipBegin = std::next(itFirst, calcMipmapSize(width, height, lod, cf));
        auto itMipEnd   = std::next(itMipBegin, calcMipmapSize(mipWidth, mipHeight, mipLevels, cf));
        return std::tuple(mipWidth, mipHeight, itMipBegin, itMipEnd);
    }

    /**
     * Reads Texture from input pixel data range to reference argument and return end iterator.
     *
     * @param first     - const iterator to the beginning of pixel data.
     * @param last      - const iterator to the end of pixel data.
     * @param width     - out texture width.
     * @param height    - out texture height.
     * @param mipLevels - out texture mipmap levels.
     * @param colorInfo - const reference to ColorFormat out texture.
     * @param tex       - reference to Texture object to write out texture to.
     *
     * @return cont interator of source buffer to the end of read texture.
     * @throw - std::out_of_range if texture size is greater than pixel data buffer size.
     *        - TextureError if texture can't be created.
    */
    Pixdata::const_iterator
    copyTextureFromPixdata(
        Pixdata::const_iterator first,
        Pixdata::const_iterator last,
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels,
        const ColorFormat& colorInfo,
        Texture& tex
    );

    /**
     * Returns color component bit mask.
     * @param bpc - bits per color component.
     * @return uint32_t color component bit mask.
     */
    inline uint32_t getColorMask(uint32_t bpc) {
        return 0xFFFFFFFF >> (32 - bpc);
    }

     /**
     * Returns mask of color component.
     * @param bpc  - bits per color component.
     * @param cshl - color component left bit shift.
     * @return color component bit mask.
     */
    inline constexpr uint32_t getColorShlMask(uint32_t bpc, uint32_t cshl) {
        return ((1 << bpc) - 1) << cshl;
    }

    /**
     * Returns bit count per color component from left shift position mask
     * @param mask - color mask
     *
     * @note reference implementation taken from stb lib:
     *       https://github.com/nothings/stb/blob/0224a44a10564a214595797b4c88323f79a5f934/stb_image.h#L5170
     */
    static uint32_t getBPCFromMask(uint32_t mask)
    {
        mask = (mask & 0x55555555) + ((mask >>  1) & 0x55555555); // max 2
        mask = (mask & 0x33333333) + ((mask >>  2) & 0x33333333); // max 4
        mask = (mask + (mask >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
        mask = (mask + (mask >> 8));  // max 16 per 8 bits
        mask = (mask + (mask >> 16)); // max 32 per 8 bits
        return mask & 0xff;
    }

    /**
     * Returns color component left shit position from mask
     * @param mask - color mask
     */
    inline uint32_t getLeftShiftPosFromMask(uint32_t mask) {
        return mask == 0 ? 0: getBPCFromMask((mask & (~mask + 1)) - 1);
    }

    /**
     * Decodes the encoded pixel and returns it as Color object.
     *
     * @param encPixel - encoded pixel of type PT. PT should be integral type of size 4 bytes (eg.: uint32_t) or less.
     * @param ci       - color format of encoded pixel.
     * @return Color object of decoded pixel.
     */
    template<typename PT>
    inline Color decodePixel(PT encPixel, const ColorFormat& ci)
    {
        static_assert(std::is_integral_v<PT> && sizeof(PT) <= 4);

        uint32_t r = ((encPixel >> ci.redShl)   & getColorMask(ci.redBPP))   << ci.redShr;
        uint32_t g = ((encPixel >> ci.greenShl) & getColorMask(ci.greenBPP)) << ci.greenShr;
        uint32_t b = ((encPixel >> ci.blueShl)  & getColorMask(ci.blueBPP))  << ci.blueShr;
        uint32_t a = 255;
        if (ci.alphaBPP != 0)
        {
            a = ((encPixel >> ci.alphaShl) & getColorMask(ci.alphaBPP)) << ci.alphaShr;
            if (ci.alphaBPP == 1) { // RGBA5551
                a = a > 0 ?  255  : 0;
            }
        }

        return makeColor(
            clamp<uint8_t>(r, Color::min(), Color::max()),
            clamp<uint8_t>(g, Color::min(), Color::max()),
            clamp<uint8_t>(b, Color::min(), Color::max()),
            clamp<uint8_t>(a, Color::min(), Color::max())
        );
    }

    /**
     * Encodes pixel.
     *
     * @param pixel - Color object pixel to encode.
     * @param ci    - color format to encode pixel to.
     * @return encoded uint32_t pixel.
     */
    inline uint32_t encodePixel(Color pixel, const ColorFormat& ci)
    {
        uint32_t r = pixel.red();//uint32_t(pixel.red()   * 255.f);
        uint32_t g = pixel.green();//uint32_t(pixel.green() * 255.f);
        uint32_t b = pixel.blue();//uint32_t(pixel.blue()  * 255.f);
        uint32_t a = pixel.alpha();//uint32_t(pixel.alpha() * 255.f);

        uint32_t ep =
            ((r >> ci.redShr)   << ci.redShl)   |
            ((g >> ci.greenShr) << ci.greenShl) |
            ((b >> ci.blueShr)  << ci.blueShl);

        if (ci.alphaBPP != 0) {
            ep |= (a >> ci.alphaShr) << ci.alphaShl;
        }

        return ep;
    }

    /**
     * Reads pixel from buffer.
     *
     * @param pPixdataSrc - byte_t pointer to buffer to read pixel from.
     * @param size        - available size to read. Argument is used to verify
     *                      enough data is availabe to read pixel from buffer.
     * @param ci          - color format of encoded pixel to decode to Color object.
     *
     * @return non-linear space Color object.
     * @throw std::overflow_error not enough data is available to read encoded pixel from buffer.
     */
    inline Color readPixel(const byte_t* pPixdataSrc, uint32_t size, const ColorFormat& ci)
    {
        auto pixelSize = bbs(ci.bpp);
        assert(pixelSize > 1 && pixelSize <= 5);
        assert(pPixdataSrc != nullptr);

        if (size < pixelSize) {
            throw std::overflow_error("Can't extract pixel from pixdata due to overflow");
        }

        if (pixelSize == 2) {
            return decodePixel(*reinterpret_cast<const uint16_t*>(pPixdataSrc), ci);
        }
        else if (pixelSize == 3)
        {
            uint32_t encPixel = *pPixdataSrc  | (*(pPixdataSrc + 1) << 8) | (*(pPixdataSrc + 2) << 16);
            return decodePixel(encPixel, ci);
        }
        else { // pixelSize == 4
            return decodePixel(
                *reinterpret_cast<const uint32_t*>(pPixdataSrc), ci
            );
        }
    }

    /**
     * Writes pixel to buffer.
     *
     * @param pixel        - non-linear space Color object to write to buffer.
     * @param pPixdataDest - byte_t pointer to buffer to write pixel to.
     * @param size         - available size to write. Argument is used to verify
     *                      enough data is availabe to write pixel to buffer.
     * @param ci           - color format to encode pixel to.
     * @throw std::overflow_error if encoded pixel size is greater than available size in buffer.
     */
    inline void writePixel(const Color& pixel, byte_t* pPixdataDest, uint32_t size, const ColorFormat& ci)
    {
        auto encPixel  = encodePixel(pixel, ci);
        auto pixelSize = bbs(ci.bpp);

        assert(pixelSize > 1 && pixelSize <= 4);
        assert(pPixdataDest != nullptr);
        if (size < pixelSize) {
            throw std::overflow_error("Can't write pixel to pixdata due to overflow");
        }

        memcpy(pPixdataDest, reinterpret_cast<byte_t*>(&encPixel), pixelSize);
    }

    /**
     * Reads pixel from buffer at specific location defined by x,y coordinates.
     *
     * @param itFirst - const iterator of Pixdata to read pixel from.
     *                  Iterator should point at the beginning of image.
     * @param x       - x coordinate to location to read encoded pixel from.
     * @param y       - y coordinate to location to read encoded pixel from.
     * @param width   - image width.
     * @param height  - image height.
     * @param cf      - color format of encoded pixel to decode to Color object.
     *
     * @return non-linear space Color object.
     * @throw std::overflow_error if x/y is greater than width/height or there is not enough data ro read pixel.
     */
    inline Color readPixelAt(Pixdata::const_iterator itFirst, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const ColorFormat& cf)
    {
        if (x >= width || y >= height) {
            throw std::overflow_error("Can't get pixel from pixdata due to overflow");
        }

        const uint32_t pixSize = bbs(cf.bpp);
        const uint32_t rowLen  = width * pixSize;
        const uint32_t pos     = y * rowLen + x * pixSize;
        const byte_t* pPixdata = &(*(itFirst + pos));
        return readPixel(pPixdata, (rowLen * height) - pos, cf);
    }

    /**
     * Writes pixel to buffer at specific location defined by x,y coordinates.
     *
     * @param pixel   - non-linear space Color object to write to buffer.
     * @param itFirst - const iterator of Pixdata to write pixel to.
     *                  Iterator should point at the beginning of image.
     * @param x       - x coordinate to location to write encoded pixel from.
     * @param y       - y coordinate to location to write encoded pixel from.
     * @param width   - image width.
     * @param height  - image height.
     * @param cf      - color format to encode pixel to.
     *
     * @throw std::overflow_error if x/y is greater than width/height or there is not enough data to write pixel.
     */
    inline void writePixelAt(const Color& pixel, Pixdata::iterator itFirst, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const ColorFormat& cf)
    {
        if (x >= width || y >= height) {
            throw std::overflow_error("Can't put pixel to pixdata due to overflow");
        }

        const uint32_t pixSize = bbs(cf.bpp);
        const uint32_t rowLen  = width * pixSize;
        const uint32_t pos     = y * rowLen + x * pixSize;
        byte_t* pPixdata       = &(*(itFirst + pos));
        writePixel(pixel, pPixdata, (rowLen * height) - pos, cf);
    }

    /**
     * Converts pixeldata row to different ColorFormat.
     * Data pointed by source pixel data is not modified.
     *
     * @param pRowSrc    - const pointer to the source pixel data.
     * @param rowLenSrc  - source row length (stride).
     * @param ciSrc      - ColorFormat of source pixel data.
     * @param pRowDest   - pointer to the destination pixel data.
     * @param rowLenDest - destination row length (stride)
     * @param ciDest     - ColorFormat to convert source pixel data to and write it to pRowDest.
     *
     * @throw - if readPixel or writePixel throw.
     */
    void convertPixdataRow(const byte_t* pRowSrc, uint32_t rowLenSrc, const ColorFormat& ciSrc,
        byte_t* pRowDest, uint32_t rowLenDest, const ColorFormat& ciDest);

    /**
     * Converts inputted pixdata to different ColorFormat and return new pixel data.
     * Data pointed by inputted pixel data is not modified.
     *
     * @param ptrPixdataSrc - input shared pointer to pixel data (PixdataPtr).
     * @param width         - width of image pointed by the range iterators itSrcFirst - itSrcLast.
     * @param height        - height of image pointed by the range iterators itSrcFirst - itSrcLast.
     * @param from          - const reference to the ColorFormat of the image pointed by the range iterators itSrcFirst - itSrcLast.
     * @param to            - const reference to the ColorFormat to convert returned pixel data to.
     *
     * @return shared pointer to new converted pixdata (PixDataPtr).
     * @throw - std::runtime_error if range iterators itSrcFirst and itSrcLast are invalid
     *          e.g.: calculated image size - stride * height != distance(itSrcFirst, itSrcLast).
     *        - std::runtime_error if bpp of argument to or from is invalid. e.g.: bpp % != 0 or bpp < 16 or bpp > 32.
     */
    [[nodiscard]] PixdataPtr
    convertPixdata(PixdataPtr ptrPixdataSrc, uint32_t width, uint32_t height, const ColorFormat& from, const ColorFormat& to);

    /**
     * Converts inputted pixel data range iterators to different ColorFormat and return new pixel data.
     * Data pointed by range iterators is not modified.
     *
     * @param itSrcFirst - input const iterator to begining.
     * @param itSrcLast  - input const iterator to end.
     * @param width      - width of image pointed by the range iterators itSrcFirst - itSrcLast.
     * @param height     - height of image pointed by the range iterators itSrcFirst - itSrcLast.
     * @param from       - const reference to the ColorFormat of the image pointed by the range iterators itSrcFirst - itSrcLast.
     * @param to         - const reference to the ColorFormat to convert returned pixel data to.
     *
     * @return shared pointer to new converted pixdata (PixDataPtr).
     * @throw - std::runtime_error if range iterators itSrcFirst and itSrcLast are invalid
     *          e.g.: calculated image size - stride * height != distance(itSrcFirst, itSrcLast).
     *        - std::runtime_error if bpp of argument to or from is invalid. e.g.: bpp % != 0 or bpp < 16 or bpp > 32.
     *        - if convertPixdataRow throws.
     */
    [[nodiscard]] PixdataPtr
    convertPixdata(Pixdata::const_iterator itSrcFirst, Pixdata::const_iterator itSrcLast, uint32_t width, uint32_t height, const ColorFormat& from, const ColorFormat& to);

    /**
     * Scales image using box-filtering algorithm (pixel-averaging)
     *
     * @param itSrc      - source Pixdata const iterator.
     * @param srcWidth   - source Pixdata width.
     * @param srcHeight  - source Pixdata height.
     * @param itDest     - destination Pixdata const iterator.
     * @param destWidth  - destination Pixdata width.
     * @param destHeight - destination Pixdata height.
     * @param cf         - ColorFormat of source / destination Pixdata.
     * @param sRGB       - (optional) do sRGB color space conversion when scaling down mipmap images.
     *                     Default is true.
     * @throw there is not enough data to read or write pixel.
     */
    void boxFilterScale(Pixdata::const_iterator itSrc, uint32_t srcWidth, uint32_t srcHeight, Pixdata::iterator itDest, uint32_t destWidth, uint32_t destHeight, const ColorFormat& cf, bool sRGB = true);
}
#endif // LIBIM_TEXUTILS_H