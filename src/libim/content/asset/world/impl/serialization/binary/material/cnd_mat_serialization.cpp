#include "../cnd.h"
#include "cnd_mat_header.h"

using namespace libim;
using namespace libim::content::asset;


uint32_t CND::GetMatSectionOffset(const CndHeader& header)
{
    return sizeof(CndHeader) +
            header.worldSoundUnknown + // worldSoundUnknown = sound data size;
            48 * header.worldSounds  + // size of sound file header struct * number of sound files in cnd file
            4;                         // 4 = unknown 4 bytes
}

std::vector<Material> CND::ReadMaterials(const InputStream& istream)
{
    try
    {
        std::vector<Material> materials;

        /* Read cnd file header */
        auto cndHeader = LoadHeader(istream);

        /* Return if no materials are present in file*/
        if(cndHeader.numMaterials < 1)
        {
            std::cout << "CND Info: No materials found in CND file!\n";
            return materials;
        }

        /* Seek to materials position */
        istream.seek(GetMatSectionOffset(cndHeader));

        /* Read materials pixel data size */
        uint32_t nBitmapBuffSize = istream.read<uint32_t>();
        if(nBitmapBuffSize == 0)
        {
            std::cerr << "CND Warning: Read materials bitmap data size == 0!\n";
            return materials;
        }

        /* Read material header list from file stream */
        auto matHeaders = istream.read<std::vector<CndMatHeader>>(cndHeader.numMaterials);

        /* Read materials pixel data from file stream */
        Bitmap vecBitmapBuff = istream.read<Bitmap>(nBitmapBuffSize);

        /* Extract materials from pixel data buffer */
        for(auto&& matHeader : matHeaders)
        {
            if(matHeader.mipmapCount < 1 || matHeader.texturesPerMipmap < 1)
            {
                std::cerr << "CND Warning: No pixel data found for material: " << matHeader.name << std::endl;
                continue;
            }

            /* Verify material bitdepth */
            if(matHeader.colorInfo.bpp % 8 != 0) // TODO: check for 16 and 32 bbp
            {
                std::cerr << "CND Error: Cannot extract material " << matHeader.name << " from buffer. Wrong bitdepth size: " <<  matHeader.colorInfo.bpp << std::endl;
                materials.clear();
                return materials;
            }

            /* Read mipmaps from buffer */
            std::vector<Mipmap> mipmaps(matHeader.mipmapCount);
            for(auto&& mipmap : mipmaps) {
                mipmap = MoveMipmapFromBuffer(vecBitmapBuff, matHeader.texturesPerMipmap, matHeader.width, matHeader.height, matHeader.colorInfo);
            }

            /* Init new material */
            Material mat(matHeader.name);
            mat.setSize(matHeader.width, matHeader.height);
            mat.setColorFormat(matHeader.colorInfo);
            mat.setMipmaps(std::move(mipmaps));

            materials.emplace_back(std::move(mat));
        }

        if(!vecBitmapBuff.empty()) {
            std::cerr << "CND Warning: Not all bitmap data was copied from buffer!\n";
        }

        return materials;
    }
    catch(const std::exception& e)
    {
        std::cerr << "CND Error: An exception was thrown while loading material from CND file stream: " << e.what() << "!\n";
        return std::vector<Material> ();
    }
}

