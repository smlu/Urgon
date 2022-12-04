#include "tokenizer_p.h"
#include "../tokenizer.h"

#include <libim/log/log.h>
#include <libim/utils/utils.h>
#include <string_view>

using namespace libim;
using namespace text;
using namespace std::string_view_literals;

Tokenizer::Tokenizer(const InputStream& s)
{
    cachedTkn_.location().filename = s.name();
    tp_ = std::make_unique<TokenizerPrivate>(s);
}

Tokenizer::~Tokenizer()
{}

ParseLocation Tokenizer::currentLocation()
{
    return ParseLocation{
        tp_->istream().name(),
        tp_->currentLine(),
        tp_->currentColumn(),
        tp_->currentLine(),
        tp_->currentColumn()
    };
}

const Token& Tokenizer::currentToken() const
{
    return cachedTkn_;
}

bool Tokenizer::getNextToken(Token& out, bool lowercased, bool cache)
{
    tp_->readToken(out);
    if (lowercased) out.toLowercase();
    if (cache) cachedTkn_ = out;
    return out.isValid();
}

const Token& Tokenizer::getNextToken(bool lowercased)
{
    getNextToken(cachedTkn_, lowercased);
    return cachedTkn_;
}

bool Tokenizer::getNextToken(Token& out, bool lowercased)
{
    return getNextToken(out, lowercased, true);
}

Token Tokenizer::peekNextToken(bool lowercased)
{
    Token t;
    peekNextToken(t, lowercased);
    return t;
}

bool Tokenizer::peekNextToken(Token& out, bool lowercased)
{
    tp_->peekNextToken(out);
    if (lowercased){
        out.toLowercase();
    }
    return out.isValid();
}

std::string_view Tokenizer::getIdentifier()
{
    getNextToken(cachedTkn_);
    if (cachedTkn_.type() != Token::Identifier) {
        throw SyntaxError("Expected identifier"sv, cachedTkn_.location());
    }

    return cachedTkn_.value();
}

std::string_view Tokenizer::getStringLiteral()
{
    getNextToken(cachedTkn_);
    if (cachedTkn_.type() != Token::String) {
        throw SyntaxError("Expected string literal"sv, cachedTkn_.location());
    }
    return cachedTkn_.value();
}

bool Tokenizer::getSpaceDelimitedString(Token& out, bool throwIfEmpty)
{
    getDelimitedString(out, [](char c) { return isspace(c); });
    if (throwIfEmpty && out.isEmpty()) {
        throw SyntaxError("Expected string fragment"sv, out.location());
    }
    return out.type() == Token::String;
}

std::string_view Tokenizer::getSpaceDelimitedString(bool throwIfEmpty)
{
    getSpaceDelimitedString(cachedTkn_, throwIfEmpty);
    return cachedTkn_.value();
}

bool Tokenizer::getDelimitedString(Token& out, const std::function<bool(char)>& isDelim)
{
    tp_->readDelimitedString(out, isDelim);
    cachedTkn_ = out;
    return out.type() == Token::String;
}

std::string_view Tokenizer::getDelimitedString(const std::function<bool(char)>& isDelim)
{
    getDelimitedString(cachedTkn_, isDelim);
    return cachedTkn_.value();
}

bool Tokenizer::getString(Token& out, std::size_t len)
{
    tp_->readString(out, len);
    cachedTkn_ = out;
    return out.type() == Token::String;
}

std::string_view Tokenizer::getString(std::size_t len)
{
    getString(cachedTkn_, len);
    return cachedTkn_.value();
}

void Tokenizer::assertIdentifier(std::string_view id)
{
    getNextToken(cachedTkn_);
    if(cachedTkn_.type() != Token::Identifier || !utils::iequal(cachedTkn_.value(), id))
    {
        LOG_DEBUG("assertIdentifier: expected '%', found '%'", id, cachedTkn_.value());
        throw SyntaxError("expected identifier"sv, cachedTkn_.location());
    }
}

void Tokenizer::assertPunctuator(std::string_view punc)
{
    getNextToken(cachedTkn_);
    if(cachedTkn_.type() != Token::Punctuator || cachedTkn_.value() != punc)
    {
        LOG_DEBUG("assertPunctuator: expected '%', found '%'", punc, cachedTkn_.value());
        throw SyntaxError("Expected punctuator"sv, cachedTkn_.location());
    }
}

void Tokenizer::assertEndOfFile()
{
    getNextToken(cachedTkn_);
    if (cachedTkn_.type() != Token::EndOfFile) {
        throw SyntaxError("Expected end of file"sv, cachedTkn_.location());
    }
}

void Tokenizer::skipNextToken()
{
    Token t;
    getNextToken(t, /*lowercased=*/false, /*cache=*/false);
}

bool Tokenizer::skipNextTokenIf(TypeMask<Token::Type> mask)
{
    Token t;
    using namespace utils;
    if (peekNextToken(t); TypeMask(t.type()) & mask)
    {
        skipNextToken();
        return true;
    }
    return false;
}

bool Tokenizer::skipNextTokenIfNot(TypeMask<Token::Type> mask)
{
    Token t;
    using namespace utils;
    if (peekNextToken(t); !(TypeMask(t.type()) & mask))
    {
        skipNextToken();
        return true;
    }
    return false;
}

void Tokenizer::skipToNextLine()
{
    tp_->skipToNextLine();
}

void Tokenizer::setReportEol(bool report)
{
    tp_->setReportEol(report);
}

bool Tokenizer::reportEol() const
{
    return tp_->reportEol();
}

const InputStream& Tokenizer::istream() const
{
    return tp_->istream();
}
