#include "../texture.h"
#include "../texutils.h"
#include "../colorformat.h"
#include "bmp.h"

#include <libim/math/math.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <png.h>
#include <zlib.h>

using namespace libim;
using namespace libim::content::asset;
using namespace std::string_literals;

#if !defined(PNG_READ_TRANSFORMS_SUPPORTED)  || \
    !defined(PNG_READ_GRAY_TO_RGB_SUPPORTED) || \
    !defined(PNG_READ_EXPAND_SUPPORTED)      || \
    !defined(PNG_READ_STRIP_16_TO_8_SUPPORTED)
#  error "libpng doesn't support transformations!"
#endif

static_assert(sizeof(png_byte) == sizeof(byte_t));


// Flips image over y coord (height)
void pixdataFlipOverY(uint32_t bpp, uint32_t width, uint32_t height, Pixdata& pixdata)
{
    auto target = bbs(bpp);
    auto stride = target * width;
    for (uint32_t r = 0; r < height >> 1; r++)
    {
        byte_t* p1 = &pixdata.at(r * stride);
        byte_t* p2 = &pixdata.at((height - 1 - r) * stride);
        for (uint32_t c = 0; c < stride; c++) {
            std::swap(p1[c], p2[c]);
        }
    }
}

Texture libim::content::asset::pngReadTexture(const InputStream& istream)
{
    /* Read PNG file signature */
    auto pngsig = istream.read<std::array<png_byte, 8>>();
    if (!png_check_sig(pngsig.data(), pngsig.size())) {
        throw StreamError("Can't read PNG from stream, bad signature");
    }

    png_structp pngPtr = png_create_read_struct(png_get_libpng_ver(nullptr), nullptr, nullptr, nullptr);
    if (!pngPtr) {
        StreamError("Error creating png_structp");
    }

    /* Set PNG write and flush functions */
    png_set_read_fn(pngPtr, const_cast<InputStream*>(&istream),
        [](png_structp pngPtr, png_bytep data, png_size_t size){
            InputStream* pstream =
                reinterpret_cast<InputStream*>(png_get_io_ptr(pngPtr));
            if (pstream->read(reinterpret_cast<byte_t*>(data), size) != size) {
                throw StreamError("Error reading texture as PNG file format from stream");
            }
        }
    );

    /* Allocate the image information data. */
    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        png_destroy_read_struct(&pngPtr,  nullptr, nullptr);
        StreamError("Error creating png_infop");
    }

    /* Set clean-up function */
    AT_SCOPE_EXIT([pngPtr, infoPtr]() mutable {
        png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
    });

    /* Read info header */
    if (setjmp(png_jmpbuf(pngPtr))) {
        StreamError("Error reading PNG info header");
    }

    png_set_sig_bytes(pngPtr, pngsig.size());
    png_read_info(pngPtr, infoPtr);

    png_uint_32 width       = png_get_image_width(pngPtr, infoPtr);
    png_uint_32 height      = png_get_image_height(pngPtr, infoPtr);
    png_uint_32 bpc         = png_get_bit_depth(pngPtr, infoPtr); // bits per channel
    png_uint_32 colorType   = png_get_color_type(pngPtr, infoPtr);

    /* Register transformations */
    switch (colorType)
    {
        case PNG_COLOR_TYPE_PALETTE:
            png_set_palette_to_rgb(pngPtr);
            png_set_expand(pngPtr);
            break;
        case PNG_COLOR_TYPE_GRAY:
            if (bpc < 8) {
                png_set_expand_gray_1_2_4_to_8(pngPtr);
            }
            [[fallthrough]];
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            png_set_gray_to_rgb(pngPtr);
            break;
    }

    /* Convert tRNS chunk to alpha channel */
    if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(pngPtr);
    }

    /* Convert 16 bpc to 8 bpc */
    if (bpc == 16) {
        png_set_strip_16(pngPtr);
    }

    /* Update info */
    png_read_update_info(pngPtr, infoPtr);

    /* Read image */
    if (setjmp(png_jmpbuf(pngPtr))) {
        StreamError("Error reading PNG image");
    }

    png_uint_32 rowbytes = safe_cast<png_uint_32>(png_get_rowbytes(pngPtr, infoPtr));
    auto ptrPixdata      = makePixdataPtr(rowbytes * height);
    auto vecRowPointers  = std::vector<png_bytep>(height);
    for (uint32_t i = 0;  i < height;  i++) {
        vecRowPointers[i] = &ptrPixdata->at(i * rowbytes);
    }

    png_read_image(pngPtr, vecRowPointers.data());
    png_read_end(pngPtr, nullptr);

    bpc = png_get_bit_depth(pngPtr, infoPtr);
    png_uint_32 numChannels = png_get_channels(pngPtr, infoPtr);
    assert(numChannels == 3 || numChannels == 4 && "invalid number of channels in PNG image");
    assert(bpc == 8&& "bit per channel is invalid in PNG image");
    auto cf = numChannels == 3 ? RGB24be : RGBA32be;

    return Texture(width, height, 1, cf, std::move(ptrPixdata));
}

