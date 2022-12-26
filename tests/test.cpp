#include "libim/io/filestream.h"
#include "libim/content/text/text_resource_reader.h"
#include "libim/utils/utils.h"

#include <string>
#include <string_view>
#include <vector>

using namespace libim;
using namespace libim::content::text;
using namespace std::string_view_literals;

int main([[maybe_unused]]int argc, char** argv)
{
    TextResourceReader rr((InputFileStream(std::string(argv[1]))));
    rr.assertSection("HEADER");
    rr.assertKeyValue("3DO"sv, 2.3f);
    rr.assertSection("MODELRESOURCE");
    auto materials = rr.readList<std::vector<std::string>>("MATERIALS"sv, [](auto& rr, [[maybe_unused]] auto rowidx, auto& matName){
        matName = rr.getSpaceDelimitedString();
    });

    return 0;
}
