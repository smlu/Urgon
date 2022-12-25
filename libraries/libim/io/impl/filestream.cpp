#include "iobuffer.h"

#include <libim/common.h>
#include <libim/io/filestream.h>
#include <libim/io/binarystream.h>
#include <libim/platform.h>
#include <libim/types/safe_cast.h>
#include <libim/utils/utils.h>

#include <algorithm>
#include <filesystem>
#include <iterator>

#ifdef LIBIM_OS_WINDOWS
# include <windows.h>
# include <locale>
# include <codecvt>
# include <string>
# ifdef LIBIM_PLATFORM_64BIT
    typedef int64_t ssize_t;
# else
    typedef int32_t ssize_t;
# endif
#else
# include <assert.h>
# include <cstring>
# include <errno.h>
# include <fcntl.h>
# include <string.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
#endif

using namespace libim;

constexpr std::size_t kBufferSize = 4096;

#define MAX_WRITE_FILE_SIZE 1'000'000'000 // 1GB


std::string getLastErrorAsString()
{
    std::string message;
#ifdef LIBIM_OS_WINDOWS
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                    nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);

    message = std::string(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);
#else
    message = strerror(errno);
#endif
    return message;
}

struct FileStream::FileStreamImpl
{
     FileStreamImpl(std::string fp, bool truncate, Mode mode) :
        mode(mode),
        filePath(std::move(fp))
    {
        auto flags = [&]()
        {
            switch (mode)
            {
            #ifdef LIBIM_OS_WINDOWS
                case Read:      return GENERIC_READ;
                case Write:     return static_cast<DWORD>(GENERIC_WRITE);
                case ReadWrite: return static_cast<DWORD>(GENERIC_WRITE | GENERIC_READ);
            default:
                return static_cast<DWORD>(-1);
            #else
                case Read:      return O_RDONLY;
                case Write:     return O_WRONLY | O_CREAT;
                case ReadWrite: return O_RDWR   | O_CREAT;
            default:
                return -1;
            #endif
            }
        }();

        if(flags == static_cast<decltype(flags)>(-1)) {
            throw FileStreamError("Unknown file open mode!");
        }

    #ifdef LIBIM_OS_WINDOWS
        /* Open file
           Since obuffer is used the write operations could be also done without buffering.
           To do this add flags FILE_FLAG_NO_BUFFERING and FILE_FLAG_WRITE_THROUGH
           to flag param of function CreateFileX.
           See: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-flushfilebuffers#remarks
        */
        DWORD dwCreationDisposition = flags == GENERIC_READ ? OPEN_EXISTING : OPEN_ALWAYS;
        #if _WIN32_WINNT >= _WIN32_WINNT_WIN8
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wPath = converter.from_bytes(filePath.c_str());
            hFile = CreateFile2(
                        wPath.c_str(),
                        flags,
                        FILE_SHARE_READ,
                        dwCreationDisposition,
                        nullptr
                    );
        #else
            hFile = CreateFileA(
                        filePath.c_str(),
                        flags,
                        FILE_SHARE_READ,
                        nullptr,
                        dwCreationDisposition,
                        FILE_ATTRIBUTE_NORMAL,
                        nullptr
                    );
        #endif

        if (hFile == INVALID_HANDLE_VALUE) {
            throw FileStreamError(
                utils::format("Failed to open file %: %",  filePath, getLastErrorAsString())
            );
        }

        /* Truncate file if writable */
        if ((flags & GENERIC_WRITE) && truncate && !SetEndOfFile(hFile)) {
            throw FileStreamError(
                utils::format("Failed to to truncate the file %: %",  filePath, getLastErrorAsString())
            );
        }

        /* Get file size */
        LARGE_INTEGER lSize {{0, 0}};
        if (!GetFileSizeEx(hFile, &lSize)) {
            throw FileStreamError(
                utils::format("Failed to get the size of file %: %",  filePath, getLastErrorAsString())
            );
        }

        #ifdef LIBIM_PLATFORM_64BIT
            fileSize = lSize.QuadPart;
        #else
            fileSize = lSize.LowPart;
        #endif
    #else // Unix
        /* Open file */
        if (mode != Read && truncate) {
            flags |= O_TRUNC;
        }

        fd = open(filePath.c_str(), flags, (mode_t)0600);
        if (fd == -1) {
            throw FileStreamError(
                utils::format("Failed to open file %: %",  filePath, strerror(errno))
            );
        }

        /* Get file size */
        struct stat fileInfo {};
        if (fstat(fd, &fileInfo) == -1) {
            throw FileStreamError(
                utils::format("Failed to get the size of file %: %",  filePath, strerror(errno))
            );
        }

        fileSize = fileInfo.st_size;
    #endif
    }

