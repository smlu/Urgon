#include "../text_resource_writer.h"
#include "text_resource_literals.h"
#include <libim/math/math.h>

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

TextResourceWriter& TextResourceWriter::indent(std::size_t width)
{
    return indent(width, indch_);
}

TextResourceWriter& TextResourceWriter::write(std::string_view text)
{
    ostream_ << text;
    return  *this;
}

TextResourceWriter& TextResourceWriter::write(std::string_view text, std::size_t fieldWidth, std::size_t minSep, char indentChar)
{
    write(text);

    auto[min, max] = minmax(fieldWidth, clamp(text.size(), minSep, fieldWidth));
    auto indw      = ::max(minSep, max - min);
    indent(indw, indentChar);
    return *this;
}

TextResourceWriter& TextResourceWriter::writeEol()
{
    ostream_ << ChEol;
    return *this;
}

TextResourceWriter& TextResourceWriter::writeGradientColor(const GradientColor& color)
{
    write("(");
    writeNumericArray</*precision=*/6>(color.top, /*width=*/0, /*separator=*/ '/');          write("/");
    writeNumericArray</*precision=*/6>(color.middle, /*width=*/0, /*separator=*/ '/');       write("/");
    writeNumericArray</*precision=*/6>(color.bottomLeft, /*width=*/0, /*separator=*/ '/');   write("/");
    writeNumericArray</*precision=*/6>(color.bottomRight, /*width=*/0, /*separator=*/ '/');
    write(")");
    return *this;
}

TextResourceWriter& TextResourceWriter::writeKeyValue(std::string_view key, std::string_view value, std::size_t indent)
{
    write(key);
    this->indent(indent);
    write(value);
    return writeEol();
}

TextResourceWriter& TextResourceWriter::writeLabel(std::string_view name, std::string_view text)
{
    ostream_ << name << kResLabelDelim << indch_ << text;
    return writeEol();
}

TextResourceWriter& TextResourceWriter::writeLine(std::string_view line)
{
    ostream_ << line;
    return writeEol();
}

TextResourceWriter& TextResourceWriter::writePathFrame(const asset::PathFrame& frame)
{
    write("(");
    writeNumericArray</*precision=*/6>(frame.position, /*width=*/0, /*separator=*/ '/');
    write(":");
    writeNumericArray</*precision=*/6>(frame.orient, /*width=*/0, /*separator=*/ '/');
    write(")");
    return *this;
}

TextResourceWriter& TextResourceWriter::writeRowIdx(std::size_t idx, std::size_t indent)
{
    const auto strIdx = utils::to_string(idx);
    if(indent > 0)
    {
        auto[min, max] = minmax(indent, strIdx.size());
        this->indent(max - min);
    }

    ostream_ << strIdx << kResLabelDelim;
    return *this;
}

TextResourceWriter& TextResourceWriter::writeSection(std::string_view section, bool overline)
{
    if (overline) {
        writeLine(kResSectionHeader);
    }
    return writeLabel(kResName_Section, section);
}

char TextResourceWriter::commentChar() const
{
    return ChComment;
}

char TextResourceWriter::spaceChar() const
{
    return ChSpace;
}