void libim::content::asset::pngWriteTexture(OutputStream& ostream, const TextureView& tex)
{
    // Note: PNG file format stores pixel channels in big-endian RGB8 ot RGBA8 format.
    //       Texture which has color depth less then 24 BPP (e.g.16bit) is converted
    //       to RGB24be or RGBA24be color format to unpack color channels to 1 byte per channel.


    if (tex.isEmpty()) {
        throw StreamError("Can't write empty texture as PNG file format to stream");
    }

    const auto ci = tex.format();
    if (ci.mode == ColorMode::Indexed) {
        StreamError(
            "Can't write palette color mode texture as PNG file format to stream"
        );
    }

    /* Convert pixeldata to 24 BPP or 32 BPP */
    if (ci.bpp == 16)
    {
        auto destCi     = ci.mode == ColorMode::RGB ? RGB24be : RGBA32be;
        auto ptrPixData = convertPixdata(tex.begin(), tex.end(), tex.width(), tex.height(), ci, destCi);
        auto convTex    = Texture(tex.width(), tex.height(), 1, destCi, std::move(ptrPixData));
        return pngWriteTexture(ostream, convTex);
    }

    png_structp pngPtr = png_create_write_struct(png_get_libpng_ver(nullptr), nullptr, nullptr, nullptr);
    if (!pngPtr) {
        StreamError("Error creating png_structp");
    }

    /* Set PNG write and flush functions */
    png_set_write_fn(pngPtr, &ostream,
        [](png_structp pngPtr, png_bytep data, png_size_t size){
            OutputStream* pstream =
                reinterpret_cast<OutputStream*>(png_get_io_ptr(pngPtr));
            if (pstream->write(reinterpret_cast<const byte_t*>(data), size) != size) {
                throw StreamError("Error writing texture as PNG file format to stream");
            }
        },
        [](png_structp pngPtr){
            OutputStream* pstream =
                reinterpret_cast<OutputStream*>(png_get_io_ptr(pngPtr));
            pstream->flush();
        }
    );

    /* Allocate the image information data. */
    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        png_destroy_write_struct(&pngPtr,  nullptr);
        StreamError("Error creating png_infop");
    }

    AT_SCOPE_EXIT([pngPtr, infoPtr]() mutable {
       png_destroy_write_struct(&pngPtr, &infoPtr);
    });

    /* Write PNG info header */
    if (setjmp(png_jmpbuf(pngPtr))) {
        StreamError("Error writing PNG info header");
    }

    int colorType = ci.mode == ColorMode::RGB ?
        PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA;
    png_set_IHDR(
        pngPtr,
        infoPtr,
        tex.width(),
        tex.height(),
        8, // bits per channel
        colorType,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_BASE
    );

    /* Write info */
    png_write_info(pngPtr, infoPtr);

    /* Write image pixel data */

    /* Swap little-endian RGB to BGR */
    if (ci.redShl > ci.blueShl) {
        png_set_bgr(pngPtr);
    }

    /* Swap Alpha if little-endian RGBA or BGRA */
    if (ci.mode == ColorMode::RGBA && ci.alphaShl == 0) {
        png_set_swap_alpha(pngPtr);
    }

    /* Write pixel data */
    const auto rowLen   = tex.stride();
    png_bytep pRow      = const_cast<png_bytep>(&(*tex.begin()));
    auto vecRowPointers = std::vector<png_bytep>(tex.height());
    for (std::size_t i = 0; i < tex.height(); i++, pRow += rowLen) {
        vecRowPointers[i] = pRow;
    }

    if (setjmp(png_jmpbuf(pngPtr))) {
        StreamError("Error calling png_write_image");
    }
    png_write_image(pngPtr, vecRowPointers.data());

    /* Finish writing PNG to stream */
    if (setjmp(png_jmpbuf(pngPtr))) {
        StreamError("Error calling png_write_end");
    }
    png_write_end(pngPtr, nullptr);
    ostream.flush();
}

