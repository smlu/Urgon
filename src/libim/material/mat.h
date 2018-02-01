#ifndef LIBIM_MAT_H
#define LIBIM_MAT_H
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <utility>

#include "bmp.h"
#include "../io/filestream.h"
#include "material.h"
#include "colorformat.h"


constexpr std::array<char,4> MAT_FILE_SIG    = {{'M', 'A', 'T', ' '}};
constexpr uint32_t           MAT_VERSION     = 0x32;
constexpr uint32_t           MAT_MIPMAP_TYPE = 2;


struct MatHeader
{
    std::array<char,4> magic;     // Should be 'MAT '
    int version;                  // Should 0x32

    unsigned int type;           // 0 = color, 1 = ?, 2 = texture (Should be 2)
    int recordCount;             // Number of MAT records
    int mipmapCount;             // Number of mipmaps

    ColorFormat colorInfo;
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


/* Stream class template function specialization */
template<> inline MatHeader Stream::read<MatHeader>() const
{
    MatHeader matHeader;
    auto nRead = this->readsome(reinterpret_cast<byte_t*>(&matHeader), sizeof(MatHeader));
    if(nRead != sizeof(MatHeader)) {
        throw StreamError("Error reading MatHeader from stream!");
    }

    /* Verify file signature */
    if(matHeader.magic != MAT_FILE_SIG) {
        throw StreamError("Unknown MAT file");
    }

    /* Verify file version */
    if(matHeader.version != MAT_VERSION) {
        throw StreamError(std::string("Wrong MAT file version: ") + std::to_string(matHeader.version));
    }

    return matHeader;
}

template<> inline Stream& Stream::write(const MatHeader& matHeader)
{
    auto nWritten = this->write(reinterpret_cast<const byte_t*>(&matHeader), sizeof(MatHeader));
    if(nWritten != sizeof(MatHeader)) {
        throw StreamError("Error writing MatHeader to stream");
    }

    return *this;
}

template<> inline MatRecordHeader Stream::read<MatRecordHeader>() const
{
    MatRecordHeader matRecord;
    auto nRead = this->readsome(reinterpret_cast<byte_t*>(&matRecord), sizeof(MatRecordHeader));
    if(nRead != sizeof(MatRecordHeader)) {
        throw StreamError("Error reading MatRecordHeader from stream");
    }

    return matRecord;
}

template<> inline Stream& Stream::write(const MatRecordHeader& matRecord)
{
    auto nWritten = this->write(reinterpret_cast<const byte_t*>(&matRecord), sizeof(MatRecordHeader));
    if(nWritten != sizeof(MatRecordHeader)) {
        throw StreamError("Error writing MatRecordHeader to stream");
    }

    return *this;
}

template<> inline MatMipmapHeader Stream::read<MatMipmapHeader>() const
{
    MatMipmapHeader mipmapHeader;
    auto nRead = this->readsome(reinterpret_cast<byte_t*>(&mipmapHeader), sizeof(MatMipmapHeader));
    if(nRead != sizeof(MatMipmapHeader)) {
        throw StreamError("Error reading MatMipmapHeader from stream");
    }

    return mipmapHeader;
}

template<> inline Stream& Stream::write(const MatMipmapHeader& mipmapHeader)
{
    auto nWritten = this->write(reinterpret_cast<const byte_t*>(&mipmapHeader), sizeof(MatMipmapHeader));
    if(nWritten != sizeof(MatMipmapHeader)) {
        throw StreamError("Error writing MatMipmapHeader to stream");
    }

    return *this;
}



uint32_t MatTextureBitmapSize(const MatTexture& tex, uint32_t bpp)
{
    return GetBitmapSize(tex.header.height, tex.header.width, bpp);
}

//void UpdateAlpha(ColorFormat& colorInfo)
//{
//    auto alphaBpp = colorInfo.alphaBPP;
//    if(alphaBpp)
//    {
//        if(alphaBpp == 1){
//            memcpy(&colorInfo, &ARGB5551, sizeof(ColorFormat));
//        }
//        else{
//            memcpy(&colorInfo, &ARGB4444, sizeof(ColorFormat));
//        }
//    }
//}



std::shared_ptr<Material> LoadMaterialFromFile(const std::string& path)
{
    try
    {
        InputFileStream ifstream(path);

        /* Read header */
        auto header = ifstream.read<MatHeader>();

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
            std::cerr << "Error: MAT file record count == 0!\n";
            return nullptr;
        }

        /* Read Material records */
        auto records = ifstream.read<std::vector<MatRecordHeader>>(header.recordCount);

        /* Read mipmaps */
        std::vector<Mipmap> mipmaps(header.mipmapCount);
        for(auto& mipmap : mipmaps)
        {
            try
            {
                auto mmHeader = ifstream.read<MatMipmapHeader>();
                mipmap = ifstream.read<Mipmap,uint32_t, uint32_t, uint32_t, const ColorFormat&>(mmHeader.textureCount, mmHeader.width, mmHeader.height, header.colorInfo);
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error reading mipmap header and bitmap data from MAT file stream: " << e.what() << "!\n";
                return nullptr;
            }
        }

        auto mat = std::make_shared<Material>(ifstream.name());//GetFileNameFromPath(filepath) + ".mat");
        mat->setSize(mipmaps.at(0).at(0).width(), mipmaps.at(0).at(0).height());
        mat->setColorFormat(header.colorInfo);
        mat->setMipmaps(std::move(mipmaps));

        return mat;
    }
    catch (const std::exception& e)
    {
        std::cerr << "An error has occurred while loading material from MAT file stream: " << e.what() <<"!\n";
        return nullptr;
    }
}

static bool SaveMaterialToFile(std::string file, const Material& mat)
{
    if(mat.mipmaps().empty() || mat.mipmaps().at(0).empty()) {
        return false;
    }

    try
    {
        OutputFileStream ofstream(std::move(file));

        /* Write MAT header to file */
        MatHeader header{};
        header.magic       = MAT_FILE_SIG;
        header.version     = MAT_VERSION;
        header.type        = MAT_MIPMAP_TYPE;
        header.recordCount = mat.mipmaps().size();
        header.mipmapCount = mat.mipmaps().size();
        header.colorInfo   = mat.colorFormat();

        ofstream.write(header);

        /* Write record headers to file */
        MatRecordHeader record {};
        record.recordType = 8;

        for(uint32_t i = 0; i < header.recordCount; i++) {
            ofstream.write(record);
        }

        /* Write mipmaps to file */
        MatMipmapHeader mmHeader {};
        for(const auto& mipmap : mat.mipmaps())
        {
            /* Write mipmap header to fiel */
            mmHeader.width  = mipmap.at(0).width();
            mmHeader.height = mipmap.at(0).height();
            mmHeader.textureCount = mipmap.size();

            ofstream.write(mmHeader);

            /* Write mipmap's textures to file */
            for(const auto& tex : mipmap) {
                ofstream.write(tex.bitmap());
            }
        }

        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "An error has occurred while writing material to file stream: " << e.what() << "!\n";
        return false;
    }
}

#endif // LIBIM_MAT_H
