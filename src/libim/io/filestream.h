#ifndef LIBIM_FILESTREAM_H
#define LIBIM_FILESTREAM_H
#include "stream.h"
#include "../common.h"

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace libim {
    class FileStream : public virtual Stream
    {
    public:
        enum Mode
        {
            Read,
            Write,
            ReadWrite
        };

        explicit FileStream(std::string filePath, Mode mode = ReadWrite);
        explicit FileStream(std::string filePath, bool truncate, Mode mode = ReadWrite);
        explicit FileStream(const std::filesystem::path& filePath, Mode mode = ReadWrite);
        explicit FileStream(const std::filesystem::path& filePath, bool truncate, Mode mode = ReadWrite);
        virtual ~FileStream() override;

        virtual void seek(std::size_t position) const override;
        virtual std::size_t size() const override;
        virtual std::size_t tell() const override;
        virtual bool canRead() const override;
        virtual bool canWrite() const override;
        virtual void close();

    protected:
        virtual std::size_t readsome(byte_t* data, std::size_t length) const override;
        virtual std::size_t writesome(const byte_t* data, std::size_t length) override;

    private:
        struct FileStreamImpl;
        std::shared_ptr<FileStreamImpl> m_fs;
    };

    class InputFileStream final : public InputStream, public FileStream
    {
    public:
        explicit InputFileStream(std::string filePath) :
            FileStream(std::move(filePath), Read)
        {}

        InputFileStream(const std::filesystem::path& filePath) :
            FileStream(filePath, Read)
        {}

    private:
        using FileStream::write;
    };

    class OutputFileStream final : public OutputStream, public FileStream
    {
    public:
        explicit OutputFileStream(std::string filePath, const bool truncate = false) :
            FileStream(std::move(filePath), truncate, Write)
        {}

        OutputFileStream(const std::filesystem::path& filePath, const bool truncate = false) :
            FileStream(filePath, truncate, Write)
        {}

    private:
        using FileStream::read;
    };
}

#endif // LIBIM_FILESTREAM_H
