#include "../tokenizer.h"
#include "tokenizer_p.h"
#include "../../log/log.h"
#include "../../utils/utils.h"
#include <string_view>

using namespace libim;
using namespace text;
using namespace std::string_view_literals;

Tokenizer::Tokenizer(InputStream& s)
{
    cachedTkn_.location().filename = s.name();
    tkns_ = std::make_unique<TokenizerPrivate>(s);
}

Tokenizer::~Tokenizer()
{}

Token Tokenizer::getToken()
{
    getToken(cachedTkn_);
    return cachedTkn_;
}

void Tokenizer::getToken(Token& out)
{
    tkns_->readToken(out);
}

std::string Tokenizer::getIdentifier()
{
    getToken(cachedTkn_);
    if(cachedTkn_.type() != Token::Identifier) {
        throw TokenizerError("expected identifier"sv, cachedTkn_.location());
    }

    return cachedTkn_.value();
}

std::string Tokenizer::getStringLiteral()
{
    getToken(cachedTkn_);
    if(cachedTkn_.type() != Token::String) {
        throw TokenizerError("expected string literal"sv, cachedTkn_.location());
    }

    return cachedTkn_.value();
}

std::string Tokenizer::getSpaceDelimitedString()
{
    getDelimitedString(cachedTkn_, [](char c) { return isspace(c); });
    if(cachedTkn_.isEmpty()) {
        throw TokenizerError("expected string fragment"sv, cachedTkn_.location());
    }

    return cachedTkn_.value();
}

void Tokenizer::getDelimitedString(Token& out, const std::function<bool(char)>& isDelim)
{
    tkns_->readDelimitedString(out, isDelim);
}

void Tokenizer::assertIdentifier(std::string_view id)
{
    getToken(cachedTkn_);
    if(cachedTkn_.type() != Token::Identifier || utils::iequal(cachedTkn_.value(), id))
    {
        LOG_DEBUG("expected '%', found '%'", id, cachedTkn_.value());
        throw TokenizerError("expected identifier"sv, cachedTkn_.location());
    }
}

void Tokenizer::assertPunctuator(std::string_view punc)
{
    getToken(cachedTkn_);
    if(cachedTkn_.type() != Token::Punctuator || cachedTkn_.value() != punc)
    {
        LOG_DEBUG("expected '%', found '%'", punc, cachedTkn_.value());
        throw TokenizerError("expected punctuator"sv, cachedTkn_.location());
    }
}

void Tokenizer::assertLabel(std::string_view label)
{
    assertIdentifier(label);
    assertPunctuator(":");
}

void Tokenizer::assertEndOfFile()
{
    getToken(cachedTkn_);
    if(cachedTkn_.type() != Token::EndOfFile) {
        throw TokenizerError("expected end of file"sv, cachedTkn_.location());
    }
}

void Tokenizer::skipToNextLine()
{
    tkns_->skipToNextLine();
}

void Tokenizer::setReportEol(bool report)
{
    tkns_->setReportEol(report);
}

bool Tokenizer::reportEol() const
{
    return tkns_->reportEol();
}
