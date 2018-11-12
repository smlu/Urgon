#include "../resource_reader.h"
#include "resource_literals.h"

#include <utility>

using namespace libim::text;
using namespace std::string_view_literals;

void ResourceReader::readKey(std::string_view key, Token& t)
{
    getSpaceDelimitedString(cachedTkn_);
    if(!utils::iequal(key, cachedTkn_.value()))
    {
        LOG_DEBUG("readNextKey: expected key '%', found '%'", key, cachedTkn_.value());
        throw TokenizerError("invalid key"sv, cachedTkn_.location());
    }

    const bool bReportEol = reportEol();
    setReportEol(true);
    getSpaceDelimitedString(t);
    setReportEol(bReportEol);
}

void ResourceReader::assertSection(std::string_view section)
{
    readSection(cachedTkn_);
    if(!utils::iequal(cachedTkn_.value(), section))
    {
        LOG_DEBUG("expected '%', found '%'", section, cachedTkn_.value());
        throw TokenizerError("invalid section"sv, cachedTkn_.location());
    }
}

std::string ResourceReader::readSection()
{
    readSection(cachedTkn_);
    return std::move(cachedTkn_).value();
}

void ResourceReader::readSection(Token& t)
{
    assertLabel(kResSection);
    getToken(t);
}
