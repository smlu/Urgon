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

    inline constexpr bool operator == (const AnimationNode& n1, const AnimationNode& n2)
    {
        return n1.id == n2.id && n1.meshName == n2.meshName && n1.frames == n2.frames;
    }
}
#endif // LIBIM_ANIMATION_NODE_H
