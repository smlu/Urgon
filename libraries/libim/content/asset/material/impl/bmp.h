#ifndef LIBIM_BMP_H
#define LIBIM_BMP_H
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>

#include "../texture.h"
#include "../texture_view.h"

#include <libim/common.h>
#include <libim/log/log.h>
#include <libim/io/filestream.h>

namespace libim::content::asset {

    constexpr uint16_t BMP_TYPE = 0x4D42;

#ifdef OS_WINDOWS
    using BiCompression = uint32_t;
    using LcsType = uint32_t;
#else
    enum BiCompressionEnum : uint32_t
    {
        BI_RGB            = 0,
        BI_RLE8           = 1,
        BI_RLE4           = 2,
        BI_BITFIELDS      = 3,
        BI_JPEG           = 4,
        BI_PNG            = 5
    };
    using BiCompression = BiCompressionEnum;

    enum LcsTypeEnum : uint32_t // LogicalColorSpaceType
    {
        LCS_CALIBRATED_RGB      = 0x00000000,
        LCS_sRGB                = 0x73524742,
        LCS_WINDOWS_COLOR_SPACE = 0x57696E20
    };
    using LcsType = LcsTypeEnum;
#endif

    PACKED(struct BitmapFileHeader final
    {
        uint16_t   type;
        uint32_t   size;
        uint16_t   reserved1;
        uint16_t   reserved2;
        uint32_t   offBits;
    });
    static_assert(sizeof(BitmapFileHeader) == 14);

    struct Ciexyz
    {
        int32_t ciexyzX;
        int32_t ciexyzY;
        int32_t ciexyzZ;
    };

    PACKED(struct BitmapInfoHeader
    {
        uint32_t        size;
        int32_t         width;
        int32_t         height;
        uint16_t        planes;
        uint16_t        bpp;
        BiCompression   compression;
        uint32_t        sizeImage;
        int32_t         X_PelsPerMeter;
        int32_t         Y_PelsPerMeter;
        uint32_t        colorUsed;
        uint32_t        colorImportant;
    });
    static_assert(sizeof(BitmapInfoHeader) == 40);

    PACKED(struct BitmapV2Header : BitmapInfoHeader
    {
        uint32_t        redMask;
        uint32_t        greenMask;
        uint32_t        blueMask;
    });
    static_assert(sizeof(BitmapV2Header) == 52);

    PACKED(struct BitmapV3Header : BitmapV2Header
    {
        uint32_t        alphaMask;
    });
    static_assert(sizeof(BitmapV3Header) == 56);

    PACKED(struct BitmapV4Header : BitmapV3Header
    {
        LcsType         csType;
        struct {
            Ciexyz red;
            Ciexyz green;
            Ciexyz blue;
        } endpoints;
        uint32_t        gammaRed;
        uint32_t        gammaGreen;
        uint32_t        gammaBlue;
    });
    static_assert(sizeof(BitmapV4Header) == 108);

    PACKED(struct BitmapV5Header final : BitmapV4Header
    {
        uint32_t        intent;
        uint32_t        profileData;
        uint32_t        profileSize;
        uint32_t        reserved;
    });
    static_assert(sizeof(BitmapV5Header) == 124);


    inline std::string bmpType2Str(uint16_t type)
    {
        type = (type>>8) | (type<<8); // swap
        return  reinterpret_cast<const char*>(&type);
    }

    inline std::string bmpCompressType2Str(BiCompression type)
    {
        switch(type)
        {
            case BI_RGB:            return "BI_RGB";
            case BI_RLE8:           return "BI_RLE8";
            case BI_RLE4:           return "BI_RLE4";
            case BI_BITFIELDS:      return "BI_BITFIELDS";
            case BI_JPEG:           return "BI_JPEG";
            case BI_PNG:            return "BI_PNG";
        }
        return "";
    }

    template<typename InfoHeaderT,
        typename = std::enable_if_t<std::is_base_of_v<BitmapInfoHeader, InfoHeaderT>>>
    inline BitmapFileHeader bmpMakeFileHeader(uint32_t pixdataSize)
    {
        BitmapFileHeader header {};
        header.type      = BMP_TYPE;
        header.offBits   = sizeof(BitmapFileHeader) + sizeof(InfoHeaderT);
        header.size      = header.offBits + pixdataSize;
        return header;
    }

    template<typename InfoHeaderT,
        typename = std::enable_if_t<std::is_base_of_v<BitmapInfoHeader, InfoHeaderT>>>
    constexpr inline InfoHeaderT bmpMakeInfoHeader(uint32_t width, int32_t height, const ColorFormat& ci, uint32_t pixdataSize)
    {
        InfoHeaderT info {};
        info.size   = sizeof(InfoHeaderT);
        info.width  = safe_cast<int32_t>(width);
        info.height = height;
        info.planes = 1;
        info.bpp    = ci.bpp;

        if (ci.mode == ColorMode::Indexed) {
            throw std::invalid_argument("Can't construct BitmapInfoHeader for index color mode.");
        }
        else if (ci == RGB555 || ci == RGB24)
        {
            info.compression = BI_RGB;
            info.sizeImage   = pixdataSize;
        }
        else
        {
            if constexpr (std::is_same_v<BitmapInfoHeader, InfoHeaderT>) {
                throw std::runtime_error("Can't construct BitmapInfoHeader with compression as BI_BITFIELDS");
            }
            else
            {
                info.compression = BI_BITFIELDS;
                info.sizeImage   = pixdataSize;
                info.redMask     = getColorShlMask(ci.redBPP  , ci.redShl);
                info.greenMask   = getColorShlMask(ci.greenBPP, ci.greenShl);
                info.blueMask    = getColorShlMask(ci.blueBPP , ci.blueShl);

                if constexpr (std::is_same_v<BitmapV2Header, InfoHeaderT>)
                {
                    if (ci.mode == ColorMode::RGBA) {
                        throw std::runtime_error("Can't construct BitmapV2Header with compression as BI_BITFIELDS because alphaMask is not supported.");
                    }
                }
                else {
                    info.alphaMask = getColorShlMask(ci.alphaBPP, ci.alphaShl);
                }
            }
        }

        // Set color space type in bitmap header of version 4 & 5
        if constexpr (std::is_same_v<BitmapV4Header, InfoHeaderT> ||
                      std::is_same_v<BitmapV5Header, InfoHeaderT>)
        {
            info.csType = LCS_sRGB;
        }

        return info;
    }

