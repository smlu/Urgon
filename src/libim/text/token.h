#ifndef LIBIM_TEXT_TOKEN_H
#define LIBIM_TEXT_TOKEN_H
#include <string>
#include <sstream>
#include <type_traits>
#include "diagnostic_location.h"
#include "tokenizer_error.h"

namespace libim::text {
    class Token final
    {
    public:
        enum Type
        {
            Invalid = -1,
            EndOfFile,
            EndOfLine,
            Identifier,
            String,
            Punctuator,
            Integer,
            HexInteger,
            OctInteger,
            FloatNumber
        };

        Token() = default;
        Token(Type type, std::string value) :
            m_type(type),
            m_value(std::move(value))
        {}

        Token(Type type, std::string value, diagnostic_location loc) :
            m_type(type),
            m_value(std::move(value)),
            m_loc(std::move(loc))
        {}

        void setLocation(diagnostic_location loc)
        {
            m_loc = std::move(loc);
        }

        const diagnostic_location& location() const
        {
            return m_loc;
        }

        diagnostic_location& location()
        {
            return m_loc;
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

        template<typename T>
        T getNumber() const
        {
            static_assert(std::is_arithmetic_v<T>, "T is not a numeric type");

            T result = T(0);
            std::stringstream ss(m_value);
            switch(m_type)
            {
                case OctInteger: {
                    ss >> std::oct;
                } break;
                case HexInteger: {
                    ss >> std::hex;
                } break;
                default:
                    break;
            }

            ss >> result;
            if(ss.fail()) {
                using namespace std::string_view_literals;
                throw TokenizerError("invalid numeric conversion from string"sv, m_loc);
            }

            return result;
         }

        void append(char c)
        {
            m_value.push_back(c);
        }

        bool isEmpty() const
        {
            return m_value.empty();
        }

        inline void clear()
        {
            m_type = Invalid;
            m_value.clear();
        }

    private:
        Type m_type = Invalid;
        std::string m_value;
        diagnostic_location m_loc;
    };

}
#endif // LIBIM_TEXT_TOKEN_H
