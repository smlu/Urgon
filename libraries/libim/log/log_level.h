#ifndef LIBIM_LOG_LEVEL_H
#define LIBIM_LOG_LEVEL_H
#include <stdexcept>
#include <string_view>

namespace libim {
    struct LogLevel final
    {
        enum Level
        {
            Verbose  = 8,
            Debug    = 4,
            Info     = 2,
            Warning  = 1,
            Error    = 0
        };

        constexpr LogLevel(Level l) : m_level(l) {}
        constexpr LogLevel(const LogLevel&) = default;
        constexpr LogLevel(LogLevel&&) noexcept = default;
        constexpr LogLevel& operator = (const LogLevel&) = default;
        constexpr LogLevel& operator = (LogLevel&&) noexcept = default;

        friend constexpr bool operator == (LogLevel ll1, LogLevel ll2) {
            return ll1.m_level == ll2.m_level;
        }

        friend constexpr bool operator < (LogLevel ll1, LogLevel ll2) {
            return ll1.m_level < ll2.m_level;
        }

        friend constexpr bool operator <= (LogLevel ll1, LogLevel ll2) {
            return ll1.m_level <= ll2.m_level;
        }

        friend constexpr bool operator > (LogLevel ll1, LogLevel ll2) {
            return ll1.m_level > ll2.m_level;
        }

        friend constexpr bool operator >= (LogLevel ll1, LogLevel ll2) {
            return ll1.m_level == ll2.m_level;
        }

        constexpr std::string_view toString() const
        {
            using namespace std::string_view_literals;
            switch(m_level)
            {
                case Verbose: return "Verbose"sv;
                case Debug:   return "Debug"sv;
                case Info:    return "Info"sv;
                case Warning: return "Warning"sv;
                case Error:   return "Error"sv;
            }

            //Workaround for GCC bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67371
            // FIXME: throw std::runtime_error("LogLevel::toString: fatal error");
            return ""sv;
        }

    private:
        Level m_level;
    };
}
#endif // LIBIM_LOG_LEVEL_H
