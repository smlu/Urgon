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
        if ( i > 0) rr.assertIdentifier("/");
        return rr.getNumber<vet>();
    });
    vec = static_cast<VecT&&>(std::move(a));
}

GradientColor TextResourceReader::readGradientColor()
{
    GradientColor color;
    assertIdentifier("(");
    readStrictVec(*this, color.top);
    readStrictVec(*this, color.middle);
    readStrictVec(*this, color.bottomLeft);
    readStrictVec(*this, color.bottomRight);
    assertIdentifier(")");
    return color;
}

PathFrame TextResourceReader::readPathFrame()
{
    PathFrame frame;
    assertIdentifier("(");
    readStrictVec(*this, frame.position);
    assertIdentifier(":");
    readStrictVec(*this, frame.orient);
    assertIdentifier(")");
    return frame;
}


void TextResourceReader::assertKey(std::string_view key)
{
    getString(cachedTkn_, key.size());
    if(!utils::iequal(key, cachedTkn_.value()))
    {
        LOG_DEBUG("assertKey: expected key '%', found '%'", key, cachedTkn_.value());
        throw SyntaxError("invalid key"sv, cachedTkn_.location());
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
        LOG_DEBUG("expected '%', found '%'", section, cachedTkn_.value());
        throw SyntaxError("invalid section"sv, cachedTkn_.location());
    }
}

std::size_t TextResourceReader::readRowIdx()
{
    auto num = getNumber<std::size_t>();
    assertPunctuator(kResLabelDelim);
    return num;
}
