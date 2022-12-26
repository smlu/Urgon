#ifndef LIBIM_CNDERROR_H
#define LIBIM_CNDERROR_H
#include <stdexcept>

namespace libim::content::asset {
    struct CNDError final : public std::runtime_error {
       /* CNDError(const std::string& message) :
            CNDError("", message)
        {}

        CNDError(std::string&& message) :
            CNDError("", std::move(message))
        {}*/

        CNDError(const char* function, const std::string& message) :
            std::runtime_error("CND Error: " + message),
            func_(function),
            msg_(message)
        {}

        CNDError(const char* function, std::string&& message) :
            std::runtime_error("CND Error: " + message),
            func_(function),
            msg_(std::move(message))
        {}

        CNDError(const CNDError&) noexcept = default;
        CNDError(CNDError&&) noexcept = default;
        CNDError& operator=(const CNDError&) noexcept = default;
        CNDError& operator=(CNDError&&) noexcept = default;

        /* Returns the name of function that throw the error */
        const char* function() const noexcept
        {
            return func_;
        }

        /* Returns error message */
        const std::string& message() const noexcept
        {
            return msg_;
        }

    private:
        const char* func_;
        std::string msg_;
    };
}

#endif // LIBIM_CNDERROR_H