    template<typename InfoHeaderT,
    typename = std::enable_if_t<std::is_base_of_v<BitmapInfoHeader, InfoHeaderT>>>
    bool bmpInfoToColorFormat(const InfoHeaderT& info, ColorFormat& cf)
    {
        if (info.compression == BI_RGB)
        {
            switch (info.bpp)
            {
            case 16:
                cf = RGB555;
                return true;
            case 24:
                cf = RGB24;
                return true;
            case 32:
                cf = RGB24;
                cf.bpp = 32;
                return true;
            default: return false;
            }
        }
        else if (info.compression == BI_BITFIELDS)
        {
            if constexpr (!std::is_same_v<InfoHeaderT, BitmapInfoHeader>)
            {
                // Get mode, bits per color componet, color component left and right shift
                // Note: bmiColors field which follows BitmapInfoHeader should handled via BitmapV2Header struct.
                auto rbpc = getBPCFromMask(info.redMask);   auto rls = getLeftShiftPosFromMask(info.redMask);
                auto gbpc = getBPCFromMask(info.greenMask); auto gls = getLeftShiftPosFromMask(info.greenMask);
                auto bbpc = getBPCFromMask(info.blueMask);  auto bls = getLeftShiftPosFromMask(info.blueMask);

                uint32_t abpc = 0; uint32_t als = 0;
                if constexpr (!std::is_same_v<InfoHeaderT, BitmapV2Header>) // BitmapV3Header, BitmapV4Header, BitmapV5Header
                {
                    abpc = getBPCFromMask(info.alphaMask);
                    als  = abpc > 0 ? getLeftShiftPosFromMask(info.alphaMask) : 0;
                }

                cf.mode = abpc != 0 ? ColorMode::RGBA : ColorMode::RGB;
                cf.bpp  = info.bpp;

                // bit cont per each RGBA color component
                cf.redBPP   = rbpc;
                cf.greenBPP = gbpc;
                cf.blueBPP  = bbpc;
                cf.alphaBPP = abpc;

                // left shift position per each RGBA color component
                cf.redShl   = rls;
                cf.greenShl = gls;
                cf.blueShl  = bls;
                cf.alphaShl = als;

                // right shift position per each RGBA color component to convert decoded components to 8-bit values
                cf.redShr   = 8 - rbpc;
                cf.greenShr = 8 - gbpc;
                cf.blueShr  = 8 - bbpc;
                cf.alphaShr = abpc > 0 ? 8 - abpc : 0;

                return true;
            }
        }

        return false;
    }

    inline uint32_t bmpCalcPadSize(uint32_t width, uint32_t bpp)
    {
        #if _MSC_VER
            #pragma warning(push)
            #pragma warning(disable: 4146 )
        #endif

        uint32_t bps = bbs(bpp);
        return (-((bps % 4) * width)) & 3;

        #if _MSC_VER
            #pragma warning(pop)
        #endif
    }

    inline uint32_t bmpCalcPaddedRowLen(uint32_t width, uint32_t bpp) {
        return (width * bbs(bpp)) + bmpCalcPadSize(width, bpp);
    }

    inline bool bmpHasPad(uint32_t width, uint32_t bpp) {
        return bmpCalcPadSize(width, bpp) > 0;
    }

    static PixdataPtr bmpAddPad(PixdataPtr ptrPixdata, uint32_t width, uint32_t height, uint32_t bpp)
    {
        const std::size_t rowLen    = width * bbs(bpp);
        const std::size_t padRowLen = bmpCalcPaddedRowLen(width, bpp);
        if (padRowLen * height == ptrPixdata->size()) {
            return ptrPixdata;
        }
        assert(rowLen * height <= ptrPixdata->size());

        auto ptrPadPixdata = makePixdataPtr(padRowLen * height);
        for (std::size_t r = 0; r < height; r++) {
            memcpy(&ptrPadPixdata->at(r * padRowLen), &ptrPixdata->at(r * rowLen), rowLen);
        }
        return ptrPadPixdata;
    }

    static PixdataPtr bmpRemovePad(PixdataPtr ptrPadPixdata, uint32_t width, uint32_t height, uint32_t bpp)
    {
        const std::size_t rowLen    = width * bbs(bpp);
        const std::size_t padRowLen = bmpCalcPaddedRowLen(width, bpp);
        if (rowLen * height == ptrPadPixdata->size()) {
            return ptrPadPixdata;
        }
        assert(padRowLen * height == ptrPadPixdata->size());

        auto ptrPixdata = makePixdataPtr(rowLen * height);
        for (std::size_t r = 0; r < height; r++) {
            memcpy(&ptrPixdata->at(r * rowLen), &ptrPadPixdata->at(r * padRowLen), rowLen);
        }
        return ptrPixdata;
    }
}
#endif // LIBIM_BMP_H
