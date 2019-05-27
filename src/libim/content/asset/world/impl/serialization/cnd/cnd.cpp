#include "cnd.h"
#include "../world_ser_common.h"

#include <array>
#include<cstdint>

using namespace libim;
using namespace libim::content::asset;

static constexpr uint32_t FileVersion = 3;

CndHeader CND::LoadHeader(const InputStream& istream)
{
    istream.seek(0);
    CndHeader cndHeader = istream.read<CndHeader>();

    /* Verify file copyright notice  */
    if( !std::equal(cndHeader.copyright.begin(), cndHeader.copyright.end(), kFileCopyright.begin())) {
        throw StreamError("Error bad CND file copyright");
    }

    /* Verify file version */
    if(cndHeader.version != FileVersion) {
        throw StreamError("Error wrong CND file version: " + std::to_string(cndHeader.version));
    }

    return cndHeader;
}



//uint32_t libim::CND::GetMatSectionOffset(const CndHeader& header)
//{
//    return sizeof(CndHeader) +
//            header.worldSoundUnknown + // worldSoundUnknown = sound data size;
//            48 * header.worldSounds  + // size of sound file header struct * number of sound files in cnd file
//            4;                         // 4 = unknown 4 bytes
//}

//std::vector<Material> libim::CND::LoadMaterials(const InputStream& istream)
//{
//    try
//    {
//        std::vector<Material> materials;

//        /* Read cnd file header */
//        auto cndHeader = LoadHeader(istream);

//        /* Return if no materials are present in file*/
//        if(cndHeader.numMaterials < 1)
//        {
//            std::cout << "CND Info: No materials found in CND file!\n";
//            return materials;
//        }

//        /* Seek to materials position */
//        istream.seek(GetMatSectionOffset(cndHeader));

//        /* Read materials pixel data size */
//        uint32_t nBitmapBuffSize = istream.read<uint32_t>();
//        if(nBitmapBuffSize == 0)
//        {
//            std::cerr << "CND Warning: Read materials bitmap data size == 0!\n";
//            return materials;
//        }

//        /* Read material header list from file stream */
//        auto matHeaders = istream.read<std::vector<CndMatHeader>>(cndHeader.numMaterials);

//        /* Read materials pixel data from file stream */
//        Bitmap vecBitmapBuff = istream.read<Bitmap>(nBitmapBuffSize);

//        /* Extract materials from pixel data buffer */
//        for(auto&& matHeader : matHeaders)
//        {
//            if(matHeader.mipmapCount < 1 || matHeader.texturesPerMipmap < 1)
//            {
//                std::cerr << "CND Warning: No pixel data found for material: " << matHeader.name << std::endl;
//                continue;
//            }

//            /* Verify material bitdepth */
//            if(matHeader.colorInfo.bpp % 8 != 0) // TODO: check for 16 and 32 bbp
//            {
//                std::cerr << "CND Error: Cannot extract material " << matHeader.name << " from buffer. Wrong bitdepth size: " <<  matHeader.colorInfo.bpp << std::endl;
//                materials.clear();
//                return materials;
//            }

//            /* Read mipmaps from buffer */
//            std::vector<Mipmap> mipmaps(matHeader.mipmapCount);
//            for(auto&& mipmap : mipmaps) {
//                mipmap = MoveMipmapFromBuffer(vecBitmapBuff, matHeader.texturesPerMipmap, matHeader.width, matHeader.height, matHeader.colorInfo);
//            }

//            /* Init new material */
//            Material mat(matHeader.name);
//            mat.setSize(matHeader.width, matHeader.height);
//            mat.setColorFormat(matHeader.colorInfo);
//            mat.setMipmaps(std::move(mipmaps));

//            materials.emplace_back(std::move(mat));
//        }

//        if(!vecBitmapBuff.empty()) {
//            std::cerr << "CND Warning: Not all bitmap data was copied from buffer!\n";
//        }

//        return materials;
//    }
//    catch(const std::exception& e)
//    {
//        std::cerr << "CND Error: An exception was thrown while loading material from CND file stream: " << e.what() << "!\n";
//        return std::vector<Material> ();
//    }
//}



