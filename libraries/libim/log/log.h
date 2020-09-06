#ifndef LIBIM_LOG_H
#define LIBIM_LOG_H
#include "log_level.h"
#include <libim/utils/utils.h>

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

    template<typename ...Args>
    static void writeLog(const char* file, int line, LogLevel level, std::string_view msg, Args&&... args)
    {
        if(gLogLevel < level) {
            return;
        }

        using namespace std::string_literals;
        using namespace detail;

        std::stringstream ss;
        std::stringstream prefix;

        ss << "[" << level.toString() << "]" << " ";
        utils::ssprintf(ss, msg.data(), args...);
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


#define LOG_WITH_LEVEL(l, ...) \
    libim::writeLog(__FILE__, __LINE__, l, __VA_ARGS__)

#define LOG_VERBOSE(...) \
    LOG_WITH_LEVEL(libim::LogLevel::Verbose, __VA_ARGS__)

#define LOG_DEBUG(...) \
    LOG_WITH_LEVEL(libim::LogLevel::Debug, __VA_ARGS__)

#define LOG_INFO(...) \
    LOG_WITH_LEVEL(libim::LogLevel::Info, __VA_ARGS__)

#define LOG_WARNING(...) \
    LOG_WITH_LEVEL(libim::LogLevel::Warning, __VA_ARGS__)

#define LOG_ERROR(...) \
    LOG_WITH_LEVEL(libim::LogLevel::Error, __VA_ARGS__)

#endif // LOG_H
