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

#include "../../../utils/sharedref.h"
#include "../../../math/vector3.h"

namespace libim::content::asset {
    class Cog;
    struct CogSubrutine {};
    using CogFlexType = std::variant<float, CogSubrutine>;
    using CogSymbolValue = std::variant<
        std::monostate,
        int32_t,
        float,
        std::string,
        Vector3f,
        CogFlexType,
        CogMessageType,
        utils::SharedRef<audio::Sound>,
        utils::SharedRef<Animation>,
        utils::SharedRef<Material>,
        utils::SharedRef<Surface>,
        utils::SharedRef<Sector>,
        /* utils::SharedRef<Ai>,*/
        utils::SharedRef<Cog>
        /*utils::SharedRef<Template>,
        utils::SharedRef<Thing>, */
    >;
}

#endif // LIBIM_COGSYMBOL_VALUE_H
