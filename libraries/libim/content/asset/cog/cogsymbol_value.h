#ifndef LIBIM_COGSYMBOL_VALUE_H
#define LIBIM_COGSYMBOL_VALUE_H
#include <map>
#include <string>
#include <variant>

#include "cogmessage_type.h"
#include "../animation/animation.h"
#include "../material/material.h"
#include "../world/sector.h"
#include "../world/surface.h"
#include "../../audio/sound.h"

#include <libim/types/sharedref.h>
#include <libim/math/vector3.h>

namespace libim::content::asset {
    struct Cog;
    struct CogSubroutine {};
    using CogFlexType = std::variant<float, CogSubroutine>;
    using CogSymbolValue = std::variant<
        std::monostate,
        int32_t,
        float,
        std::string,
        Vector3f,
        CogFlexType,
        CogMessageType,
        SharedRef<audio::Sound>,
        SharedRef<Animation>,
        SharedRef<Material>,
        SharedRef<Surface>,
        SharedRef<Sector>,
        /* utils::SharedRef<Ai>,*/
        SharedRef<Cog>
        /*utils::SharedRef<Template>,
        utils::SharedRef<Thing>, */
    >;
}

#endif // LIBIM_COGSYMBOL_VALUE_H
