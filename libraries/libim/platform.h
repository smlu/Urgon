#ifndef LIBIM_PLATFORM_H
#define LIBIM_PLATFORM_H
#include <cstdint>

#ifdef __APPLE__
#   include "TargetConditionals.h"
#elif defined(WIN32) || defined(_WIN32)
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <windows.h>
#   undef NOMINMAX
#endif

#if !defined(_WIN32) || defined(__MINGW32__)
#  include <sys/stat.h>
#endif

#if INTPTR_MAX == INT64_MAX
# define LIBIM_PLATFORM_64BIT 1
#elif INTPTR_MAX == INT32_MAX
# define LIBIM_PLATFORM_32BIT
#else
# error Unknown pointer size or missing size macros!
#endif

namespace libim {
    enum class PlatformOS
    {
        Windows,
        macOS,
        iOS,
        Linux,
        Android,
        UnknownOS
    };

#ifdef __APPLE__
#   ifdef TARGET_OS_MAC
#       define LIBIM_OS_MACOS 1
        inline constexpr auto platformOS = PlatformOS::macOS;
#   else
#       define LIBIM_OS_IOS 1
        inline constexpr auto platformOS = PlatformOS::iOS;
#   endif
#elif defined(_WIN32) || defined(__MINGW32__)
#   define LIBIM_OS_WINDOWS 1
    inline constexpr auto platformOS = PlatformOS::Windows;
#elif defined(__linux__)
#   ifdef __ANDROID__
#       define LIBIM_OS_ANDROID 1
        inline constexpr auto platformOS = PlatformOS::Android;
#   else
#       define LIBIM_OS_LINUX 1
        inline constexpr auto platformOS = PlatformOS::Linux;
#   endif
#else
#   warning "Unknown platform operating system!"
    inline constexpr auto platformOS = PlatformOS::UnknownOS;
#endif
}
#endif //LIBIM_PLATFORM_H
