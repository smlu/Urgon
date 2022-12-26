#include "../cnd.h"
#include "cnd_mat_header.h"

#include <libim/content/asset/material/texture.h>
#include <libim/content/asset/material/texutils.h>
#include <libim/content/audio/impl/serialization/soundbank_serializer.h>

#include <cstring>
#include <string>

#include <libim/log/log.h>
#include <libim/types/safe_cast.h>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::audio;
using namespace libim::utils;
using namespace std::string_literals;


std::size_t CND::getOffset_Materials(const InputStream& istream)
{
    AT_SCOPE_EXIT([ &istream, off = istream.tell() ](){
        istream.seek(off);
    });

    istream.seek(CND::getOffset_Sounds());
    skipSerializedSoundBank(istream);
    return istream.tell();
}

IndexMap<Material> CND::parseSection_Materials(const InputStream& istream, const CndHeader& header)
{
    IndexMap<Material> materials;
    try
    {
        /* Return if no materials are present in file*/
        if (header.numMaterials < 1)
        {
            LOG_INFO("CND::parseSection_Materials(): No materials found!");
            return materials;
        }

        /* Read materials pixel data size */
        uint32_t nPixdataBuffSize = istream.read<uint32_t>();
        if (nPixdataBuffSize == 0){
            throw CNDError("parseSection_Materials", "Pixdata buffer data size == 0");
        }

        /* Read material header list from file stream */
        auto matHeaders = istream.read<std::vector<CndMatHeader>>(header.numMaterials);

        /* Read materials pixel data from file stream */
        Pixdata vecPixdataBuff = istream.read<Pixdata>(nPixdataBuffSize);
        auto itBuffer = vecPixdataBuff.cbegin();

        /* Extract materials from pixel data buffer */
        for (auto&& matHeader : matHeaders)
        {
            if (matHeader.celCount < 1 || matHeader.mipLevels < 1)
            {
                LOG_DEBUG("CND::parseSection_Materials(): Material '%' has no pixel data!", matHeader.name);
                continue;
            }

            /* Verify material bitdepth */
            if (matHeader.colorInfo.bpp % 8 != 0) // TODO: check for 16 and 32 bbp
            {
                throw CNDError("parseSection_Materials",
                    "Cannot extract material "s +  matHeader.name.toStdString() +
                    "from buffer. Wrong color depth: " + std::to_string(matHeader.colorInfo.bpp)
                );
            }

            /* Read cells from buffer */
            std::vector<Texture> textures(
                safe_cast<uint32_t>(matHeader.celCount)
            );
            for (auto&& texture : textures)
            {

                itBuffer = copyTextureFromPixdata(
                    itBuffer,
                    vecPixdataBuff.cend(),
                    safe_cast<uint32_t>(matHeader.width),
                    safe_cast<uint32_t>(matHeader.height),
                    safe_cast<uint32_t>(matHeader.mipLevels),
                    matHeader.colorInfo,
                    texture
                );
            }

            /* Init new material */
            Material mat(matHeader.name);
            mat.setCells(std::move(textures));
            materials.pushBack(matHeader.name, std::move(mat));
        }

        if (itBuffer != vecPixdataBuff.cend()) {
            throw CNDError("parseSection_Materials", "Not all pixel data was copied from buffer");
        }

        return materials;
    }
    catch (const CNDError&) { throw; }
    catch (const std::exception& e)
    {
        throw CNDError("parseSection_Materials",
            "An exception was encountered while parsing secion 'Materials': "s + e.what()
        );
    }
}

IndexMap<Material> CND::readMaterials(const InputStream& istream)
{
    auto cndHeader = readHeader(istream);
    istream.seek(getOffset_Materials(istream));
    return parseSection_Materials(istream, cndHeader);
}

void CND::writeSection_Materials(OutputStream& ostream, const IndexMap<Material>& materials)
{
    try
    {
        std::vector<CndMatHeader> cndHeaders;
        cndHeaders.reserve(materials.size());

        Pixdata pixdataBuf;
        for (const auto& mat : materials)
        {
            CndMatHeader h;
            if (!utils::strcpy(h.name, mat.name())) {
                throw CNDError("writeSection_Materials",
                    "Too long material name to copy it to CndMatHeader.name field"
                );
            }

            h.width        = mat.width();
            h.height       = mat.height();
            h.colorInfo    = mat.format();
            h.celCount = safe_cast<decltype(h.celCount)>(mat.cells().size());
            h.mipLevels    = safe_cast<decltype(h.mipLevels)>(mat.cells().at(0).mipLevels());
            cndHeaders.push_back(h);

            pixdataBuf.reserve(mat.cells().at(0).pixdata()->size() * mat.cells().size());
            for (const auto& tex : mat.cells()) {
                pixdataBuf.insert(pixdataBuf.end(), tex.pixdata()->begin(), tex.pixdata()->end());
            }
        }

        /* Write new pixel data size */
        ostream.write(safe_cast<uint32_t>(pixdataBuf.size()));

        /* Write material headers */
        ostream.write(cndHeaders);

        /* Write pixeldata buffer of all materials to stream */
        ostream.write(pixdataBuf);
    }
    catch (const CNDError&) { throw; }
    catch (const std::exception& e) {
        throw CNDError("writeSection_Materials",
            "An exception was encountered while writing section 'Materials': "s + e.what()
        );
    }
}