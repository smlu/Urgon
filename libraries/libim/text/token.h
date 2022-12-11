#ifndef LIBIM_TEXT_TOKEN_H
#define LIBIM_TEXT_TOKEN_H
#include <string>
#include <sstream>
#include <type_traits>

#include "parselocation.h"
#include "syntax_error.h"
#include "../utils/utils.h"

namespace libim::text {
    class Token final
    {
    public:
        enum Type
        {
            Invalid     = -1,
            EndOfFile   = 0,
            EndOfLine   = 1,
            Identifier  = 2,
            String      = 3,
            Punctuator  = 4,
            Integer     = 5,
            HexInteger  = 6,
            OctInteger  = 7,
            FloatNumber = 8
        };

        Token() = default;
        Token(const Token&) = default;
        Token(Token&&) noexcept = default;
        Token& operator=(const Token&) = default;
        Token& operator=(Token&&) noexcept = default;
        ~Token() = default;

        Token(Type type, std::string value) :
            m_type(type),
            m_value(std::move(value))
        {}

        Token(Type type, std::string value, ParseLocation loc) :
            m_type(type),
            m_value(std::move(value)),
            m_loc(std::move(loc))
        {}

        void append(char c)
        {
            m_value.push_back(c);
        }

        inline void clear()
        {
            m_type = Invalid;
            m_value.clear();
            m_loc = ParseLocation{};
        }

        bool isEmpty() const
        {
            return m_value.empty();
        }

        inline bool isNumber() const
        {
            return m_type == Integer    ||
                   m_type == HexInteger ||
                   m_type == OctInteger ||
                   m_type == FloatNumber;
        }

        void setLocation(ParseLocation loc)
        {
            m_loc = std::move(loc);
        }

        const ParseLocation& location() const
        {
            return m_loc;
        }

        ParseLocation& location()
        {
            return m_loc;
        }

        void reserve(std::size_t len)
        {
            m_value.reserve(len);
        }

        void setValue(std::string value)
        {
            m_value = std::move(value);
        }

        const std::string& value() const &
        {
            return m_value;
        }

        const std::string& value() const &&
        {
            return m_value;
        }

        std::string value() && noexcept
        {
            m_type = Invalid;
            return std::move(m_value);
        }

        void setType(Type type)
        {
            m_type = type;
        }

        Type type() const
        {
            return m_type;
        }

        std::string_view stringType() const
        {
            using namespace std::string_view_literals;
            switch(m_type)
            {
                case Invalid:     return "Invalid"sv;
                case EndOfFile:   return "EndOfFile"sv;
                case EndOfLine:   return "EndOfLine"sv;
                case Identifier:  return "Identifier"sv;
                case String:      return "String"sv;
                case Punctuator:  return "Punctuator"sv;
                case Integer:     return "Integer"sv;
                case HexInteger:  return "HexInteger"sv;
                case OctInteger:  return "OctInteger"sv;
                case FloatNumber: return "FloatNumber"sv;
                default:          return "Unknown"sv;
            }
        }

        template<typename T, typename DT = std::decay_t<T>>
        DT getNumber() const
        {
            auto base = std::dec;
            switch(m_type)
            {
                case OctInteger: {
                    base = std::oct;
                } break;
                case HexInteger: {
                    base = std::hex;
                } break;
                default:
                    break;
            }

            DT num;
            if (!isNumber() || !utils::to_number(m_value, num, base))
            {
                using namespace std::string_view_literals;
                throw SyntaxError("Invalid numeric conversion from string"sv, m_loc);
            }

            return num;
        }

        template <typename T, typename DT = std::decay_t<T>>
        DT getFlags() const
        {
            static_assert(utils::isEnum<DT>,
                "T must be either enum, Flags or TypeMask type"
            );
            return DT(getNumber<utils::underlying_type_t<DT>>());
        }

        void toLowercase()
        {
            utils::to_lower(m_value);
        }

        Token lowercased() const
        {
            Token cpy = *this;
            cpy.toLowercase();
            return cpy;
        }

        /**
         * Returns true if token is not invalid, not end of file or not end of line.
         */
        bool isValid() const
        {
            return m_type != Invalid && m_type != EndOfFile && m_type != EndOfLine;
        }

    private:
        Type m_type = Invalid;
        std::string m_value;
        ParseLocation m_loc;
    };
}
#endif // LIBIM_TEXT_TOKEN_H
