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

        /**
         * Destructor
         * @note File stream is closed automatically.
         * @throw can throw if call to close() throws.
         *        See close()
         */
        virtual ~FileStream() override;

        /**
         * Move current reading or writing cursor position in file to new offset.
         *
         * @note When stream is writable it calls flush() before moving cursor.
         * @param offset - new cursor offset relative to beginning of the stream.
         * @throw FileStreamError - if call to flush() fails or
         *        unable to move cursor e.g. offset out of file size bounds.
        */
        virtual void seek(std::size_t offset) const override;

        virtual std::size_t size() const override;
        virtual std::size_t tell() const override;
        virtual bool canRead() const override;
        virtual bool canWrite() const override;

        /**
         * Closes file stream.
         * @note  Before file is closed the flush() member function is called.
         * @throw Can throw under a debugger on windows if CloseHandle fails.
         */
        virtual void close();

        /**
         * Writes data from output buffer to file
         * @throw FileStreamError if unable to write or flush data to file.
         */
        virtual void flush() override;

    protected:
        virtual std::size_t readsome(byte_t* data, std::size_t length) const override;
        virtual std::size_t writesome(const byte_t* data, std::size_t length) override;

    private:
        struct FileStreamImpl;
        std::shared_ptr<FileStreamImpl> m_fs;
    };


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

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
        using FileStream::flush;
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

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // LIBIM_FILESTREAM_H