    std::size_t read(byte_t* data, std::size_t length)
    {
        ssize_t nRead = 0;
    #ifdef LIBIM_OS_WINDOWS
        if(!ReadFile(hFile, reinterpret_cast<LPVOID>(data), safe_cast<DWORD>(length), reinterpret_cast<LPDWORD>(&nRead), nullptr)) {
    #else
        nRead = ::read(fd, data, length);
        if(nRead == -1) {
    #endif
            throw FileStreamError("Failed to read from file: " + getLastErrorAsString());
        }

        currentOffset += nRead;
        return static_cast<std::size_t>(nRead);
    }

    std::size_t flush(bool sync)
    {
        ssize_t nWritten = 0;
        if(mode == Write || mode == ReadWrite)
        {
            if(obuffer_.hasData())
            {
            #ifdef LIBIM_OS_WINDOWS
                if (!WriteFile(
                        hFile,
                        reinterpret_cast<LPCVOID>(obuffer_.data()),
                        static_cast<DWORD>(obuffer_.size()),
                        reinterpret_cast<LPDWORD>(&nWritten),
                        nullptr)){
            #else // Unix
                nWritten = ::write(fd, obuffer_.data(), obuffer_.size());
                if (nWritten == -1) {
            #endif
                    throw FileStreamError("Failed to write data to file: " + getLastErrorAsString());
                }

                // Clear out buffer
                obuffer_.reset();
            }

            if(sync)
            {
            #ifdef LIBIM_OS_WINDOWS
                if(!FlushFileBuffers(hFile)) {
            #else // Unix
                if (fsync(fd) != 0) {
            #endif
                    throw FileStreamError("Failed to flush data to file: " + getLastErrorAsString());
                }
            }
        }

        return static_cast<std::size_t>(nWritten);
    }

    std::size_t write(const byte_t* data, std::size_t length)
    {
        std::size_t nTotalWritten = 0;
        do
        {
            std::size_t nWritten = obuffer_.write(data, length - nTotalWritten);
            if(nWritten < length)
            {
                auto nFlushed = flush(/*sync=*/false);
                if(nFlushed < nWritten) {
                    return nFlushed;
                }
            }

            nTotalWritten += nWritten;
            data += nWritten;

        #ifdef MAX_WRITE_FILE_SIZE
            if( currentOffset + nTotalWritten >= MAX_WRITE_FILE_SIZE) {
                throw FileStreamError("Wrote to max file size limit");
            }
        #endif
        }
        while(nTotalWritten < length);

        currentOffset += nTotalWritten;
        if(currentOffset > fileSize) {
            fileSize = currentOffset;
        }

        return nTotalWritten;
    }

    void seek(std::size_t offset) const
    {
        const_cast<FileStreamImpl*>(this)->flush(/*sync=*/true);

    #ifdef LIBIM_OS_WINDOWS
        LARGE_INTEGER li;
        li.QuadPart = offset;
        li.LowPart  = SetFilePointer(hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
        if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
    #else
        auto off = lseek(fd, offset, SEEK_SET);
        if(off == -1) {
    #endif
            throw FileStreamError(std::string("Failed to seek to offset: ") + getLastErrorAsString());
        }

        currentOffset = offset;
        if(currentOffset > fileSize) {
            fileSize = currentOffset;
        }
    }

    void close()
    {
        try {
            flush(/*sync=*/true);
        }
        // catch any exception that could occur
        catch(...){}

        // Close file handle
    #ifdef LIBIM_OS_WINDOWS
        if(hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile); // can throw under a debugger
            hFile = INVALID_HANDLE_VALUE;
        }
    #else
        if(fd > 0)
        {
            ::close(fd);
            fd = -1;
        }
    #endif
    }

    ~FileStreamImpl()
    {
        close();
    }

    Mode mode;
    std::string filePath;
    mutable std::size_t fileSize = 0;
    mutable std::size_t currentOffset = 0;

private:
    IOBuffer<kBufferSize> obuffer_;

#ifdef LIBIM_OS_WINDOWS
    HANDLE hFile = INVALID_HANDLE_VALUE;
#else
    int fd = 0;
#endif
};


FileStream::FileStream(std::string filePath, Mode mode) :
    FileStream(std::move(filePath), false, mode)
{}

FileStream::FileStream(std::string filePath, bool truncate, Mode mode) :
    m_fs(std::make_shared<FileStreamImpl>(getNativePath(std::move(filePath)), truncate, mode))

{
    this->setName(getFilename(m_fs->filePath));
}

FileStream::FileStream(const std::filesystem::path& filePath, Mode mode) :
    FileStream(filePath.string(), mode)
{}

FileStream::FileStream(const std::filesystem::path& filePath, bool truncate, Mode mode) :
    FileStream(filePath.string(), truncate, mode)
{}

FileStream::~FileStream()
{}

std::size_t FileStream::writesome(const byte_t* data, std::size_t length)
{
    return m_fs->write(data, length);
}

void FileStream::seek(std::size_t offset) const
{
    m_fs->seek(offset);
}

std::size_t FileStream::size() const
{
    return m_fs->fileSize;
}

std::size_t FileStream::tell() const
{
    return m_fs->currentOffset;
}

bool FileStream::canRead() const
{
    return (m_fs->mode == Read || m_fs->mode == ReadWrite);
}

bool FileStream::canWrite() const
{
    return (m_fs->mode == Write || m_fs->mode == ReadWrite);
}

void FileStream::close()
{
    m_fs->close();
}

void FileStream::flush()
{
    m_fs->flush(/*sync=*/true);
}

std::size_t FileStream::readsome(byte_t* data, std::size_t length) const
{
    if(m_fs->currentOffset + length >= m_fs->fileSize){
        length = m_fs->fileSize - m_fs->currentOffset;
    }

    return m_fs->read(data, length);
}
