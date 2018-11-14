#ifndef LIBIM_TOKENIZER_P_H
#define LIBIM_TOKENIZER_P_H
#include "schars.h"
#include "../token.h"
#include "../tokenizer.h"
#include "../tokenizer_error.h"
#include "../diagnostic_location.h"
#include "../../io/stream.h"

#include <cctype>
#include <functional>
#include <string_view>

using namespace std::string_view_literals;
namespace libim::text {

    inline bool is_crlf(char c1, char c2)
    {
        return c1 == ChCr && c2 == ChEol;
    }


    class Tokenizer::TokenizerPrivate
    {
        InputStream& istream_;
        char current_ch_, next_ch_;
        std::size_t line_   = 1;
        std::size_t column_ = 1;
        bool report_eol_ = false;

    public:
        TokenizerPrivate(InputStream& s) :
            istream_(s)
        {
            current_ch_ = readNextChar();
            next_ch_    = readNextChar();
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
            return std::isalpha(c) || (c == ChIdentifier);
        }

        inline static bool isIdentifierChar(char c)
        {
            return std::isalnum(c) || (c == ChIdentifier);
        }

        void readString(Token& out, std::size_t len)
        {
            readDelimitedString(out, [&](char) {
                return (len--) == 0;
            });
        }

        void readLine(Token& out)
        {
            readDelimitedString(out, [&](char) {
                return isEol();
            });
        }

        void readDelimitedString(Token& out, const std::function<bool(char)>& isDelim)
        {
            skipWhitespace();

            out.clear();
            out.location().first_line = line_;
            out.location().first_col  = column_;

            while(!isDelim(current_ch_))
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
            while(std::isxdigit(current_ch_))
            {
                out.append(current_ch_);
                advance();
            }
        }

        void readNumericLiteralIntegerPart(Token& out)
        {
            while(std::isdigit(current_ch_))
            {
                out.append(current_ch_);
                advance();
            }
        }

        void readNumericLiteral(Token& out)
        {
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
            if(isIdentifierLead(current_ch_))
            {
                out.setType(Token::Identifier);
                do {
                    out.append(current_ch_);
                    advance();
                } while(isIdentifierChar(current_ch_));
            }
        }

        void readStringLiteral(Token& out)
        {
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

        void readToken(Token& out)
        {
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
            while(current_ch_ != ChEol && current_ch_ != ChEof) {
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
            else if(current_ch_ == ChComment) // Skip comment line
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
