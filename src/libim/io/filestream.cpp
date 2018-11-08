#include "filestream.h"
#include "../common.h"
#include <algorithm>

#ifdef OS_WINDOWS
#include <windows.h>
#include <locale>
#include <codecvt>
#include <string>
# ifdef _WIN64
  typedef int64_t ssize_t;
# else
  typedef int32_t ssize_t;
# endif
#else
# include <assert.h>
# include <cstring>
# include <errno.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
#endif

using namespace libim;

std::string GetLastErrorAsString()
{
    std::string message;
#ifdef OS_WINDOWS
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

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

    FileStreamImpl(std::string fp, Mode mode) : mode(mode), filePath(std::move(fp))
    {
        auto flags = [&]
        {
            switch (mode)
            {
            #ifdef OS_WINDOWS
            case Read:      return (int)GENERIC_READ;
            case Write:     return (int)GENERIC_WRITE;
            case ReadWrite: return (int)(GENERIC_WRITE | GENERIC_READ);
            #else
            case Read:      return O_RDONLY;
            case Write:     return O_WRONLY | O_CREAT;
            case ReadWrite: return O_RDWR   | O_CREAT;
            #endif
            default:
                return -1;
            }
        }();

        if(flags == -1) {
            throw FileStreamError("Unknown file open mode!");
        }

    #ifdef OS_WINDOWS
        
        /* Open file */
        #if _WIN32_WINNT >= _WIN32_WINNT_WIN8
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wPath = converter.from_bytes(filePath.c_str());

        fileHandle = CreateFile2(wPath.c_str(), 
                        flags, 
                        FILE_SHARE_READ, 
                        (flags == GENERIC_READ ? OPEN_EXISTING : OPEN_ALWAYS),
                        NULL);
        #else
        fileHandle = CreateFileA(filePath.c_str(),
                        flags,
                        FILE_SHARE_READ, 
                        NULL, 
                        (flags == GENERIC_READ ? OPEN_EXISTING : OPEN_ALWAYS), 
                        FILE_ATTRIBUTE_NORMAL, 
                        NULL);
        #endif

        if (fileHandle == INVALID_HANDLE_VALUE) {
            throw FileStreamError(GetLastErrorAsString());
        }

        /* Get file size */
        LARGE_INTEGER lSize {0};
        if(!GetFileSizeEx(fileHandle, &lSize)) {
            throw FileStreamError("Error getting the file size: " + GetLastErrorAsString());
        }

        #ifdef _WIN64
        fileSize = lSize.QuadPart;
        #else
        fileSize = lSize.LowPart;
        #endif

    #else // Not Win
        /* Open file */
        fd = open(filePath.c_str(), flags, (mode_t)0600);
        if (fd == -1) {
            throw FileStreamError(strerror(errno));
        }

        /* Get file size */
        struct stat fileInfo {};
        if (fstat(fd, &fileInfo) == -1) {
            throw FileStreamError(std::string("Error getting the file size: ") + strerror(errno));
        }

        fileSize = fileInfo.st_size;
    #endif
    }

    std::size_t read(byte_t* data, std::size_t length)
    {
        ssize_t nRead = 0;
    #ifdef OS_WINDOWS
        if(!ReadFile(fileHandle, reinterpret_cast<LPVOID>(data), (DWORD)length, (LPDWORD)&nRead, NULL)) {
    #else
        nRead = ::read(fd, data, length);
        if(nRead == -1) {
    #endif
            throw FileStreamError("Failed to read from file: " + GetLastErrorAsString());
        }

        currentOffset += nRead;
        return static_cast<std::size_t>(nRead);
    }

    std::size_t write(const byte_t* data, std::size_t length)
    {
        ssize_t nWritten = 0;
    #ifdef OS_WINDOWS
        if(!WriteFile(fileHandle, reinterpret_cast<LPCVOID>(data), (DWORD)length, (LPDWORD)&nWritten, NULL)) {
    #else
        nWritten = ::write(fd, data, length);
        if(nWritten == -1) {
    #endif
            throw FileStreamError("Failed to write data to file: " + GetLastErrorAsString());
        }

        currentOffset += nWritten;
        if(currentOffset > fileSize) {
            fileSize = currentOffset;
        }

        return static_cast<std::size_t>(nWritten);
    }

    void seek(std::size_t position) const
    {
    #ifdef OS_WINDOWS
        LARGE_INTEGER li;
        li.QuadPart = position;
        li.LowPart = SetFilePointer(fileHandle, li.LowPart, &li.HighPart, FILE_BEGIN);
        if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
    #else
        auto off = lseek(fd, position, SEEK_SET);
        if(off == -1) {
    #endif
            throw FileStreamError(std::string("Failed to seek to position: ") + GetLastErrorAsString());
        }

        currentOffset = position;
        if(currentOffset > fileSize) {
            fileSize = currentOffset;
        }
    }

    void close()
    {
#ifdef OS_WINDOWS
        if(fileHandle != INVALID_HANDLE_VALUE)
        {
            if(mode == Write || mode == ReadWrite){
                FlushFileBuffers(fileHandle);
            }

            CloseHandle(fileHandle);
            fileHandle = INVALID_HANDLE_VALUE;
        }
#else
        if(fd > 0)
        {
            if(mode == Write || mode == ReadWrite){
                fsync(fd);
            }

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

#ifdef OS_WINDOWS
HANDLE fileHandle = INVALID_HANDLE_VALUE;

//#error "FileStreamImpl not defined!"
#else
    int fd = 0;
#endif
};

FileStream::FileStream(std::string filePath, Mode mode) :
    m_fs(std::make_shared<FileStreamImpl>(GetNativePath(std::move(filePath)), mode))
{
    this->setName(GetFileName(m_fs->filePath));
}

FileStream::~FileStream()
{}

std::size_t FileStream::writesome(const byte_t* data, std::size_t length)
{
    return m_fs->write(data, length);
}

void FileStream::seek(std::size_t position) const
{
    m_fs->seek(position);
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

std::size_t FileStream::readsome(byte_t* data, std::size_t length) const
{
    if(m_fs->currentOffset + length >= m_fs->fileSize){
        length = m_fs->fileSize - m_fs->currentOffset;
    }

    return m_fs->read(data, length);
}