template<typename InfoHeaderT,
        typename = std::enable_if_t<std::is_base_of_v<BitmapInfoHeader, InfoHeaderT>>>
InfoHeaderT bmpReadInfoHeader(const InputStream& istream)
{
    InfoHeaderT info;
    /* Read bmp info header */
    if (istream.read(reinterpret_cast<byte_t*>(&info), sizeof(info)) != sizeof(info)) {
        throw StreamError("Failed to read BMP DIB header from stream");
    }

    /* Check image BPP - should be 16, 24 or 32 */
    if (info.bpp % 8 != 0 || info.bpp < 16 || info.bpp > 32) {
        throw StreamError("Can't read BMP from stream, invalid pixdata BPP");
    }

    return info;
}

template<typename InfoHeaderT,
    typename = std::enable_if_t<std::is_base_of_v<BitmapInfoHeader, InfoHeaderT>>>
void bmpReadImageInfo(const InputStream& istream, int32_t& width, int32_t& height, uint32_t& imgSize,  ColorFormat& cf)
{
    auto bmih = bmpReadInfoHeader<InfoHeaderT>(istream);
    width     = bmih.width;
    height    = bmih.height;
    imgSize   = bmih.sizeImage;

    if (imgSize == 0)
    {
        if (bmih.compression == BI_RGB) {
            imgSize = bmpCalcPaddedRowLen(width, bmih.bpp) * abs(height);
        }
        else {
            throw StreamError("Corrupted BMP in stream, image size is 0");
        }
    }

    bool bConvSuccess = false;
    if (bmih.size == sizeof(BitmapInfoHeader) && bmih.compression == BI_BITFIELDS)
    {
        // We should have BITMAPINFO struct in the stream.
        // We first read the bmiColors field (RGBQUAD type) then
        // construct bitmap v2 header with bmih and rgb masks from bmiColors field.
        BitmapV2Header bmih2;
        memcpy(&bmih2, &bmih, bmih.size);
        const std::size_t nRead = sizeof(bmih2.redMask) * 3; // 3 RGB color components (RGBQUAD)
        if (istream.read(reinterpret_cast<byte_t*>(&bmih2.redMask), nRead) != nRead) {
            throw StreamError("Failed to read BMP DIB info from stream");
        }
        bConvSuccess = bmpInfoToColorFormat(bmih2, cf);
    }
    else {
        bConvSuccess = bmpInfoToColorFormat(bmih, cf);
    }

    if (!bConvSuccess) {
        throw StreamError("Can't read BMP from stream, unsupported compression type '"s +
            bmpCompressType2Str(bmih.compression) + "'"
        );
    }
}

Texture libim::content::asset::bmpReadTexture(const InputStream& istream)
{
    /* Read header */
    BitmapFileHeader header{};
    if (istream.read(reinterpret_cast<byte_t*>(&header), sizeof(header)) != sizeof(header)) {
        throw StreamError("Failed to read BMP header from stream");
    }

    /* Check bmp type */
    if (header.type != BMP_TYPE) {
        throw StreamError("Can't read BMP from stream, invalid type '"s + bmpType2Str(header.type) + "'");
    }

    ColorFormat cf;
    int32_t width, height;
    uint32_t imgSize;

    const std::size_t dbiSize = header.offBits - sizeof(BitmapFileHeader);
    switch (dbiSize)
    {
        case sizeof(BitmapInfoHeader): {}
            bmpReadImageInfo<BitmapInfoHeader>(istream, width, height, imgSize, cf);
            break;
        case sizeof(BitmapV2Header):
            bmpReadImageInfo<BitmapV2Header>(istream, width, height, imgSize, cf);
            break;
        case sizeof(BitmapV3Header):
            bmpReadImageInfo<BitmapV3Header>(istream, width, height, imgSize, cf);
            break;
        case sizeof(BitmapV4Header):
            bmpReadImageInfo<BitmapV4Header>(istream, width, height, imgSize, cf);
            break;
        case sizeof(BitmapV5Header):
            bmpReadImageInfo<BitmapV5Header>(istream, width, height, imgSize, cf);
            break;
        default:
            throw StreamError("Can't read BMP from stream, unsupported DIB header");
    }

    /* Read pixel data */
    const uint32_t absHeight = abs(height);
    auto ptrPixdata = istream.read<PixdataPtr>(imgSize);
    if (bmpHasPad(width, cf.bpp)) {
        ptrPixdata = bmpRemovePad(ptrPixdata, width, absHeight, cf.bpp);
    }

    /* Flip image if bottom-up to top-down */
    if (height > 0) {
        pixdataFlipOverY(cf.bpp, width, absHeight, *ptrPixdata);
    }

    /* Create Texture */
    return Texture(width, absHeight, /*mipLevels=*/ 1, cf, ptrPixdata);
}

