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
#include <libim/io/stream.h>
#include <libim/types/safe_cast.h>


using namespace libim;
using namespace libim::content::asset;


Material& Material::deserialize(InputStream&& istream)
{
    return deserialize(istream);
}

Material& Material::deserialize(const InputStream& istream)
{
    /* Read header */
    auto header = istream.read<MatHeader>();

    if (header.type != MAT_TEXTURE_TYPE) {
        throw StreamError("MAT file contains no textures");
    }

    if (header.recordCount != header.celCount) {
        throw StreamError("Cannot read older version of MAT file");
    }

    if (header.recordCount <= 0) {
        throw StreamError("MAT file record count <= 0");
    }

    if (header.colorInfo.mode < ColorMode::RGB ||
       header.colorInfo.mode > ColorMode::RGBA) {
        throw StreamError("Can't read MAT file from stream, invalid color mode");
    }

    if (header.colorInfo.bpp % 8 != 0) {
        throw StreamError("Can't read MAT file from stream, BPP % 8 != 0");
    }

    if (header.colorInfo.bpp < 16 || header.colorInfo.bpp > 32) {
        throw StreamError("Can't read MAT file from stream, invalid BPP");
    }

    /* Read Material records */
    auto records = istream.read<std::vector<MatRecordHeader>>(
        static_cast<std::size_t>(header.recordCount)
    );

    /* Read textures */
    const auto celCount = safe_cast<std::size_t>(header.celCount);
    for (std::size_t i = 0; i < celCount; i++)
    {
        auto texHeader = istream.read<MatTextureHeader>();
        auto tex = istream.read<Texture, uint32_t, uint32_t, uint32_t, const ColorFormat&>(
            safe_cast<uint32_t>(texHeader.width),
            safe_cast<uint32_t>(texHeader.height),
            safe_cast<uint32_t>(texHeader.mipLevels),
            header.colorInfo
        );
        this->addCel(std::move(tex));
    }

    this->setName(getFilename(istream.name()));
    return *this;
}


bool Material::serialize(OutputStream&& ostream) const
{
    return serialize(ostream);
}

bool Material::serialize(OutputStream& ostream) const
{
    if (cells().empty()) {
        return false;
    }

    /* Write MAT header to file */
    int32_t celCount = safe_cast<int32_t>(count());

    MatHeader header{};
    header.magic        = MAT_FILE_SIG;
    header.version      = MAT_VERSION;
    header.type         = MAT_TEXTURE_TYPE;
    header.recordCount  = celCount;
    header.celCount     = celCount;
    header.colorInfo    = format();

    ostream.write(header);

    /* Write record headers to file */
    MatRecordHeader record {};
    record.recordType = 8;
    record.texIdx = 0;
    while (celCount --> 0)
    {
        ostream.write(record);
        record.texIdx++;
    }

    /* Write textures to stream */
    MatTextureHeader texHeader {};
    for (const auto& tex : cells())
    {
        /* Write texture header & mipmap pixdata to stream */
        texHeader.width     = safe_cast<int32_t>(tex.width());
        texHeader.height    = safe_cast<int32_t>(tex.height());
        texHeader.mipLevels = safe_cast<int32_t>(tex.mipLevels());

        ostream.write(texHeader);
        ostream.write(tex.pixdata());
    }

    ostream.flush();
    return true;
}