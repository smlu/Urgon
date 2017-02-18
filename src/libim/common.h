#ifndef COMMON_H
#define COMMON_H
#include <cstdint>
#include <cstring>
#include <climits>
#include <ios>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>

#if CHAR_BIT != 8
#error Byte size is not 8 bits!
#endif

#if defined(_WIN32) || defined(WIN32)
    constexpr char PATH_SEP_CH = '\\';
#else
    constexpr char PATH_SEP_CH = '/';
#endif

#ifndef _MSC_VER
#define PACKED( class_to_pack ) class_to_pack __attribute__((packed, aligned(1)))
#else
#define PACKED( class_to_pack ) __pragma( pack(push, 1) ) class_to_pack __pragma( pack(pop) )
#endif

constexpr size_t BYTE_SIZE = 8;
using byte_t = uint8_t;

using Bitmap = std::vector<byte_t>;
using BitmapPtr = std::shared_ptr<std::vector<byte_t>>;


template <typename T,
    typename R = std::enable_if_t<std::is_integral<T>::value,
        typename std::make_unsigned<T>::type>>
inline R Abs(T val)
{
    R mask = val >> ((2 << sizeof(val)) - 1);
    return (mask ^ val) - mask;
}

inline uint32_t BBS(uint32_t bits) //Bits byte size
{
    return bits / BYTE_SIZE;
}

inline uint32_t GetBitmapSize(int32_t width, int32_t height, uint32_t bpp)
{
    return Abs(height) * (Abs(width) * BBS(bpp));
}

inline uint32_t GetRowSize(int32_t width, uint32_t bpp)
{
    return Abs(width) * BBS(bpp);
}

inline uint32_t GetMipmapPixelDataSize(uint32_t nMipmaps, int32_t width, int32_t height, uint32_t bpp)
{
    uint32_t size = GetBitmapSize(width, height, bpp);
    while( 0 < (int32_t) --nMipmaps)
    {
        width = width >> 1;
        height = height >> 1;
        size += GetBitmapSize(width, height, bpp);
    }
    return size;
}

inline uint32_t RGBMask(uint32_t bitsPerColor, uint32_t colorLeftShift)
{
    return ((1 << bitsPerColor ) - 1) << colorLeftShift;
}

std::string GetFileNameFromPath(const std::string& path)
{
    std::string name = path;

    size_t sep = name.find_last_of(PATH_SEP_CH);
    if (sep != std::string::npos){
        name = name.substr(sep + 1, name.size() - sep - 1);
    }

    size_t dot = name.find_last_of(".");
    if (dot != std::string::npos) {
        name = name.substr(0, dot);
    }

    return name;
}

std::string GetNativePath(std::string path)
{
#if defined(_WIN32) || defined(WIN32)
    char nnPathSep = '/';
#else
    char nnPathSep = '\\';
#endif

    auto nPos = path.find(nnPathSep);
    while(std::string::npos != nPos)
    {
        path[nPos] = PATH_SEP_CH;
        nPos = path.find(nnPathSep);
    }

    return path;
}

std::string IosErrorStr(const std::ios& ios)
{
    std::string error = "No error";
    if(ios.eof()){
        error = "End of stream reached";
    } else if(ios.bad()) {
        error = "I/O stream error!";
    }
    else if(ios.fail()) {
        error = "Internal stream error";
    }

    return error;
}

#endif // COMMON_H
