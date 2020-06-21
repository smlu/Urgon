#include "../cnd.h"
#include "../sound/cnd_sound_header.h"
#include "cnd_mat_header.h"

#include <cstring>
#include <strings.h>

#include <libim/log/log.h>
#include <libim/types/safe_cast.h>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;
using namespace std::string_literals;


std::size_t CND::getOffset_Materials(const InputStream& istream)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    istream.seek(sizeof(CndHeader));
    std::size_t numSoundHeaders = istream.read<uint32_t>();
    std::size_t sizeSoundData   = istream.read<uint32_t>();
    constexpr std::size_t sizeNextFileIdField = sizeof(uint32_t);

    return istream.tell() +
           numSoundHeaders * sizeof(CndSoundHeader) +
           sizeSoundData +
           sizeNextFileIdField;
}

HashMap<Material> CND::parseSection_Materials(const InputStream& istream, const CndHeader& header)
{
    HashMap<Material> materials;
    try
    {
        /* Return if no materials are present in file*/
        if(header.numMaterials < 1)
        {
            LOG_INFO("CND::parseSection_Materials(): No materials found!");
            return materials;
        }

        /* Read materials pixel data size */
        uint32_t nBitmapBuffSize = istream.read<uint32_t>();
        if(nBitmapBuffSize == 0){
            throw CNDError("parseSection_Materials", "Bitmap buffer data size == 0");
        }

        /* Read material header list from file stream */
        auto matHeaders = istream.read<std::vector<CndMatHeader>>(header.numMaterials);

        /* Read materials pixel data from file stream */
        Bitmap vecBitmapBuff = istream.read<Bitmap>(nBitmapBuffSize);

        /* Extract materials from pixel data buffer */
        for(auto&& matHeader : matHeaders)
        {
            if(matHeader.mipmapCount < 1 || matHeader.texturesPerMipmap < 1)
            {
                LOG_DEBUG("CND::parseSection_Materials(): Material '%' has no pixel data!", matHeader.name);
                continue;
            }

            /* Verify material bitdepth */
            if(matHeader.colorInfo.bpp % 8 != 0) // TODO: check for 16 and 32 bbp
            {
                throw CNDError("parseSection_Materials",
                    "Cannot extract material "s +  matHeader.name.toStdString() +
                    "from buffer. Wrong color depth: " + std::to_string(matHeader.colorInfo.bpp)
                );
            }

            /* Read mipmaps from buffer */
            std::vector<Mipmap> mipmaps(matHeader.mipmapCount);
            for(auto&& mipmap : mipmaps)
            {
                mipmap = moveMipmapFromBuffer(
                    vecBitmapBuff,
                    matHeader.texturesPerMipmap,
                    matHeader.width,
                    matHeader.height,
                    matHeader.colorInfo
                );
            }

            /* Init new material */
            Material mat(matHeader.name);
            mat.setSize(matHeader.width, matHeader.height);
            mat.setColorFormat(matHeader.colorInfo);
            mat.setMipmaps(std::move(mipmaps));

            materials.pushBack(matHeader.name, std::move(mat));
        }

        if(!vecBitmapBuff.empty()) {
            throw CNDError("parseSection_Materials", "Not all bitmap data was copied from buffer");
        }

        return materials;
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e)
    {
        throw CNDError("parseSection_Materials",
            "An exception was encountered while parsing secion 'Materials': "s + e.what()
        );
    }
}

HashMap<Material> CND::readMaterials(const InputStream& istream)
{
    auto cndHeader = readHeader(istream);
    istream.seek(getOffset_Materials(istream));
    return parseSection_Materials(istream, cndHeader);
}

void CND::writeSection_Materials(OutputStream& ostream, const HashMap<Material>& materials)
{
    try
    {
        std::vector<CndMatHeader> cndHeaders;
        cndHeaders.reserve(materials.size());

        Bitmap bitmaps;
        for(const auto& mat : materials)
        {
            CndMatHeader h;

            if(!utils::strcpy(h.name, mat.name())) {
                throw CNDError("writeSection_Materials",
                    "Too long material name to copy it to CndMatHeader.name field"
                );
            }

            h.width       = mat.width();
            h.height      = mat.height();
            h.colorInfo   = mat.colorFormat();
            h.mipmapCount = safe_cast<decltype(h.mipmapCount)>(mat.mipmaps().size());
            h.texturesPerMipmap = safe_cast<decltype(h.texturesPerMipmap)>(mat.mipmaps().at(0).size());
            cndHeaders.push_back(h);

            const std::size_t sizePixeldata = getMipmapPixelDataSize(h.texturesPerMipmap, h.width, h.height, h.colorInfo.bpp);
            bitmaps.reserve(sizePixeldata);
            for(const auto& mipmap : mat.mipmaps())
            {
                for(const auto& tex : mipmap)
                {
                    const Bitmap& bitmap = *tex.bitmap();
                    bitmaps.insert(bitmaps.end(), bitmap.begin(), bitmap.end());
                }
            }
        }

        /* Write new pixel data size */
        ostream.write(safe_cast<uint32_t>(bitmaps.size()));

        /* Write material headers */
        ostream.write(cndHeaders);

        /* Write pixeldata of all materials */
        ostream.write(bitmaps);
    }
    catch (const CNDError&) { throw; }
    catch(const std::exception& e) {
        throw CNDError("writeSection_Materials",
            "An exception was encountered while writing section 'Materials': "s + e.what()
        );
    }
}