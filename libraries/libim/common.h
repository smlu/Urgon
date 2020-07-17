#ifndef LIBIM_COMMON_H
#define LIBIM_COMMON_H
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <climits>
#include <filesystem>
#include <ios>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>
#include <utility>

#include "types/optref.h"
#include "utils/utils.h"

#if defined(WIN32) || defined(_WIN32)
#  define OS_WINDOWS 1
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

#if !defined(OS_WINDOWS) || defined(__MINGW32__)
#  include <sys/stat.h>
#endif

#ifdef PACKED
#  undef PACKED
#endif

#ifndef _MSC_VER
#  define PACKED( ... ) __VA_ARGS__ __attribute__((packed, aligned(1)))
#else
#  define PACKED( ... ) __pragma( pack(push, 1) ) __VA_ARGS__ __pragma( pack(pop) )
#endif

namespace libim {
    static_assert(CHAR_BIT == 8, "byte bit count != 8");
    static constexpr std::size_t CHAR_BYTE   = 1;
    static constexpr std::size_t INT8_BYTE   = 1;
    static constexpr std::size_t INT16_BYTE  = 2;
    static constexpr std::size_t INT32_BYTE  = 4;
    static constexpr std::size_t INT64_BYTE  = 8;
    static constexpr std::size_t FLOAT_BYTE  = 4;
    static constexpr std::size_t DOUBLE_BYTE = 8;
    static constexpr std::size_t BYTE_BIT    = CHAR_BIT;

    using byte_t    = uint8_t;
    using ByteArray = std::vector<byte_t>;

    using Bitmap    = ByteArray;
    using BitmapPtr = std::shared_ptr<Bitmap>;

    inline BitmapPtr makeBitmapPtr(std::size_t size) {
        return std::make_shared<Bitmap>(size);
    }


    template <typename T,
        typename R = std::enable_if_t<std::is_integral<T>::value,
        typename std::make_unsigned<T>::type>>
    inline constexpr R abs(T val)
    {
        R mask = val >> ((2 << sizeof(val)) - 1);
        return (mask ^ val) - mask;
    }

    inline constexpr uint32_t bbs(uint32_t bits) { //Bits byte size
        return bits / BYTE_BIT;
    }

    inline constexpr uint32_t getBitmapSize(int32_t width, int32_t height, uint32_t bpp)
    {
        return abs(height * width) * bbs(bpp);
    }

    inline constexpr uint32_t getRowSize(int32_t width, uint32_t bpp)
    {
        return abs(width) * bbs(bpp);
    }

    inline constexpr uint32_t getMipmapPixelDataSize(std::size_t numTextures, int32_t width, int32_t height, uint32_t bpp)
    {
        uint32_t size = 0;
        while(numTextures --> 0)
        {
            size += getBitmapSize(width, height, bpp);
            width  = width  >> 1;
            height = height >> 1;
        }
        return size;
    }

    inline constexpr uint32_t rgbMask(uint32_t bitsPerColor, uint32_t colorLeftShift)
    {
        return ((1 << bitsPerColor) - 1) << colorLeftShift;
    }

    static std::vector<std::string> splitString(const std::string& string, const std::string& delim)
    {
        std::vector<std::string> tokens;

        std::size_t prevPos = 0;
        std::size_t pos = 0;

        while ((pos = string.find(delim, prevPos)) != std::string::npos)
        {
            std::string token = string.substr(prevPos, pos - prevPos);
            tokens.emplace_back(std::move(token));
            prevPos = ++pos;
        }

        if(prevPos < string.size()) {
            tokens.push_back(string.substr(prevPos));
        }

        return tokens;
    }

    inline std::vector<std::string> splitString(const std::string& string, char delim)
    {
        return splitString(string, std::string(1, delim));
    }

    inline constexpr char pathSeparator()
    {
    #ifdef OS_WINDOWS
        return '\\';
    #else
        return '/';
    #endif
    }

    inline constexpr char noneNativePathSeparator()
    {
    #ifdef OS_WINDOWS
        return '/';
    #else
        return '\\';
    #endif
    }

    inline bool isNativePath(const std::string& path)
    {
        return path.find(noneNativePathSeparator()) == std::string::npos;
    }

    inline std::string getNativePath(std::string path)
    {
        std::replace_if
        (
            path.begin(),
            path.end(),
            [](char ch) { return ch == noneNativePathSeparator(); },
            pathSeparator()
        );

        return path;
    }

