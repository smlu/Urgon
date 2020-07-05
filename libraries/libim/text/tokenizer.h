#ifndef LIBIM_TOKENIZER_H
#define LIBIM_TOKENIZER_H
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
        Tokenizer(const InputStream& s);
        virtual ~Tokenizer();

        // Explicitly delete copy and move ctors
        Tokenizer(const Tokenizer&) = delete;
        Tokenizer(Tokenizer&&) noexcept = delete;
        Tokenizer& operator=(const Tokenizer&) = delete;
        Tokenizer operator=(Tokenizer&&) noexcept = delete;

        [[nodiscard]] const Token& currentToken() const;
        const Token& getNextToken(bool lowercased = false);
        [[nodiscard]] const Token& peekNextToken(bool lowercased = false);


        template <typename T, typename DT = std::decay_t<T>>
        DT getFlags()
        {
            static_assert(utils::isEnum<DT>,
                "T must be either enum, Flags or TypeMask type"
            );
            auto tkn = getNextToken();
            return DT(tkn.getNumber<utils::underlying_type_t<DT>>());
        }

        template <typename T, typename DT = std::decay_t<T>>
        DT getNumber()
        {
            auto tkn = getNextToken();
            return tkn.getNumber<DT>();
        }

        std::string getIdentifier();
        std::string getStringLiteral();
        const std::string& getSpaceDelimitedString(bool throwIfEmpty = true);
        const std::string& getDelimitedString(const std::function<bool(char)>& isDelim);


        const std::string& getString(std::size_t len);

        void assertIdentifier(std::string_view id);
        void assertPunctuator(std::string_view punc);
        void assertEndOfFile();

        void skipNextToken();
        bool skipNextTokenIf(TypeMask<Token::Type> mask); // !< Skips next token if it maches the type. Returns true if skipped.
        bool skipNextTokenIfNot(TypeMask<Token::Type> mask); // !< Skips next token if it doesn't match the type. Returns true if skipped.
        void skipToNextLine();

        void setReportEol(bool report);
        bool reportEol() const;

        const InputStream& istream() const;

    protected:
        void getNextToken(Token& out);
        void peekNextToken(Token& out);
        void getSpaceDelimitedString(Token& out, bool throwIfEmpty = true);
        void getDelimitedString(Token& out, const std::function<bool(char)>& isDelim);
        void getString(Token& out, std::size_t len);

    protected:
        Token cachedTkn_;
        Token peekedTkn_;
        class TokenizerPrivate;
        std::unique_ptr<
            TokenizerPrivate
        > tp_;
    };

}
#endif // LIBIM_TOKENIZER_H
