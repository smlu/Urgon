#include "io/filestream.h"
#include "text/resource_reader.h"
#include "utils/utils.h"


using namespace libim;
using namespace libim::text;

int main([[maybe_unused]]int argc, char** argv)
{
    InputFileStream s(argv[1]);
    ResourceReader rr(s);

    rr.assertSection("HEADER");
    rr.assertKey("3DO", 2.3f);
    rr.assertSection("MODELRESOURCE");
    auto materials = rr.readList<std::string>("MATERIALS", [](auto& rr){
        return rr.getSpaceDelimitedString();
    });

    return 0;
}
