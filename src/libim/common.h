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

#if defined(WIN32) || defined(_WIN32)
#  define OS_WINDOWS 1
#  include <windows.h>
#endif

#if !defined(OS_WINDOWS) || defined(__MINGW32__)
#  include <sys/stat.h>
#endif

#ifdef PACKED
#  undef PACKED
#endif

#ifndef _MSC_VER
#  define PACKED( class_to_pack ) class_to_pack __attribute__((packed, aligned(1)))
#else
#  define PACKED( class_to_pack ) __pragma( pack(push, 1) ) class_to_pack __pragma( pack(pop) )
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
    static constexpr std::size_t BYTE_BIT = CHAR_BIT;

    using byte_t = uint8_t;
    using ByteArray = std::vector<byte_t>;

    using Bitmap = ByteArray;
    using BitmapPtr = std::shared_ptr<Bitmap>;

    inline BitmapPtr MakeBitmapPtr(std::size_t size) {
        return std::make_shared<Bitmap>(size);
    }



    template <typename T,
        typename R = std::enable_if_t<std::is_integral<T>::value,
        typename std::make_unsigned<T>::type>>
    inline constexpr R Abs(T val)
    {
        R mask = val >> ((2 << sizeof(val)) - 1);
        return (mask ^ val) - mask;
    }

    inline constexpr uint32_t BBS(uint32_t bits) //Bits byte size
    {
        return bits / BYTE_BIT;
    }

    inline constexpr uint32_t GetBitmapSize(int32_t width, int32_t height, uint32_t bpp)
    {
        return Abs(height * width) * BBS(bpp);
    }

    inline constexpr uint32_t GetRowSize(int32_t width, uint32_t bpp)
    {
        return Abs(width) * BBS(bpp);
    }

    inline constexpr uint32_t GetMipmapPixelDataSize(std::size_t numTextures, int32_t width, int32_t height, uint32_t bpp)
    {
        uint32_t size = 0;
        while(numTextures --> 0)
        {
            size += GetBitmapSize(width, height, bpp);
            width  = width  >> 1;
            height = height >> 1;
        }
        return size;
    }

    inline constexpr uint32_t RGBMask(uint32_t bitsPerColor, uint32_t colorLeftShift)
    {
        return ((1 << bitsPerColor) - 1) << colorLeftShift;
    }

    static std::vector<std::string> SplitString(const std::string& string, const std::string& delim)
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

    inline std::vector<std::string> SplitString(const std::string& string, char delim)
    {
        return SplitString(string, std::string(1, delim));
    }

    inline constexpr char PathSeparator()
    {
    #ifdef OS_WINDOWS
        return '\\';
    #else
        return '/';
    #endif
    }

    inline constexpr char NoneNativePathSeparator()
    {
    #ifdef OS_WINDOWS
        return '/';
    #else
        return '\\';
    #endif
    }

    inline bool IsNativePath(const std::string& path)
    {
        return path.find(NoneNativePathSeparator()) == std::string::npos;
    }

    inline std::string GetNativePath(std::string path)
    {
        std::replace_if
        (
            path.begin(),
            path.end(),
            [](char ch) { return ch == NoneNativePathSeparator(); },
            PathSeparator()
        );

        return path;
    }

    inline std::string GetFileName(const std::string& path)
    {
        std::string name = path;
        if(!IsNativePath(name)) {
            name = GetNativePath(name);
        }

        size_t sep = name.find_last_of(PathSeparator());
        if (sep != std::string::npos) {
            name = name.substr(sep + 1, name.size() - sep - 1);
        }

        return name;
    }

    inline std::string GetBaseName(const std::string& path)
    {
        std::string name = GetFileName(path);

        size_t dot = name.find_last_of(".");
        if (dot != std::string::npos) {
            name = name.substr(0, dot);
        }

        return name;
    }

    inline std::string GetFileExtension(const std::string& path)
    {
        size_t dotPos = path.find_last_of(".");
        if (dotPos == std::string::npos) {
            return "";
        }

        return  path.substr(dotPos + 1);
    }

    inline bool IsFilePath(const std::string& path)
    {
        return path.find_last_of(".") != std::string::npos;
    }

    inline bool FileExists(const std::filesystem::path& filePath)
    {
        if(filePath.empty()) {
            return false;
        }

        return std::filesystem::exists(filePath);
    }

    inline bool DirExists(const std::filesystem::path& dirPath)
    {
        if(dirPath.empty()) {
            return false;
        }
        else if(!IsNativePath(dirPath)) {
            return DirExists(GetNativePath(dirPath));
        }

         return std::filesystem::exists(dirPath);
    }

    inline bool MakeDir(const std::filesystem::path& dirName)
    {
        return std::filesystem::create_directory(dirName);
    }

    static bool MakePath(const std::string& path, bool createFile = false)
    {
        if(path.empty()) {
            return false;
        }
        else if(!IsNativePath(path)) {
            return MakePath(GetNativePath(path));
        }

        std::string currentPath = path.at(0) == PathSeparator() ? std::string(1, PathSeparator()) : "";

        auto pathParts = SplitString(path, PathSeparator());
        for(auto&& part : pathParts)
        {
            currentPath += std::move(part);
            if(!IsFilePath(currentPath))
            {
                if(!DirExists(currentPath) && !MakeDir(currentPath)) {
                    return false;
                }
            }
            else if(createFile && !FileExists(currentPath)) {
                // TODO: make file
                break;
            }

            currentPath += std::string(1, PathSeparator());
        }

        return true;
    }

    inline bool RemoveFile(const std::filesystem::path& file)
    {
        return std::filesystem::remove(file);
    }

    inline bool RenameFile(const std::filesystem::path& from, const std::filesystem::path&& to, bool override = true)
    {
        if(FileExists(to) && !override) {
            return false;
        }

        RemoveFile(to);

        try
        {
            std::filesystem::rename(from, to);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    inline std::string IosErrorStr(const std::ios& ios)
    {
        std::string error = "No error";
        if(ios.eof()){
            error = "End of stream reached";
        }
        else if(ios.bad()) {
            error = "I/O stream error!";
        }
        else if(ios.fail()) {
            error = "Internal stream error";
        }

        return error;
    }

}
#endif // LIBIM_COMMON_H
