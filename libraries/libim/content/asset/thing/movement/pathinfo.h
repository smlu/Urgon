#ifndef LIBIM_PATHINFO_H
#define LIBIM_PATHINFO_H
#include <vector>
#include <string_view>

#include <libim/math/vector3.h>
#include <libim/math/rotator.h>

namespace libim::content::asset {

    struct PathFrame final
    {
        Vector3f position;
        FRotator orient;

        PathFrame() = default;
        PathFrame(Vector3f pos, FRotator rot)
            : position(std::move(pos))
            , orient(std::move(rot))
        {}
        PathFrame(std::string_view strpath); // defined in text_resource_reader.h

        PathFrame(const PathFrame&) = default;
        PathFrame(PathFrame&&) = default;
        PathFrame& operator=(const PathFrame&) = default;
        PathFrame& operator=(PathFrame&&) = default;
    };

    struct PathInfo final
    {
        std::vector<PathFrame> pathFrames;
    };
}
#endif // LIBIM_PATHINFO_H
