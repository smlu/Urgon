#ifndef LIBIM_SECTOR_H
#define LIBIM_SECTOR_H
#include <libim/content/audio/sound.h>
#include <libim/content/asset/primitives/box.h>
#include <libim/math/color.h>
#include <libim/math/math.h>
#include <libim/math/vector2.h>
#include <libim/math/vector3.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace libim::content::asset {

    struct SectorLight final
    {
        Vector3f position;
        LinearColor color;
        float falloffMin; // light falloff, min = 2 * radius in most cases
        float falloffMax;
    };

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
            AdjointsNotSet  = 0x80,   // flag set when cog fuction SetSectorAdjoins(sector, visible=false) is called and unset with SetSectorAdjoins(sector, visible=true).
            Aetherium       = 0x100,
            HasCollideBox   = 0x1000, // When this flag is set the engine verifies the object/point is within the sector by checking if position of object radius is within collision box (sithIntersect_IsSphereInSector)
            Seen            = 0x4000, // Sector has been seen by player/camera. 
                                      //  See: sol_bluegem.cog, pru_lightspike.cog, pru_voicelinewhipeagle.cog, pru_voiceline.cog
            SyncSector      = 0x8000  // Internal engine flag set when sector should be synchronized over net
        };

        using Id = std::size_t;
        Id id;

        Flags<Flag> flags;
        LinearColorRgb tint;

        int32_t  pvsIdx;
        Vector3f center;
        float    radius;
        Vector3f thrust;

        Box3f boundBox;
        Box3f collideBox;

        LinearColor ambientLight;
        LinearColor extraLight;
        SectorLight avgLight; // point light info

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
        return cmpf(lhs.radius, rhs.radius)                        &&
               lhs.id                   == rhs.id                  &&
               lhs.flags                == rhs.flags               &&
               lhs.tint                 == rhs.tint                &&
               lhs.center               == rhs.center              &&
               lhs.thrust               == rhs.thrust              &&
               lhs.boundBox             == rhs.boundBox            &&
               lhs.collideBox           == rhs.collideBox          &&
               lhs.ambientLight         == rhs.ambientLight        &&
               lhs.extraLight           == rhs.extraLight          &&
               lhs.avgLight.position    == rhs.avgLight.position   &&
               lhs.avgLight.color       == rhs.avgLight.color      &&
               lhs.avgLight.falloffMin  == rhs.avgLight.falloffMin &&
               lhs.avgLight.falloffMax  == rhs.avgLight.falloffMax &&
               lhs.pvsIdx               == rhs.pvsIdx              &&
               lhs.ambientSound         == rhs.ambientSound        &&
               lhs.vertIdxs             == rhs.vertIdxs            &&
               lhs.surfaces.count       == rhs.surfaces.count      &&
               lhs.surfaces.firstIdx    == rhs.surfaces.firstIdx;
    }

    inline bool operator != (const Sector& lhs, const Sector& rhs) {
        return !(lhs == rhs);
    }
}
#endif // LIBIM_SECTOR_H
