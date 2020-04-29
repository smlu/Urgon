#ifndef LIBIM_TOKENIZER_P_H
#define LIBIM_TOKENIZER_P_H
#include "schars.h"
#include "../token.h"
#include "../tokenizer.h"
#include "../tokenizer_error.h"
#include "../diagnostic_location.h"
#include "../../io/stream.h"

#include <array>
#include <cctype>
#include <functional>
#include <string_view>


using namespace std::string_view_literals;
namespace libim::text {

    // TODO BufferedRead should be used by InputFileStream insted of TokenizerPrivate
    template<std::size_t N>
    class BufferedRead : public InputStream
    {
    public:
        BufferedRead(const InputStream& istream) : istream_(istream)
        {
            refillBuffer();
        }

        virtual void seek(std::size_t position) const override
        {
            if((istream_.tell() < position) || position < (istream_.tell() - end_))
            {
                istream_.seek(position);
                refillBuffer();
            }
            else
            {
                auto[p, q] = std::minmax(istream_.tell() - position, end_);
                pos_ = q - p;
            }
        }

        virtual const std::string& name() const override
        {
            return istream_.name();
        }

        virtual std::size_t size() const override
        {
            return istream_.size();
        }

        virtual std::size_t tell() const override
        {
            return istream_.tell() - (end_ - pos_);
        }

        virtual bool canRead() const override
        {
            return istream_.canRead();
        }

        virtual bool canWrite() const override
        {
            return istream_.canWrite();
        }

    private:
        virtual std::size_t readsome(byte_t* data, std::size_t length) const override
        {
            std::size_t  totalRead = 0;
            while(length)
            {
                std::size_t nRead = end_ - pos_;
                if(nRead == 0)
                {
                    refillBuffer();
                    nRead = end_ - pos_;
                    if(nRead == 0)
                        return totalRead;
                }
                if (nRead > length) nRead = length;
                std::copy(buffer_.begin() + pos_, buffer_.begin() + pos_+ nRead, data);

                pos_ += nRead;
                length -= nRead;
                data += nRead;
                totalRead += nRead;
            }

            return totalRead;
        }

        virtual std::size_t writesome(const byte_t* , std::size_t ) override { return 0;}

        void refillBuffer() const
        {
            std::size_t nRead = buffer_.size();
            if(istream_.size() - istream_.tell() < nRead) {
                nRead = istream_.size() - istream_.tell();
            }
            nRead = istream_.read(const_cast<byte_t*>(buffer_.data()), nRead);
            pos_ = 0;
            end_ = nRead;
        }

    private:
        const InputStream& istream_;
        mutable std::size_t pos_ = 0;
        mutable std::size_t end_ = 0;
        std::array<byte_t, N> buffer_;
    };



    inline bool is_crlf(char c1, char c2)
    {
        return c1 == ChCr && c2 == ChEol;
    }


    class Tokenizer::TokenizerPrivate
    {
        BufferedRead<4096> istream_;
        char current_ch_, next_ch_;
        std::size_t line_   = 1;
        std::size_t column_ = 1;
        bool report_eol_ = false;

    public:
        TokenizerPrivate(const InputStream& s) :
            istream_(s)
        {
            current_ch_ = readNextChar();
            next_ch_    = readNextChar();
        }

        inline const InputStream& istream() const
        {
            return istream_;
        }

        inline char readNextChar()
        {
            while(!istream_.atEnd()) {
                return istream_.read<char>();
            }
            return ChEof;
        }

        bool isEol() const
        {
            return current_ch_ == ChEol ||
                   is_crlf(current_ch_, next_ch_);
        }

        void advance()
        {
             column_++;
             if(current_ch_ == ChEol)
             {
                 line_++;
                 column_ = 1;
             }

             current_ch_ = next_ch_;
             next_ch_ = readNextChar();
        }

        inline char peek() const
        {
            return current_ch_;
        }

        inline char peekNext() const
        {
            return next_ch_;
        }

        inline std::size_t currentLine() const
        {
            return line_;
        }

        inline std::size_t currentColumn() const
        {
            return column_;
        }

        inline static bool isIdentifierLead(char c)
        {
            return std::isalpha(c) || (c == ChIdentifier) || (c == ChIdentifier2);
        }

        inline static bool isIdentifierChar(char c)
        {
            return std::isalnum(c) || (c == ChIdentifier) || (c == ChIdentifier2);
        }

        void readString(Token& out, std::size_t len)
        {
            out.reserve(64);
            readDelimitedString(out, [len](char) mutable {
                return (len--) == 0;
            });

            if(out.value().size() != len){
                throw TokenizerError("unexpected end of file in sized string"sv, out.location());
            }
        }

        void readLine(Token& out)
        {
            out.reserve(64);
            readDelimitedString(out, [&](char) {
                return isEol();
            });
        }

        void readDelimitedString(Token& out, const std::function<bool(char)>& isDelim)
        {
            out.reserve(64);
            skipWhitespace();

            out.clear();
            out.location().first_line = line_;
            out.location().first_col  = column_;

            while(!isDelim(current_ch_) && !istream_.atEnd())
            {
                out.append(current_ch_);
                advance();
            }

            out.setType(Token::String);
            out.location().last_line = line_;
            out.location().last_col  = column_;
        }

        void readNumericLiteralHexPart(Token& out)
        {
            out.reserve(64);
            while(std::isxdigit(current_ch_))
            {
                out.append(current_ch_);
                advance();
            }
        }

        void readNumericLiteralIntegerPart(Token& out)
        {
            out.reserve(64);
            while(std::isdigit(current_ch_))
            {
                out.append(current_ch_);
                advance();
            }
        }

