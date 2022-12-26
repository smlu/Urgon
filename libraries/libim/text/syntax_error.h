#ifndef LIBIM_SYNTAX_ERROR_H
#define LIBIM_SYNTAX_ERROR_H
#include <exception>
#include <string_view>

#include "parselocation.h"

namespace libim::text {
    class SyntaxError : public std::exception
    {
    public:
        SyntaxError(std::string_view what, const ParseLocation& location) :
            m_what(what),
            m_loc(location) // Fixme: ParseLocation contains std::string_view
                            // which can be invalidated at any point of stack unwinding.
        {}

        virtual const char* what() const noexcept override
        {
            return m_what.data();
        }

        const ParseLocation& location() const
        {
            return m_loc;
        }

    private:
        std::string_view m_what;
        ParseLocation m_loc;
    };
}
#endif // LIBIM_SYNTAX_ERROR_H
