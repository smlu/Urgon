#include "anim_test.h"
#include "../../../../common.h"

using namespace libim;
using namespace libim::content::text;
using namespace libim::content::asset;
using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace std::filesystem;



void test_anim_file(const std::filesystem::path& filePath)
{
    const auto testFile = path(filePath).remove_filename().append("test.key"sv);

    try
    {
        Animation anim;
        {
            InputFileStream fs(filePath);
            TextResourceReader rr(fs);
            anim.load(rr);
        }

        // Write animation to file
        {
            OutputFileStream ofs(testFile, /*truncate=*/true);
            TextResourceWriter trw(ofs);
            anim.write(trw);
        }

        // Load written animation
        Animation anim2;
        {
            InputFileStream fs(testFile);
            anim2.load(TextResourceReader(fs));

            fs.close();
            RemoveFile(testFile);
        }

        // Test
        assert(anim.flages()  == anim2.flages());
        assert(anim.type()    == anim2.type());
        assert(anim.frames()  == anim2.frames());
        assert(cmpf(anim.fps() , anim2.fps()));
        assert(anim.joints()  == anim2.joints());
        assert(anim.markers() == anim2.markers());
        assert(anim.nodes()   == anim2.nodes());
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Animation unit test failed:", e.what());
        RemoveFile(testFile);
    }
}



void libim::unit_test::anim_test(const std::filesystem::path& tvRootPath )
{
    using namespace libim;
    using namespace libim::content::text;
    using namespace libim::content::asset;
    using namespace std::string_view_literals;
    using namespace std::string_literals;

    const auto tvFile1 = path(tvRootPath).append("test_vector.key"sv);
    const auto tvFile2 = path(tvRootPath).append("test_vector_no_markers.key"sv);

    test_anim_file(tvFile1);
    test_anim_file(tvFile2);
}
