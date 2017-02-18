#ifndef MAT_H
#define MAT_H
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <utility>

#include "bmp.h"
#include "material.h"
#include "colorformat.h"

constexpr std::array<char,4> MAT_FILE_SIG    = {'M', 'A', 'T', ' '};
constexpr uint32_t           MAT_VERSION     = 0x32;
constexpr uint32_t           MAT_MIPMAP_TYPE = 2;


struct MatHeader
{
    char magic[4];           // Should be 'MAT '
    int version;             // Should 0x32

    unsigned int type;       // 0 = color, 1 = ?, 2 = texture (Should be 2)
    int recordCount;        // Number of MAT records
    int mipmapCount;        // Number of mipmaps

    struct ColorFormat colorInfo;
};

struct MatRecordHeader
{
    int recordType;       // 0 = color, 8 = texture
    int transparentColor;
    int Unknown1;
    int Unknown2;
    int Unknown3;

    int Unknown4;

    int Unknown5;
    int Unknown6;
    int Unknown7;
    int Unknown8;
};

struct MatMipmapHeader
{
    int width;
    int height;
    int TransparentBool;

    int Unknown1;
    int Unknown2;

    int textureCount;
};

struct MatTexture
{
    MatMipmapHeader header;
    BitmapPtr bitmap;
};

struct MatColorHeader
{
    int RecordType;
    int ColorNum;
    float Unknown1;
    float Unknown2;
    float Unknown3;
    float Unknown4;
};

uint32_t MatTextureBitmapSize(const MatTexture& tex, uint32_t bpp)
{
    return GetBitmapSize(tex.header.height, tex.header.width, bpp);
}

void UpdateAlpha(ColorFormat& colorInfo)
{
    auto alphaBpp = colorInfo.alphaBPP;
    if(alphaBpp)
    {
        if(alphaBpp == 1){
            memcpy(&colorInfo, &ARGB5551, sizeof(ColorFormat));
        }
        else{
            memcpy(&colorInfo, &ARGB4444, sizeof(ColorFormat));
        }
    }
}



std::shared_ptr<Material> LoadMaterialFromFile(const std::string& filepath)
{
    MatHeader header;
    std::ifstream ifs(filepath, std::ios::in | std::ios::binary);
    if (!ifs.is_open())
    {
        std::cerr << "Error opening mat file for reading!\n";
        return nullptr;
    }

    /* Read header */
    if(!ifs.read(reinterpret_cast<char*>(&header), sizeof(header)))
    {
        std::cerr << "Error reading mat header: " << IosErrorStr(ifs) << "!\n";
        return nullptr;
    }

    /* Verify file signature */
    if(strncmp(header.magic, MAT_FILE_SIG.data(), MAT_FILE_SIG.size()))
    {
        std::cerr << "Error unknown MAT file!\n";
        return nullptr;
    }

    /* Verify file version */
    if(header.version != MAT_VERSION)
    {
        std::cerr << "Error wrong MAT file version: " << header.version << std::endl;
        return nullptr;
    }

    if(header.type != MAT_MIPMAP_TYPE)
    {
        std::cerr << "Error MAT file does not contain mipmaps!\n";
        return nullptr;
    }

    if(header.recordCount != header.mipmapCount)
    {
        std::cerr << "Error old MAT file!\n";
        return nullptr;
    }

    if(header.recordCount == 0)
    {
        std::cerr << "Error MAT's file record count == 0!\n";
        return nullptr;
    }

    /* Read Material records */
    std::vector<MatRecordHeader> records(header.recordCount);
    for(auto& record : records)
    {
        if(!ifs.read(reinterpret_cast<char*>(&record), sizeof(MatRecordHeader)))
        {
            std::cerr << "Error reading record from MAT file: " << IosErrorStr(ifs) << "!\n";
            return nullptr;
        }
    }

    /* Read mipmaps */
    std::vector<Mipmap> mipmaps(header.mipmapCount);
    for(auto& mipmap : mipmaps) // Mipmaps
    {
        MatMipmapHeader mmHeader{};
        if(!ifs.read(reinterpret_cast<char*>(&mmHeader), sizeof(MatMipmapHeader)))
        {
            std::cerr << "Error reading mipmap header from MAT file: "  << IosErrorStr(ifs) << "!\n";
            return nullptr;
        }

        uint32_t nBitmapBuffSize = GetMipmapPixelDataSize(mmHeader.textureCount, mmHeader.width, mmHeader.height, header.colorInfo.bpp);
        Bitmap bitmapBuffer(nBitmapBuffSize);
        if(!ifs.read(reinterpret_cast<char*>(&bitmapBuffer[0]), nBitmapBuffSize))
        {
            std::cerr << "Error reading mipmap pixel data from MAT file: " << IosErrorStr(ifs) << "!\n";
            return nullptr;
        }

        /* Copy mipmap textures from buffer */
        auto it = CopyMipmapFromBuffer(mipmap, bitmapBuffer, mmHeader.textureCount, mmHeader.width, mmHeader.height, header.colorInfo);
        if(it != bitmapBuffer.end()) {
            std::cout << "Warning: Not all data was copied from bitmap buffer!\n";
        }

        bitmapBuffer.erase(bitmapBuffer.begin(), it);
    }

    //UpdateAlpha(mat->header.colorInfo);

    auto mat = std::make_shared<Material>(GetFileNameFromPath(filepath) + ".mat");
    mat->setSize(mipmaps.at(0).at(0).width(), mipmaps.at(0).at(0).height());
    mat->setColorFormat(header.colorInfo);
    mat->setMipmaps(std::move(mipmaps));
    return mat;
}