bool CND::ReplaceMaterial(const Material& mat, const std::string& cndFile)
{
    if(mat.mipmaps().empty() || mat.mipmaps().at(0).empty()) {
        return false;
    }

    try
    {
        InputFileStream ifstream(cndFile);

        /* Read cnd file header */
        auto cndHeader = LoadHeader(ifstream);

        /* If no materials are present in file, return */
        if(cndHeader.numMaterials < 1)
        {
            std::cout << "CND Info: No materials were found in CND file!\n";
            return false;
        }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Find material that is being patched
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /* Seek to material header list */
        const auto matListOffset = GetMatSectionOffset(cndHeader);
        ifstream.seek(matListOffset);

        /* Read size of raw data of all materials */
        uint32_t nBitmapBufSize = ifstream.read<uint32_t>();
        if(nBitmapBufSize == 0)
        {
            std::cerr << "CND Error: read materials bitmap data size == 0!\n";
            return false;
        }

        /* Read material header list */
        auto matHeaders = ifstream.read<std::vector<CndMatHeader>>(cndHeader.numMaterials);

        /* Get patching material pixel data offset and size */
        std::size_t replMatOff  = 0;
        uint32_t    replMatSize = 0;
        for(auto& matHeader : matHeaders)
        {
            if(matHeader.mipmapCount < 1 || matHeader.texturesPerMipmap < 1)
            {
                std::cerr << "CND Warning: No pixel data found for material: " << matHeader.name << std::endl;
                continue;
            }

            if(matHeader.colorInfo.bpp % 8 != 0) // TODO: validate that mat bitdepth is 16 or 32
            {
                std::cerr << "CND Error: Cannot extract material " << matHeader.name << " from buffer. Wrong bitdepth size: " <<  matHeader.colorInfo.bpp << std::endl;
                return false;
            }

            /* Do we have material header that is being patched? */
            if(matHeader.name == mat.name())
            {
                /* Calculate material's mipmap size */
                for(int32_t i = 0; i < matHeader.mipmapCount; i++){
                    replMatSize += GetMipmapPixelDataSize(matHeader.texturesPerMipmap, matHeader.width, matHeader.height, matHeader.colorInfo.bpp);
                }

                matHeader.width       = mat.width();
                matHeader.height      = mat.height();
                matHeader.colorInfo   = mat.colorFormat();
                matHeader.mipmapCount = mat.mipmaps().size();
                matHeader.texturesPerMipmap = mat.mipmaps().at(0).size();

                break;
            }

            for(int32_t i = 0; i < matHeader.mipmapCount; i++){
                replMatOff += GetMipmapPixelDataSize(matHeader.texturesPerMipmap, matHeader.width, matHeader.height, matHeader.colorInfo.bpp);
            }
        }

        /* Verify offset and size */
        if(replMatOff >=  nBitmapBufSize || replMatSize == 0)
        {
            std::cerr << "CND Error: Cannot replace material in cnd file: material not found!\n";
            return false;
        }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Patch cnd file
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Writting material list
        /* Get patching material pixel data size */
        uint32_t nPatchMatSize = 0;
        for(std::size_t i = 0; i < mat.mipmaps().size(); i++){
            nPatchMatSize += GetMipmapPixelDataSize(mat.mipmaps().at(i).size(), mat.width(), mat.height(), mat.colorFormat().bpp);
        }

        /* Calculate new bitmap buffer size */
        nBitmapBufSize = (nBitmapBufSize - replMatSize) + nPatchMatSize;


        /* Open output cnd file */
        const std::string patchedCndFile = cndFile + ".patched";
        OutputFileStream ofstream(patchedCndFile);

        /* Copy input cnd file to output stream until materials section */
        ofstream.write(ifstream, 0, matListOffset);
        //ifstream.seek(matListOffset); // Move istream cur forward

        /* Write new pixel data size */
        ofstream.write(nBitmapBufSize);

        /* Write material list headers */
        ofstream.write(matHeaders);

        /* Seek to begining of material list raw data */
        ifstream.seek(ifstream.tell() + sizeof(nBitmapBufSize) + cndHeader.numMaterials * sizeof(CndMatHeader));

        /* Write pixel data until offset of material that's being patched */
        ofstream.write(ifstream, ifstream.tell(), replMatOff);
        //ifstream.seek(ifstream.tell() + replMatOff); // Move istream cur forward

        /* Write new material data to output cnd file */
        for(const auto& mipmap : mat.mipmaps())
        {
            for(const auto& tex : mipmap) {
                ofstream.write(tex.bitmap());
            }
        }

        /* Write the rest of input cnd file to output cnd file */
        ofstream.write(ifstream, ifstream.tell() + replMatSize);

        /* Write new file size to the beginning of the output cnd file*/
        ofstream.seekBegin();
        ofstream.write(static_cast<uint32_t>(ofstream.size()));

        /* Close IO file stream */
        ifstream.close();
        ofstream.close();

        /* Rename patched file name to original name */
        RenameFile(patchedCndFile, cndFile);
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "CND Error: An error has occurred while loading material from MAT file stream: " << e.what() << "!\n";
        return false;
    }
}
