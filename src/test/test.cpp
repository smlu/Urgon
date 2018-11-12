#include "io/filestream.h"
#include "content/text/text_resource_reader.h"
#include "utils/utils.h"


using namespace libim;
using namespace libim::content::text;

int main([[maybe_unused]]int argc, char** argv)
{
    InputFileStream s(argv[1]);
    TextResourceReader rr(s);

    rr.assertSection("HEADER");
    rr.assertKey("3DO", 2.3f);
    rr.assertSection("MODELRESOURCE");
    auto materials = rr.readList<std::string>("MATERIALS", [](auto& rr){
        return rr.getSpaceDelimitedString();
    });

    return 0;
}
