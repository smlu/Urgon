#ifndef LIBIM_TOKENIZER_H
#define LIBIM_TOKENIZER_H
#include "parselocation.h"
#include "token.h"

#include <libim/io/stream.h>
#include <libim/utils/traits.h>
#include <libim/types/flags.h>
#include <libim/types/typemask.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace libim::text {

    class Tokenizer
    {
    public:
        /**
         * Constructs a new tokenizer.
         * @param s - InputStream to read from.
        */
        Tokenizer(const InputStream& s);
        virtual ~Tokenizer();

        // Explicitly delete copy and move ctors
        Tokenizer(const Tokenizer&) = delete;
        Tokenizer(Tokenizer&&) noexcept = delete;
        Tokenizer& operator=(const Tokenizer&) = delete;
        Tokenizer operator=(Tokenizer&&) noexcept = delete;

        /** Returns the current tokenizer stream parse location */
        [[nodiscard]] ParseLocation currentLocation();

        /** Returns last cached token */
        [[nodiscard]] const Token& currentToken() const;

        /**
         * Parses next token from stream.
         * Parsed token is also cached internally.
         *
         * @param out        - Token to store the parsed token.
         * @param lowercased - If true, the token will be lowercased.
         * @return True if parsed token is valid.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         */
        bool getNextToken(Token& out, bool lowercased = false);

        /**
         * Parses next token from stream.
         * Parsed token is also cached internally.
         *
         * @param lowercased - If true, the token will be lowercased.
         * @return Const reference to cached parsed token.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         */
        const Token& getNextToken(bool lowercased = false);

        /**
         * Peeks next token from stream.
         * Parsed token is not cached internally.
         * @param out        - Token to store the parsed token.
         * @param lowercased - If true, the token will be lowercased.
         * @return True if parsed token is valid.
         *
         * @throw StreamError - If an error occurs while reading from stream.
        */
        bool peekNextToken(Token& out, bool lowercased = false);

        /**
         * Peeks next token from stream.
         * Parsed token is not cached internally.
         * @param lowercased - If true, the token will be lowercased.
         * @return Parsed token.
        */
        [[nodiscard]] Token peekNextToken(bool lowercased = false);

        /**
         * Parses next token from stream as flag number.
         * Parsed token is also cached internally.
         *
         * @tparam T - Type of the flag number.
         * @return Parsed flag number.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not a flag number.
        */
        template <typename T, typename DT = std::decay_t<T>>
        DT getFlags()
        {
            auto tkn = getNextToken();
            return tkn.getFlags<DT>();
        }

        /**
         * Parses next token from stream as number.
         * Parsed token is also cached internally.
         *
         * @tparam T - Type of the number.
         * @return Parsed number.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not a number.
        */
        template <typename T, typename DT = std::decay_t<T>>
        DT getNumber()
        {
            auto tkn = getNextToken();
            return tkn.getNumber<DT>();
        }

        /**
         * Parses next token from stream as identifier.
         * Parsed token is also cached internally.
         * @return Parsed identifier.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not identifier.
        */
        std::string_view getIdentifier();

        /**
         * Parses next token from stream as string literal, i.e. string in format "some string".
         * Parsed token is also cached internally.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not a string.
        */
        std::string_view getStringLiteral();

        /**
         * Parses next token from stream as delimited string.
         * Parsed token is also cached internally.
         *
         * @param out       - Token to store the parsed token.
         * @param isDelim   - Function that returns true if the given character is a delimiter.
         * @return True if parsed token is valid.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not a string.
        */
        bool getDelimitedString(Token& out, const std::function<bool(char)>& isDelim);

        /**
         * Parses next token from stream as delimited string.
         * Parsed token is also cached internally.
         *
         * @param isDelim   - Function that returns true if the given character is a delimiter.
         * @return Const reference to cached parsed token.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not a string.
        */
        std::string_view getDelimitedString(const std::function<bool(char)>& isDelim);

        /**
         * Parses next token from stream as string.
         * Parsed token is also cached internally.
         *
         * @param out - Token to store the parsed token.
         * @param len - Expected length of the string.
         * @return True if parsed token is valid.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not a string.
        */
        bool getString(Token& out, std::size_t len);

        /**
         * Parses next token from stream as string.
         * Parsed token is also cached internally.
         *
         * @param len - Expected length of the string.
         * @return Const reference to cached parsed token.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not a string.
        */
        std::string_view getString(std::size_t len);

        /**
         * Parses next token from stream as space delimited string.
         * Parsed token is also cached internally.
         *
         * @param out          - Token to store the parsed token.
         * @param throwIfEmpty - If true, throws an exception if the string is empty.
         * @return True if parsed token is valid.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not a string or if throwIfEmpty is true and string is empty.
        */
        bool getSpaceDelimitedString(Token& out, bool throwIfEmpty = true);

        /**
         * Parses next token from stream as space delimited string.
         * Parsed token is also cached internally.
         *
         * @param throwIfEmpty - If true, throws an exception if the string is empty.
         * @return Const reference to cached parsed token.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not a string or if throwIfEmpty is true and string is empty.
        */
        std::string_view getSpaceDelimitedString(bool throwIfEmpty = true);

        /**
         * Asserts next token from stream as expected punctuator.
         * Parsed token is also cached internally.
         *
         * @param punc - Expected punctuator.
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not a punctuator.
        */
        void assertIdentifier(std::string_view id);

        /**
         * Asserts next token from stream as expected identifier.
         * Parsed token is also cached internally.
         *
         * @param id - Expected identifier.
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not an identifier.
        */
        void assertPunctuator(std::string_view punc);

        /**
         * Asserts next token from stream as end of file.
         * Parsed token is also cached internally.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the parsed token is not end of file.
        */
        void assertEndOfFile();

        /**
         * Skips Next token.
         * Parsed token is not cached internally.
         * @throw StreamError - If an error occurs while reading from stream.
        */
        void skipNextToken();

        /**
         * Skips next token if it matches the type.
         * Parsed token is not cached internally.
         *
         * @param mask - Type mask to match.
         * @return True if skipped.
         *
         * @throw StreamError - If an error occurs while reading from stream.
        */
        bool skipNextTokenIf(TypeMask<Token::Type> mask); // !< Skips next token if it matches the type. Returns true if skipped.

        /**
         * Skips next token if it doesn't match the type.
         * Parsed token is not cached internally.
         *
         * @param mask - Type mask to match.
         * @return True if skipped.
         *
         * @throw StreamError - If an error occurs while reading from stream.
        */
        bool skipNextTokenIfNot(TypeMask<Token::Type> mask);

        /**
         * Moves stream position to the next line.
         * @throw StreamError - If an error occurs while reading from stream.
        */
        void skipToNextLine();

        /**
         * Enables/disables reporting of end of line.
         * If true, end of line is reported as a token, else it is skipped.
         * @note Default is false.
         *
         * @param report - True to report end of line.
         * @throw StreamError - If an error occurs while reading from stream.
        */
        void setReportEol(bool report);

        /**
         * Returns end of line reporting status.
         * @return True if end of line is reported as a token.
        */
        bool reportEol() const;

        /**
         * Returns the input stream.
         * @return Const reference to input stream.
        */
        const InputStream& istream() const;

        /**
         * Returns the size of input stream.
         * @return Stream size.
        */
        std::size_t size() const
        {
            return istream().size();
        }

        /**
         * Returns the current position of input stream.
         * @return Current position.
        */
        std::size_t tell() const
        {
            return istream().tell();
        }

        /**
         * Returns the remaining size available to read.
         * @return Remaining size.
        */
        std::size_t remaining() const
        {
            return istream().remaining();
        }

    protected:
        /**
         * Parses next token from stream.
         * @param out        - Token to store the parsed token.
         * @param lowercased - If true, the token will be lowercased.
         * @param cache      - If true, the token will be cached.
         * @return True if parsed token is valid.
         */
        bool getNextToken(Token& out, bool lowercased, bool cache);

        Token cachedTkn_;
        class TokenizerPrivate;
        std::unique_ptr<
            TokenizerPrivate
        > tp_;
    };

}
#endif // LIBIM_TOKENIZER_H