template<typename InfoHeaderT,
        typename = std::enable_if_t<std::is_base_of_v<BitmapInfoHeader, InfoHeaderT>>>
void bmpWriteFileHeader(OutputStream& ostream, uint32_t pixdataSize)
{
    auto header = bmpMakeFileHeader<InfoHeaderT>(pixdataSize);
    ostream.write(header);
}

template<typename InfoHeaderT,
        typename = std::enable_if_t<std::is_base_of_v<BitmapInfoHeader, InfoHeaderT>>>
void bmpWriteInfoHeader(OutputStream& ostream, uint32_t width, int32_t height, const ColorFormat& ci, uint32_t pixdataSize)
{
    auto info = bmpMakeInfoHeader<InfoHeaderT>(width, height, ci, pixdataSize);
    ostream.write(info);
}

void libim::content::asset::bmpWriteTexture(OutputStream& ostream, const TextureView& texView)
{
    if (texView.isEmpty()) {
        throw std::invalid_argument("Can't write empty texture as BMP file format to stream");
    }

    const auto& cf   = texView.format();
    auto ptrPixdata  = texView.pixdata(); // Should return pixel data of top image at LOD 0
    auto pixdataSize = safe_cast<uint32_t>(ptrPixdata->size());
    assert( calcPixdataSize(texView.width(), texView.height(), cf) == pixdataSize );
    if (bmpHasPad(texView.width(), cf.bpp))
    {
        ptrPixdata  = bmpAddPad(ptrPixdata, texView.width(), texView.height(), cf.bpp);
        pixdataSize = safe_cast<uint32_t>(ptrPixdata->size()); // Re-set image size to new size
    }

    auto height = - safe_cast<int32_t>(texView.height()); //indicate image is top-down
    if (cf == RGB24 || cf == RGB555)
    {
        bmpWriteFileHeader<BitmapInfoHeader>(ostream, pixdataSize);
        bmpWriteInfoHeader<BitmapInfoHeader>(ostream, texView.width(), height, cf, pixdataSize);
    }
    else
    {
        bmpWriteFileHeader<BitmapV4Header>(ostream, pixdataSize);
        bmpWriteInfoHeader<BitmapV4Header>(ostream, texView.width(), height, cf, pixdataSize);
    }

    ostream.write(reinterpret_cast<const byte_t*>(ptrPixdata->data()), pixdataSize);
    ostream.flush();
}

Pixdata::const_iterator
libim::content::asset::copyTextureFromPixdata(Pixdata::const_iterator first, Pixdata::const_iterator last, uint32_t width, uint32_t height, uint32_t mipLevels, const ColorFormat& colorInfo, Texture& tex)
{
    const auto mipSize = calcMipmapSize(width, height, mipLevels, colorInfo);
    if (std::distance(first, last) < mipSize) {
        throw std::out_of_range("Can't copy Texture from pixel data. Range out of bounds");
    }

    const auto ptrPixData = makePixdataPtr(mipSize);
    auto itSrcEnd         = std::next(first, mipSize);
    std::copy(first, itSrcEnd, ptrPixData->begin());

    tex = Texture(width, height, mipLevels, colorInfo, std::move(ptrPixData));
    return itSrcEnd;
}

