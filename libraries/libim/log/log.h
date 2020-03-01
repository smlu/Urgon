#ifndef LIBIM_LOG_H
#define LIBIM_LOG_H
#include "log_level.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

namespace libim {
    struct LoggingError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    inline LogLevel gLogLevel = LogLevel::Debug;

    namespace detail {
        inline  void ss_printf(std::stringstream& ss, const char* format) {
            ss << format;
        }

        template<typename T, typename... Targs>
        static void ss_printf(std::stringstream& ss, const char* format, T value, Targs&&... Fargs) // recursive variadic function
        {
            for ( ; *format != '\0'; format++ )
            {
                if( *format == '%' )
                {
                   ss << value;
                   ss_printf(ss, format + 1, Fargs...);
                   return;
                }
                ss << *format;
            }
        }
    }

    template<typename ...Args>
    static void write_log(const char* file, int line, LogLevel level, std::string_view msg, Args&&... args)
    {
        if(gLogLevel < level) {
            return;
        }

        using namespace std::string_literals;
        using namespace detail;

        std::stringstream ss;
        std::stringstream prefix;

        ss << "[" << level.toString() << "]" << " ";
        detail::ss_printf(ss, msg.data(), args...);
        if(level <= LogLevel::Warning)
        {
#ifndef NDEBUG
        ss << "\nfile: '" << file << "':" << line << " ";
#endif
            std::cerr << ss.str() << std::endl;
        } else {
            std::cout << ss.str() << std::endl;
        }
    }
}

#define LOG_WITH_LEVEL(l, msg, ...) \
    libim::write_log(__FILE__, __LINE__, l, msg, ##__VA_ARGS__)

#define LOG_VERBOSE(msg, ...) \
    LOG_WITH_LEVEL(libim::LogLevel::Verbose, msg, ##__VA_ARGS__)

#define LOG_DEBUG(msg, ...) \
    LOG_WITH_LEVEL(libim::LogLevel::Debug, msg, ##__VA_ARGS__)

#define LOG_INFO(msg, ...) \
    LOG_WITH_LEVEL(libim::LogLevel::Info, msg, ##__VA_ARGS__)

#define LOG_WARNING(msg, ...) \
    LOG_WITH_LEVEL(libim::LogLevel::Warning, msg, ##__VA_ARGS__)

#define LOG_ERROR(msg, ...) \
    LOG_WITH_LEVEL(libim::LogLevel::Error, msg, ##__VA_ARGS__)

#endif // LOG_H