        void readNumericLiteral(Token& out)
        {
            out.reserve(64);
            // Check for sign
            if(current_ch_ == ChMinus || current_ch_ == ChPlus)
            {
                out.append(current_ch_);
                advance();
            }

            if(current_ch_ == '0' && (next_ch_ == 'x' || next_ch_ == 'X'))
            {
                out.setType(Token::HexInteger);
                out.append(current_ch_);
                out.append(next_ch_);

                advance();
                advance();

                readNumericLiteralHexPart(out);
                return;
            }

            out.setType(Token::Integer);
            readNumericLiteralIntegerPart(out);

            if(current_ch_ == ChDecimalSep && std::isdigit(next_ch_))
            {
                if(out.isEmpty() || !std::isdigit(out.value().back())) {
                    // Poorly formatted floating point number. Prepend 0.
                    out.append('0');
                }

                out.append(current_ch_);
                advance();

                readNumericLiteralIntegerPart(out);
                out.setType(Token::FloatNumber);
            }

            if(current_ch_ == 'e' || current_ch_ == 'E')
            {
                out.append(current_ch_);
                advance();

                if(current_ch_ == ChMinus || current_ch_ == ChPlus)
                {
                    out.append(current_ch_);
                    advance();
                }

                readNumericLiteralIntegerPart(out);
                out.setType(Token::FloatNumber);
            }
        }

        void readIdentifier(Token& out)
        {
            out.reserve(64);
            if(isIdentifierLead(current_ch_))
            {
                out.setType(Token::Identifier);
                do {
                    out.append(current_ch_);
                    advance();
                } while(isIdentifierChar(current_ch_) || current_ch_ == ChMinus);
            }
        }

        void readStringLiteral(Token& out)
        {
            out.reserve(64);
            while(true)
            {
                advance();
                if(current_ch_ == ChEof)
                {
                    out.location().last_line = line_;
                    out.location().last_col  = column_;
                    throw TokenizerError("unexpected end of file in string literal"sv, out.location());
                }
                else if(current_ch_ == ChEol)
                {
                    out.location().last_line = line_;
                    out.location().last_col  = column_;
                    throw TokenizerError("unexpected new line in string literal"sv, out.location());
                }
                else if(current_ch_ == ChDblQuote)
                {
                    out.setType(Token::String);
                    advance();
                    return;
                }
                else if(current_ch_ == ChBackSlash) // Escape sequence.
                {
                    advance();
                    switch(current_ch_)
                    {
                        case ChEol: break; // Escaped new line

                        case ChQuote:
                        case ChDblQuote:
                        case ChBackSlash: {
                            out.append(current_ch_);
                        } break;

                        case 'n': {
                            out.append(ChEol);
                        } break;

                        case 't': {
                            out.append(ChTab);
                        } break;

                        default:
                        {
                            out.location().last_line = line_;
                            out.location().last_col  = column_;
                            throw TokenizerError("unknown escape sequence"sv, out.location());
                        }
                    }
                }
                else {
                    out.append(current_ch_);
                }
            }
        }

        void peekNextToken(Token& out)
        {
            out.reserve(64);
            const auto pos = istream_.tell();
            const auto cch = current_ch_;
            const auto nch = next_ch_;
            const auto lin = line_;
            const auto col = column_;

            readToken(out);

            istream_.seek(pos);
            current_ch_ = cch;
            next_ch_    = nch;
            line_       = lin;
            column_     = col;
        }

        void readToken(Token& out)
        {
            out.reserve(64);
            skipWhitespace();

            out.clear();
            out.location().first_line = line_;
            out.location().first_col  = column_;

            if(current_ch_ == ChEof) { // Stream has reached end of file.
                out.setType(Token::EndOfFile);
            }
            else if(current_ch_ == ChEol) {
                out.setType(Token::EndOfLine);
                advance();
            }
            else if(current_ch_ == ChDblQuote) {
                readStringLiteral(out);
            }
            else if(isIdentifierLead(current_ch_)) {
                readIdentifier(out);
            }
            else if(std::isdigit(current_ch_)) {
                readNumericLiteral(out);
            }
            else if(std::ispunct(current_ch_))
            {
                if(current_ch_ == ChMinus && (next_ch_ == ChDecimalSep || std::isdigit(next_ch_))) {
                    readNumericLiteral(out);
                }
                else if(current_ch_ == ChDecimalSep && std::isdigit(next_ch_)) {
                    readNumericLiteral(out);
                }
                else
                {
                    out.append(current_ch_);
                    out.setType(Token::Punctuator);
                    advance();
                }
            }

            out.location().last_line = line_;
            out.location().last_col  = column_;
        }

        inline void setReportEol(bool report)
        {
            report_eol_ = report;
        }

        bool inline reportEol() const
        {
            return  report_eol_;
        }

        void skipToNextLine()
        {
            while(current_ch_ != ChEol && !istream_.atEnd()) {
               advance();
            }
        }

        inline bool skipWhitespaceStep()
        {
            if(current_ch_ == ChEof) {
                return false;
            }
            else if(report_eol_ && current_ch_ == ChEol) {
                return false;
            }
            else if(std::isspace(current_ch_))
            {
                advance();
                return true;
            }
            else if(current_ch_ == ChComment || // Skip comment line
                   (current_ch_ == ChComment2  && next_ch_ == ChComment2))
            {
                skipToNextLine();
                return true;
            }

            return false;
        }

        inline void skipWhitespace()
        {
            while(skipWhitespaceStep()) {
                // Repeatedly call skipWhitespaceStep() until it returns false.
                // Returning false indicates no more whitespace to skip.
            }
        }
    };
}

#endif // LIBIM_TOKENIZER_P_H