void libim::content::asset::convertPixdataRow(const byte_t* pRowSrc, uint32_t rowLenSrc, const ColorFormat& ciSrc,
    byte_t* pRowDest, uint32_t rowLenDest, const ColorFormat& ciDest)
{
    const auto pixelSizeSrc  = bbs(ciSrc.bpp);
    const auto pixelSizeDest = bbs(ciDest.bpp);
    assert(pixelSizeSrc > 1 && pixelSizeSrc <= 4);
    assert(pixelSizeDest > 1 && pixelSizeDest <= 4);

    const double psr = pixelSizeDest / double(pixelSizeSrc); // pixel size ratio
    for (uint32_t colSrc = 0; colSrc < rowLenSrc; colSrc += pixelSizeSrc)
    {
        Color pixel = readPixel(pRowSrc + colSrc, rowLenSrc - colSrc, ciSrc);
        const uint32_t destCol = colSrc * psr;
        writePixel(pixel, pRowDest + destCol, rowLenDest - destCol, ciDest);
    }
}

PixdataPtr libim::content::asset::convertPixdata(PixdataPtr ptrPixdataSrc, uint32_t width, uint32_t height, const ColorFormat& from, const ColorFormat& to)
{
    if (from == to) {
        return ptrPixdataSrc;
    }

    auto itBegin = ptrPixdataSrc->begin();
    auto itEnd   = std::next(itBegin, calcStride(width, from) * height);
    return convertPixdata(itBegin, itEnd, width, height, from, to);
}

PixdataPtr libim::content::asset::convertPixdata(Pixdata::const_iterator itSrcFirst, Pixdata::const_iterator itSrcLast, uint32_t width, uint32_t height, const ColorFormat& from, const ColorFormat& to)
{
    auto strideSrc  = calcStride(width, from);
    if (std::distance(itSrcFirst, itSrcLast) != strideSrc * height) {
        throw std::runtime_error("Can't convert pixdata invalid src iterators");
    }

    if (from.bpp % 8 != 0 || from.bpp < 16 || from.bpp > 32) {
        throw std::runtime_error("Can't convert pixdata invalid BPP of src");
    }

    if (to.bpp % 8 != 0 || to.bpp < 16 || to.bpp > 32) {
        throw std::runtime_error("Can't convert pixdata invalid BPP of dest color format");
    }

    auto strideDest     = calcStride(width, to);
    auto ptrPixdataDest = makePixdataPtr(strideDest * height);
    auto itDest         = ptrPixdataDest->begin();
    while (itSrcFirst != itSrcLast)
    {
        convertPixdataRow(
            &(*itSrcFirst), strideSrc , from,
            &(*itDest)    , strideDest, to
        );
        std::advance(itSrcFirst, strideSrc);
        std::advance(itDest, strideDest);
    }
    return ptrPixdataDest;
}

void libim::content::asset::boxFilterScale(Pixdata::const_iterator itSrc, uint32_t srcWidth, uint32_t srcHeight, Pixdata::iterator itDest, uint32_t destWidth, uint32_t destHeight, const ColorFormat& cf, bool sRGB)
{
    const float idw = 1 / static_cast<float>(destWidth);
    const float idh = 1 / static_cast<float>(destHeight);
    for (uint32_t y = 0; y < destHeight; y++)
    {
        const uint32_t gy  = (y * idh) * srcHeight;
        const uint32_t gy1 = min(gy + 1, srcHeight - 1);
        for (uint32_t x = 0; x < destWidth; x++)
        {
            const uint32_t gx  = (x * idw) * srcWidth;
            const uint32_t gx1 = min(gx + 1, srcWidth - 1);

            LinearColor px00 = makeLinearColor(readPixelAt(itSrc, gx , gy , srcWidth, srcHeight, cf), sRGB);
            LinearColor px10 = makeLinearColor(readPixelAt(itSrc, gx1, gy , srcWidth, srcHeight, cf), sRGB);
            LinearColor px01 = makeLinearColor(readPixelAt(itSrc, gx , gy1, srcWidth, srcHeight, cf), sRGB);
            LinearColor px11 = makeLinearColor(readPixelAt(itSrc, gx1, gy1, srcWidth, srcHeight, cf), sRGB);

            const auto pxd = (px00 + px01 + px10 + px11) * 0.25f; // pixel averaging
            writePixelAt(makeColor(pxd, sRGB), itDest, x, y, destWidth, destHeight, cf);
        }
    }
}