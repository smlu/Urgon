#ifndef TOKENIZER_ERROR_H
#define TOKENIZER_ERROR_H
#include <exception>
#include <string_view>

#include "diagnostic_location.h"

namespace libim::text {
    class TokenizerError : public std::exception
    {
    public:
        TokenizerError(std::string_view what, const diagnostic_location& location) :
            m_what(what),
            m_loc(location) // Fixme: diagnostic_location contains std::string_view
                            // which can be invalidated at any point of stack unwinding.
        {}

        virtual const char* what() const noexcept override
        {
            return m_what.data();
        }

        const diagnostic_location& location() const
        {
            return m_loc;
        }

    private:
        std::string_view m_what;
        diagnostic_location m_loc;
    };
}
#endif // TOKENIZER_ERROR_H
