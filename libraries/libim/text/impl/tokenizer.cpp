#include "../tokenizer.h"
#include "tokenizer_p.h"
#include "../../log/log.h"
#include "../../utils/utils.h"
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

const Token& Tokenizer::currentToken() const
{
    return cachedTkn_;
}

const Token& Tokenizer::getNextToken(bool lowercased)
{
    getNextToken(cachedTkn_);
    if(lowercased){
        cachedTkn_.toLowercase();
    }
    return cachedTkn_;
}

void Tokenizer::getNextToken(Token& out)
{
    tp_->readToken(out);
}

const Token& Tokenizer::peekNextToken(bool lowercased)
{
    peekNextToken(peekedTkn_);
    if(lowercased){
        peekedTkn_.toLowercase();
    }
    return peekedTkn_;
}

void Tokenizer::peekNextToken(Token& out)
{
    tp_->peekNextToken(out);
}

std::string Tokenizer::getIdentifier()
{
    getNextToken(cachedTkn_);
    if(cachedTkn_.type() != Token::Identifier) {
        throw TokenizerError("expected identifier"sv, cachedTkn_.location());
    }

    return std::move(cachedTkn_).value();
}

std::string Tokenizer::getStringLiteral()
{
    getNextToken(cachedTkn_);
    if(cachedTkn_.type() != Token::String) {
        throw TokenizerError("expected string literal"sv, cachedTkn_.location());
    }
    return std::move(cachedTkn_).value();
}

const std::string& Tokenizer::getSpaceDelimitedString(bool throwIfEmpty)
{
    getSpaceDelimitedString(cachedTkn_, throwIfEmpty);
    return cachedTkn_.value();
}

void Tokenizer::getSpaceDelimitedString(Token& out, bool throwIfEmpty)
{
    getDelimitedString(out, [](char c) { return isspace(c); });
    if(throwIfEmpty && out.isEmpty()) {
        throw TokenizerError("expected string fragment"sv, out.location());
    }
}

const std::string& Tokenizer::getDelimitedString(const std::function<bool(char)>& isDelim)
{
    getDelimitedString(cachedTkn_, isDelim);
    return cachedTkn_.value();
}

void Tokenizer::getDelimitedString(Token& out, const std::function<bool(char)>& isDelim)
{
    tp_->readDelimitedString(out, isDelim);
}

void Tokenizer::getString(Token& out, std::size_t len)
{
    tp_->readString(out, len);
}

const std::string& Tokenizer::getString(std::size_t len)
{
    tp_->readString(cachedTkn_, len);
    return cachedTkn_.value();
}

void Tokenizer::assertIdentifier(std::string_view id)
{
    getNextToken(cachedTkn_);
    if(cachedTkn_.type() != Token::Identifier || !utils::iequal(cachedTkn_.value(), id))
    {
        LOG_DEBUG("assertIdentifier: expected '%', found '%'", id, cachedTkn_.value());
        throw TokenizerError("expected identifier"sv, cachedTkn_.location());
    }
}

void Tokenizer::assertPunctuator(std::string_view punc)
{
    getNextToken(cachedTkn_);
    if(cachedTkn_.type() != Token::Punctuator || cachedTkn_.value() != punc)
    {
        LOG_DEBUG("assertPunctuator: expected '%', found '%'", punc, cachedTkn_.value());
        throw TokenizerError("expected punctuator"sv, cachedTkn_.location());
    }
}

void Tokenizer::assertEndOfFile()
{
    getNextToken(cachedTkn_);
    if(cachedTkn_.type() != Token::EndOfFile) {
        throw TokenizerError("expected end of file"sv, cachedTkn_.location());
    }
}

void Tokenizer::skipNextToken()
{
    Token t;
    getNextToken(t);
}

bool Tokenizer::skipNextTokenIf(TypeMask<Token::Type> mask)
{
    Token t;
    using namespace utils;
    if (peekNextToken(t); TypeMask(t.type()) & mask)
    {
        getNextToken(t);
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
        getNextToken(t);
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
