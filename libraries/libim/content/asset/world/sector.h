#ifndef LIBIM_SECTOR_H
#define LIBIM_SECTOR_H
#include "../../../math/box.h"
#include "../../../math/color.h"
#include "../../../math/math.h"
#include "../../../math/vector2.h"
#include "../../../math/vector3.h"
#include "../../audio/sound.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace libim::content::asset {

    struct Sector final
    {
        enum Flag : uint32_t
        {
            None            = 0x0,
            NoGravity       = 0x1,
            Underwater      = 0x2,
            CogLinked       = 0x4,
            HasThrust       = 0x8,
            HideOnAutomap   = 0x10,
            NoActorEnter    = 0x20,
            Pit             = 0x40,
            Water           = 0x100,
            HasCollideBox   = 0x1000,
            Seen            = 0x4000  // Sector has been seen by player/camera. See: sol_bluegem.cog, pru_lightspike.cog, pru_voicelinewhipeagle.cog, pru_voiceline.cog
        };

        using Id = std::size_t;
        Id id;

        Flag flags;
        ColorRgb tint;

        int32_t  pvsIdx;
        Vector3f center;
        float    radius;
        Vector3f thrust;

        Box3f boundBox;
        Box3f collideBox;

        Color ambientLight;
        Color extraLight;
        Vector3f avgLightPos;  // average light position
        Color    avgLightInt;  // average light intensity
        Vector2f avgLightFalloff;

        struct AmbientSound {
            std::string sound;
            float volume;
        };
        std::optional<AmbientSound> ambientSound;

        std::vector<std::size_t> vertIdxs;

        struct {
            std::size_t firstIdx;
            std::size_t count; // number of surfaces from first idx this sector includes
        } surfaces;
    };

    inline bool operator == (const Sector::AmbientSound& lhs, const Sector::AmbientSound& rhs) {
        return lhs.sound == rhs.sound && lhs.volume == rhs.volume;
    }

    inline bool operator != (const Sector::AmbientSound& lhs, const Sector::AmbientSound& rhs) {
        return !(lhs == rhs);
    }

    inline bool operator == (const Sector& lhs, const Sector& rhs)
    {
        return cmpf(lhs.radius, rhs.radius)                   &&
               lhs.id                == rhs.id                &&
               lhs.flags             == rhs.flags             &&
               lhs.tint              == rhs.tint              &&
               lhs.center            == rhs.center            &&
               lhs.thrust            == rhs.thrust            &&
               lhs.boundBox          == rhs.boundBox          &&
               lhs.collideBox        == rhs.collideBox        &&
               lhs.ambientLight      == rhs.ambientLight      &&
               lhs.extraLight        == rhs.extraLight        &&
               lhs.avgLightPos       == rhs.avgLightPos       &&
               lhs.avgLightInt       == rhs.avgLightInt       &&
               lhs.avgLightFalloff   == rhs.avgLightFalloff   &&
               lhs.pvsIdx            == rhs.pvsIdx            &&
               lhs.ambientSound      == rhs.ambientSound      &&
               lhs.vertIdxs          == rhs.vertIdxs          &&
               lhs.surfaces.count    == rhs.surfaces.count    &&
               lhs.surfaces.firstIdx == rhs.surfaces.firstIdx;
    }

    inline bool operator != (const Sector& lhs, const Sector& rhs) {
        return !(lhs == rhs);
    }
}
#endif // LIBIM_SECTOR_H
