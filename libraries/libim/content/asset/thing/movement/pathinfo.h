#ifndef LIMIM_PATHINFO_H
#define LIMIM_PATHINFO_H
#include <vector>

#include <libim/math/vector3.h>
#include <libim/math/rotator.h>

namespace libim::content::asset {

    struct PathFrame final {
        Vector3f pos;
        FRotator orient;
        std::string toString() const
        {
            auto poser = pos.toString();
            *(--poser.end()) = ':';
            return poser + orient.toString().substr(1);
        }
    };

    struct PathInfo final {
        std::vector<PathFrame> pathFrames;
    };
}
#endif // LIMIM_PATHINFO_H
