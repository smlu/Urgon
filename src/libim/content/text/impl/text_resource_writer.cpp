#include "../text_resource_writer.h"
#include "text_resource_literals.h"

using namespace libim;
using namespace libim::text;
using namespace libim::content::text;
using namespace std::string_view_literals;

TextResourceWriter::TextResourceWriter(OutputStream& os) :
    ostream_(os)
{}

TextResourceWriter& TextResourceWriter::indent(std::size_t width, char indch)
{
    std::string indent(width, indch);
    return write(indent);
}

TextResourceWriter& TextResourceWriter::startNewRow(std::size_t idx)
{
    return indent(3).writeRowIdx(idx).indent(3);
}

TextResourceWriter& TextResourceWriter::write(std::string_view text)
{
    ostream_ << text;
    return  *this;
}

TextResourceWriter& TextResourceWriter::writeCommentLine(std::string_view comment)
{
    ostream_ << ChComment << ChSpace << comment;
    return writeEol();
}

TextResourceWriter& TextResourceWriter::writeEol()
{
    ostream_ << ChEol;
    return *this;
}

TextResourceWriter& TextResourceWriter::writeKey(std::string_view key, std::string_view value)
{
    ostream_ << key << ChSpace << value;
    return writeEol();
}

TextResourceWriter& TextResourceWriter::writeLabel(std::string_view name, std::string_view text)
{
    ostream_ << name << kResLabelPunc << ChSpace << text;
    return writeEol();
}

TextResourceWriter& TextResourceWriter::writeLine(std::string_view line)
{
    ostream_ << line;
    return writeEol();
}

TextResourceWriter& TextResourceWriter::writeRowIdx(std::size_t idx)
{
    ostream_ << convertToString(idx) << kResLabelPunc;
    return *this;
}

TextResourceWriter& TextResourceWriter::writeSection(std::string_view section)
{
    return writeLine(kResSectionHeader).
           writeLabel(kResSectionTag, section);
}
