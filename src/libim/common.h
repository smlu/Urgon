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

#if defined(WIN32) || defined(_WIN32)
#define OS_WINDOWS 1
#endif

#ifdef OS_WINDOWS
    static constexpr char PATH_SEP_CH = '\\';
#else
    static constexpr char PATH_SEP_CH = '/';
#endif

#ifndef _MSC_VER
#define PACKED( class_to_pack ) class_to_pack __attribute__((packed, aligned(1)))
#else
#define PACKED( class_to_pack ) __pragma( pack(push, 1) ) class_to_pack __pragma( pack(pop) )
#endif


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
    return bits / BYTE_BIT;
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
    while( 0 < static_cast<int32_t>(--nMipmaps))
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

inline std::string GetFileNameFromPath(const std::string& path)
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

inline std::string GetNativePath(std::string path)
{
#ifdef OS_WINDOWS
    static constexpr char notNativePathSep = '/';
#else
    static constexpr char notNativePathSep = '\\';
#endif

    std::replace_if
    (
        path.begin(),
        path.end(),
        [](char ch) { return ch == notNativePathSep;},
        PATH_SEP_CH
    );

    return path;
}

inline std::string IosErrorStr(const std::ios& ios)
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
