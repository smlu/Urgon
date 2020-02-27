#ifndef LIBIM_MATERIAL_SER_HELPERS_H
#define LIBIM_MATERIAL_SER_HELPERS_H
#include <cstdint>
#include <memory>
#include <utility>


#include "mat_structs.h"
#include "../../material.h"
#include "../../colorformat.h"
#include "../../../../../io/stream.h"

/* Defines stream io overloads to serialize mat structs from stream */

namespace libim {
    using namespace libim;
    using namespace libim::content::asset;


    // Mat header
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

    // Mat record header
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


    // Mat mipmap header
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
}

#endif // LIBIM_MATERIAL_SER_HELPERS_H
