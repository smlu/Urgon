#include "../gradientcolor.h"
#include "../text_resource_reader.h"
#include "../text_resource_writer.h"

#include <libim/io/binarystream.h>
#include <string>

using namespace libim;
using namespace libim::content::text;

GradientColor::GradientColor(const std::string_view strcolor)
{
    InputBinaryStream istream(strcolor);
    TextResourceReader rr(istream);
    *this = rr.readGradientColor();
}

std::string GradientColor::toString() const
{
    std::string strcolor;
    strcolor.reserve(
        (16 * 4) * 10  + /* (16 * 4) * max float num char len */
        (16 * 4 - 1)   + /* 16 * 4  - 1 fwd. slashes */
        2                /* 2 parentheses */
    );

    OutputBinaryStream ostream(strcolor);
    TextResourceWriter wr(ostream);
    wr.writeGradientColor(*this);
    return strcolor;
}
