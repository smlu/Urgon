#ifndef LIBIM_ANIM_TEST_H
#define LIBIM_ANIM_TEST_H
#include "../animation.h"
#include "../../../../io/filestream.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace libim::unit_test {

    void run_animation_tests(const std::filesystem::path& tvRootPath);
}
#endif // LIBIM_ANIM_TEST_H
