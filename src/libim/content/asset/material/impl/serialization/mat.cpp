#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <utility>


#include "mat_structs.h"
#include "mat_ser_helpers.h"
#include "../../material.h"
#include "../../colorformat.h"
#include "../../../../../io/stream.h"


using namespace libim;
using namespace libim::content::asset;




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

Material& Material::deserialize(const InputStream&& istream)
{
    return deserialize(istream);
}

Material& Material::deserialize(const InputStream& istream)
{
    /* Read header */
    auto header = istream.read<MatHeader>();

    if(header.type != MAT_MIPMAP_TYPE) {
        throw StreamError("MAT file contains no mipmaps");
    }

    if(header.recordCount != header.mipmapCount) {
        throw StreamError("Cannot read older version of MAT file");
    }

    if(header.recordCount <= 0) {
        throw StreamError("MAT file record count <= 0");
    }

    if(header.colorInfo.bpp % 8 != 0) {
        throw StreamError("BPP % 8 != 0");
    }

    /* Read Material records */
    auto records = istream.read<std::vector<MatRecordHeader>>(header.recordCount);

    /* Read mipmaps */
    std::vector<Mipmap> mipmaps(static_cast<std::size_t>(header.mipmapCount));
    for(auto& mipmap : mipmaps)
    {
        auto mmHeader = istream.read<MatMipmapHeader>();
        mipmap = istream.read<Mipmap, uint32_t, uint32_t, uint32_t, const ColorFormat&>(
                    mmHeader.textureCount, mmHeader.width, mmHeader.height, header.colorInfo);
    }

    this->setName(GetFilename(istream.name()));
    this->setSize(mipmaps.at(0).at(0).width(), mipmaps.at(0).at(0).height());
    this->setColorFormat(header.colorInfo);
    this->setMipmaps(std::move(mipmaps));
    return *this;
}


bool Material::serialize(OutputStream&& ostream) const
{
    return serialize(ostream);
}

bool Material::serialize(OutputStream& ostream) const
{
    if(mipmaps().empty() || mipmaps().at(0).empty()) {
        return false;
    }

    /* Write MAT header to file */
    if(mipmaps().size() > std::numeric_limits<int32_t>::max()) {
        throw StreamError("Cannot write material sto stream, mipmaps().size() > std::numeric_limits<int32_t>::max()");
    }

    int32_t mimpamCount = static_cast<int32_t>(mipmaps().size());

    MatHeader header{};
    header.magic       = MAT_FILE_SIG;
    header.version     = MAT_VERSION;
    header.type        = MAT_MIPMAP_TYPE;
    header.recordCount = mimpamCount;
    header.mipmapCount = mimpamCount;
    header.colorInfo   = colorFormat();

    ostream.write(header);

    /* Write record headers to file */
    MatRecordHeader record {};
    record.recordType = 8;

    while(mimpamCount --> 0) {
        ostream.write(record);
    }
//    for(int32_t i = 0; i < mimpamCount; i++) {
//        ostream.write(record);
//    }

    /* Write mipmaps to file */
    MatMipmapHeader mmHeader {};
    for(const auto& mipmap : mipmaps())
    {
        /* Write mipmap header to fiel */
        mmHeader.width  = mipmap.at(0).width();
        mmHeader.height = mipmap.at(0).height();
        mmHeader.textureCount = mipmap.size();

        ostream.write(mmHeader);

        /* Write mipmap's textures to file */
        for(const auto& tex : mipmap) {
            ostream.write(tex.bitmap());
        }
    }

    return true;
}


