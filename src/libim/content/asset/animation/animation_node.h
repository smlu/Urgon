#ifndef LIBIM_ANIMATION_NODE_H
#define LIBIM_ANIMATION_NODE_H
#include "keyframe.h"

#include <cstdint>
#include <vector>
#include <string>

namespace libim::content::asset {
    struct AnimationNode final
    {
        uint32_t id;
        std::string meshName;
        std::vector<Keyframe> frames;
    };
}
#endif // LIBIM_ANIMATION_NODE_H