    inline std::string getFilename(const std::string& path)
    {
        std::string name = path;
        if(!isNativePath(name)) {
            name = getNativePath(name);
        }

        size_t sep = name.find_last_of(pathSeparator());
        if (sep != std::string::npos) {
            name = name.substr(sep + 1, name.size() - sep - 1);
        }

        return name;
    }

    inline std::string getBaseName(const std::string& path)
    {
        std::string name = getFilename(path);

        size_t dot = name.find_last_of(".");
        if (dot != std::string::npos) {
            name = name.substr(0, dot);
        }

        return name;
    }

    inline std::string getFileExtension(const std::string& path)
    {
        size_t dotPos = path.find_last_of(".");
        if (dotPos == std::string::npos) {
            return "";
        }

        return  path.substr(dotPos + 1);
    }

    inline bool fileExtMatch(const std::filesystem::path& path, std::string_view ext, bool icase = true)
    {
        auto pext = path.extension().string();
        if (pext.empty() != ext.empty()) {
            return false;
        }

        const std::string_view svpext(pext);
        if (icase) {
            return utils::iequal(svpext, ext);
        }

        return svpext == ext;
    }

    inline bool isFilePath(const std::filesystem::path& path, OptionalRef<std::error_code> ec = std::nullopt)
    {
        using namespace std::filesystem;
        if (ec) {
            return is_regular_file(path, *ec) || path.has_extension();
        }
        return is_regular_file(path) || path.has_extension();
    }

    inline bool isFilePath(const std::string& path, OptionalRef<std::error_code> ec)
    {
        return isFilePath(std::filesystem::path(path), ec);
    }

    inline bool fileExists(const std::filesystem::path& filePath)
    {
        if(filePath.empty() || !isFilePath(filePath)) {
            return false;
        }

        return std::filesystem::exists(filePath);
    }

    inline bool isDirPath(const std::filesystem::path& path, OptionalRef<std::error_code> ec = std::nullopt)
    {
        using namespace std::filesystem;
        if (ec) {
            return is_directory(path, *ec) || !isFilePath(path);
        }
        return is_directory(path) || !isFilePath(path);
    }

    inline bool isDirPath(const std::string& path, OptionalRef<std::error_code> ec)
    {
        return isDirPath(std::filesystem::path(path), ec);
    }

    inline bool dirExists(const std::filesystem::path& dirPath)
    {
        if(dirPath.empty() || isFilePath(dirPath)) {
            return false;
        }
        else if(!isNativePath(dirPath.string())) {
            return dirExists(getNativePath(dirPath.string()));
        }

        return std::filesystem::exists(dirPath);
    }

    inline bool makeDir(const std::filesystem::path& dirName)
    {
        return std::filesystem::create_directory(dirName);
    }

    static bool makePath(const std::string& path, bool createFile = false)
    {
        if(path.empty()) {
            return false;
        }
        else if(!isNativePath(path)) {
            return makePath(getNativePath(path));
        }

        std::string currentPath = path.at(0) == pathSeparator() ? std::string(1, pathSeparator()) : "";

        auto pathParts = splitString(path, pathSeparator());
        for(auto&& part : pathParts)
        {
            currentPath += std::move(part);
            if(!isFilePath(currentPath))
            {
                if(!dirExists(currentPath) && !makeDir(currentPath)) {
                    return false;
                }
            }
            else if(createFile && !fileExists(currentPath)) {
                // TODO: make file
                break;
            }

            currentPath += std::string(1, pathSeparator());
        }

        return true;
    }

    inline bool makePath(const std::filesystem::path& path, bool createFile = false)
    {
        return makePath(path.u8string(), createFile);
    }

    inline bool removeFile(const std::filesystem::path& file, OptionalRef<std::error_code> ec = std::nullopt)
    {
        if(ec) {
            return std::filesystem::remove(file, *ec);
        }
        return std::filesystem::remove(file);
    }

    inline bool deleteFile(const std::filesystem::path& file)
    {
        static std::error_code ec;
        return removeFile(file, ec);
    }

    inline bool renameFile(const std::filesystem::path& from, const std::filesystem::path& to, bool override = true)
    {
        if(fileExists(to) && !override) {
            return false;
        }

        deleteFile(to);
        try
        {
            std::filesystem::rename(from, to);
            return true;
        }
        catch (...) {
            return false;
        }
    }
}
#endif // LIBIM_COMMON_H