static bool SaveMaterialToFile(const Material& mat, const std::string& filename)
{
    if(mat.mipmaps().empty() || mat.mipmaps().at(0).empty()) return false;

    std::ofstream ofs(filename, std::ios::out | std::ios::binary);
    if (!ofs.is_open())
    {
        std::cerr << "Error opening file stream for saving bmp!\n";
        return false;
    }

    /* Write MAT header to file */
    MatHeader header{};
    memcpy(&header.magic, MAT_FILE_SIG.data(), MAT_FILE_SIG.size());
    header.version  = MAT_VERSION;
    header.type     = MAT_MIPMAP_TYPE;
    header.recordCount = mat.mipmaps().size();
    header.mipmapCount = mat.mipmaps().size();
    header.colorInfo   = mat.colorFormat();

    if(!ofs.write(reinterpret_cast<const char*>(&header), sizeof(header)))
    {
        std::cerr << "Error writing MAT header to file: " << IosErrorStr(ofs) << "!\n";
        return false;
    }

    /* Write record headers to file */
    MatRecordHeader record {};
    record.recordType = 8;
    for(uint32_t i = 0; i <  header.recordCount; i++)
    {
        if(!ofs.write(reinterpret_cast<const char*>(&record), sizeof(record)))
        {
            std::cerr << "Error writing MAT record header to file: " << IosErrorStr(ofs) << "!\n";
            return false;
        }
    }

    /* Write mipmaps to file */
    MatMipmapHeader mmHeader {};
    for(const auto& mipmap : mat.mipmaps())
    {
        mmHeader.width = mipmap.at(0).width();
        mmHeader.height = mipmap.at(0).height();
        mmHeader.textureCount = mipmap.size();

        if(!ofs.write(reinterpret_cast<const char*>(&mmHeader), sizeof(mmHeader)))
        {
            std::cerr << "Error writing MAT mipmap header to file: " << IosErrorStr(ofs) << "!\n";
            return false;
        }

        /* Write mipmap's textures to file */
        for(const auto& tex : mipmap)
        {
            if(!ofs.write(reinterpret_cast<const char*>(tex.bitmap()->data()), tex.bitmap()->size()))
            {
                std::cerr << "Error writing MAT mipmap header to file: " << IosErrorStr(ofs) << "!\n";
                return false;
            }
        }
    }

    ofs.flush();
    ofs.close();
    return true;
}

#endif // MAT_H
