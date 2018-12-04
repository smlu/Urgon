#ifndef LIBIM_BMP_H
#define LIBIM_BMP_H
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../../../common.h"
#include "../../../log/log.h"
#include "../../../io/filestream.h"

namespace libim::content::asset {

    constexpr uint16_t BMP_TYPE = 0x4D42;

#ifdef OS_WINDOWS
    using BV5Compression_t = uint32_t;
#else
    enum BV5Compression : uint32_t
    {
        BI_RGB            = 0,
        BI_RLE8           = 1,
        BI_RLE4           = 2,
        BI_BITFIELDS      = 3,
        BI_JPEG           = 4,
        BI_PNG            = 5,
        BI_ALPHABITFIELDS = 6
    };
    using BV5Compression_t = BV5Compression;
#endif

    PACKED(
    typedef struct {
        uint16_t   type;
        uint32_t   size;
        uint16_t   reserved1;
        uint16_t   reserved2;
        uint32_t   offBits;
    }) BitmapFileHeader;

    typedef struct
    {
        int32_t ciexyzX;
        int32_t ciexyzY;
        int32_t ciexyzZ;
    } Ciexyz;

    PACKED(
    typedef struct {
      uint32_t        size;
      int32_t         width;
      int32_t         height;
      uint16_t        planes;
      uint16_t        bitCount;
      BV5Compression_t  compression;
      uint32_t        sizeImage;
      int32_t         X_PelsPerMeter;
      int32_t         Y_PelsPerMeter;
      uint32_t        colorUsed;
      uint32_t        colorImportant;
      uint32_t        redMask;
      uint32_t        greenMask;
      uint32_t        blueMask;
      uint32_t        alphaMask;
      uint32_t        csType;
      struct {
        Ciexyz red;
        Ciexyz green;
        Ciexyz blue;
      } endpoints;
      uint32_t        gammaRed;
      uint32_t        gammaGreen;
      uint32_t        gammaBlue;
      uint32_t        intent;
      uint32_t        profileData;
      uint32_t        profileSize;
      uint32_t        reserved;
    }) BitmapV5Header;


    typedef struct {
        BitmapFileHeader header{};
        BitmapV5Header info{};
        BitmapPtr pixelData;
    } Bmp;


    //std::shared_ptr<Bmp> LoadBmpFromFile(const std::string& filename)
    //{
    //    Bmp bmp;

    //    std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary);
    //    if (!ifs.is_open())
    //    {
    //        std::cerr << "Error opening file: " << strerror(errno) << std::endl;
    //        return 1;
    //    }

    //    /* Read header */
    //    if(!ifs.read(reinterpret_cast<char*>(&bmp.header), sizeof(bmp.header)))
    //    {
    //        std::cerr << "Error reading bmp header: " << strerror(errno) << std::endl;
    //        return 1;
    //    }

    //    /* Read bmp info header */
    //    if(!ifs.read(reinterpret_cast<char*>(&bmp.info), sizeof(bmp.info)))
    //    {
    //        std::cerr << "Error reading bmp info header: " << strerror(errno) << std::endl;
    //        return 1;
    //    }

    //    /* Check bmp type */
    //    if(bmp.header.type != BMP_TYPE)
    //    {
    //        std::cerr << "Error not a bitmap file!\n";
    //        return 2;
    //    }

    //    /* Check taht we have v5 info header */
    //    if(bmp.info.size != sizeof(BitmapV5Header))
    //    {
    //        std::cerr << "No v5 bitmap header found!\n";
    //        return 2;
    //    }
    //}

    static bool SaveBmpToFile(const std::string& filename, const Bmp& bmp)
    {
        try
        {
            OutputFileStream ofs(filename);
            ofs.write(reinterpret_cast<const byte_t*>(&bmp.header), sizeof(bmp.header));
            ofs.write(reinterpret_cast<const byte_t*>(&bmp.info),   sizeof(bmp.info));
            ofs.write(reinterpret_cast<const byte_t*>(bmp.pixelData->data()), bmp.pixelData->size());

            ofs.close();
            return true;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("An exception was thrwon while writing BMP to file: %", e.what());
            return false;
        }
    }

}
#endif // LIBIM_BMP_H
