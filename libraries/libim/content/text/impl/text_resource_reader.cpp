#include "../text_resource_reader.h"
#include "text_resource_literals.h"

#include <libim/text/impl/tokenizer_p.h>
#include <utility>

using namespace libim::content::asset;
using namespace libim::content::text;
using namespace std::string_view_literals;

void TextResourceReader::assertLabel(std::string_view label)
{
    assertIdentifier(label);
    assertPunctuator(kResLabelDelim);
}

template<typename VecT>
void readStrictVec(TextResourceReader& rr, VecT& vec)
 {
    using vet = typename VecT::value_type;
    auto a = rr.readArray<vet, VecT::size()>([](std::size_t i, TextResourceReader& rr) {
        if ( i > 0) rr.assertPunctuator("/");
        return rr.getNumber<vet>();
    });
    vec = static_cast<VecT&&>(std::move(a));
}

GradientColor TextResourceReader::readGradientColor()
{
    GradientColor color;
    assertPunctuator("(");
    readStrictVec(*this, color.top);
    assertPunctuator("/");
    readStrictVec(*this, color.middle);
    assertPunctuator("/");
    readStrictVec(*this, color.bottomLeft);
    assertPunctuator("/");
    readStrictVec(*this, color.bottomRight);
    assertPunctuator(")");
    return color;
}

PathFrame TextResourceReader::readPathFrame()
{
    PathFrame frame;
    assertPunctuator("(");
    readStrictVec(*this, frame.position);
    assertPunctuator(":");
    readStrictVec(*this, frame.orient);
    assertPunctuator(")");
    return frame;
}

void TextResourceReader::assertKey(std::string_view key)
{
    getString(cachedTkn_, key.size());
    if(!utils::iequal(key, cachedTkn_.value()))
    {
        LOG_DEBUG("assertKey: Expected key '%', found '%'", key, cachedTkn_.value());
        throw SyntaxError("Invalid key"sv, cachedTkn_.location());
    }
}

void TextResourceReader::readKey(std::string_view key, Token& t)
{
    assertKey(key);
    AT_SCOPE_EXIT([this, reol = reportEol()] {
        setReportEol(reol);
    });
    setReportEol(true);
    getNextToken(t);
}

std::string_view TextResourceReader::readLine()
{
    tp_->readLine(cachedTkn_);
    return cachedTkn_.value();
}

std::string_view TextResourceReader::readSection()
{
    assertLabel(kResName_Section);
    tp_->readLine(cachedTkn_);
    return cachedTkn_.value();
}

void TextResourceReader::assertSection(std::string_view section)
{
    const auto sectionName = readSection();
    if(!utils::iequal(sectionName, section))
    {
        LOG_DEBUG("Expected '%', found '%'", section, cachedTkn_.value());
        throw SyntaxError("Invalid section"sv, cachedTkn_.location());
    }
}

std::size_t TextResourceReader::readRowIdx()
{
    auto num = getNumber<std::size_t>();
    assertPunctuator(kResLabelDelim);
    return num;
}
