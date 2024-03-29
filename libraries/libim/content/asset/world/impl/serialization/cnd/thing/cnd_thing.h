#ifndef LIBIM_CNDTHING_H
#define LIBIM_CNDTHING_H
#include <cstdint>
#include <variant>

#include "../crn.h"
#include <libim/content/asset/thing/actor.h>
#include <libim/content/asset/thing/collide.h>
#include <libim/content/asset/thing/explosion.h>
#include <libim/content/asset/thing/damage_type.h>
#include <libim/content/asset/thing/item.h>
#include <libim/content/asset/thing/movement/pathinfo.h>
#include <libim/content/asset/thing/movement/physicsinfo.h>
#include <libim/content/asset/thing/particle.h>
#include <libim/content/asset/thing/thing.h>
#include <libim/content/asset/thing/weapon.h>
#include <libim/content/text/gradientcolor.h>
#include <libim/math/color.h>
#include <libim/math/rotator.h>
#include <libim/math/vector3.h>
#include <libim/types/flags.h>

namespace libim::content::asset {
    using namespace libim::content::text;

    enum class CndThingMoveType : uint32_t
    {
        None    = 0x0,
        Physics = 0x1,
        Path    = 0x2
    };

    enum class CndThingControlType : uint32_t
    {
        Plot      = 0x0,
        Player    = 0x1,
        AI        = 0x2,
        Explosion = 0x6,
        Particle  = 0x7
    };

    enum class CndRdThingType : uint32_t // Thing render type
    {
        RdModel    = 0x1,  // 3do
        RdSprite   = 0x4,  // spr
        RdParticle = 0x5,
        RdPolyline = 0x6
    };

    struct CndCollide final
    {
        Collide::Type type;
        float movesize;
        float size;
        float collideWidth;
        float collideHeight;
        float unkWidth;
        float unkHeight;
    };

    struct CndThingLight final
    {
        LinearColor color;
        LinearColor emitColor;
    };


    struct CndThingHeader
    {
        CndResourceName baseName;
        CndResourceName name;
        Vector3f position;
        FRotator pyrOrient;
        int32_t unknown;                        // Possible: padding
        int32_t sectorNum = -1;
        Thing::Type type;
        Flags<Thing::Flag> flags;
        CndThingMoveType moveType;
        CndThingControlType controlType;
        CndThingLight light;
        uint32_t msecLifeLeft;
        CndRdThingType rdThingType;
        CndResourceName rdThingFilename;    // [*.3do, *.spr, *.par]
        CndResourceName pupFilename;
        CndResourceName sndFilename;
        CndResourceName createThingTemplateName;    // The name of thing template that will be created when this thing is created
        CndResourceName cogScriptFilename;
        CndCollide collide;
        int performanceLevel;  // performance level. Note: If greater then what the game is configured the thing won't be created
    };
    static_assert(sizeof(CndThingHeader) == 568);


    // Thing movement info
    struct CndPhysicsInfo final
    {
        Flags<PhysicsInfo::Flag> flags;
        float mass;
        float height;
        float airDrag;
        float surfaceDrag;
        float staticDrag;
        Vector3f angularVelocity;
        Vector3f velocity;
        float maxRotationVelocity;
        float maxVelocity;
        float orientSpeed;
        float buoyancy;
    };
    static_assert(sizeof(CndPhysicsInfo) == 64);


    // Thing infos
    struct CndActorInfo final
    {
        Flags<Actor::Flag> flags;
        CndResourceName weaponTemplateName;
        float health;
        float maxHealth;
        float maxThrust;
        float maxRotThrust;
        float maxHeadVelocity;
        float maxHeadYaw;
        float jumpSpeed;
        Vector3f eyeOffset;
        float minHeadPitch;
        float maxHeadPitch;
        Vector3f fireOffset;
        Vector3f lightOffset;
        LinearColor lightIntensity;
        GradientColor voiceColor;
        CndResourceName explodeTemplateName;
    };
    static_assert (sizeof(CndActorInfo) == 284);


    struct CndWeaponInfo final
    {
        Flags<Weapon::Flag> flags;
        CndResourceName explosionTemplateName;
        float damage;
        float minDamage;
        float rate;
        DamageType damageType;
        float range;
        float force;
    };
    static_assert (sizeof(CndWeaponInfo) == 92);


    struct CndExplosionInfo final
    {
        Flags<ExplosionThing::Flag> flags;
        float damage;
        DamageType damageType;
        float range;
        float force;
        uint32_t msecBlastTime;
        uint32_t msecBabyTime;
        uint32_t msecExpandTime;
        uint32_t msecFadeTime;
        float maxLight;
        std::array<CndResourceName, 16> aDebrisTemplateNames;
        Vector3f spriteStartPos;
        Vector3f spriteEndPos;
        CndResourceName spriteTemplateName;
    };
    static_assert(sizeof(CndExplosionInfo) == 1152);


    struct CndItemInfo final
    {
      Flags<Item::Flag> flags;
      float secRespawnInterval;
    };
    static_assert (sizeof(CndItemInfo) == 8);


    using CndHintUserVal = float; // |= 0x40000 - means hint solved
    static_assert(sizeof(CndHintUserVal) == 4);


    struct  CndParticleInfo final
    {
        Flags<ParticleThing::Flag> flags;
        float growthSpeed;
        float minRadius;
        float maxRadius;
        float size;
        float timeoutRate;
        int numParticles;
        float pitchRange;
        float yawRange;
        CndResourceName materialFilename;
    };
    static_assert (sizeof(CndParticleInfo) == 100);


    // Thing control info
    struct CndAIControlInfo final // Used if controlType == AI
    {
        CndResourceName aiFileName;
        std::vector<Vector3f> pathFrames;
    };

    // This struct is written in the cnd file and represents part of CndAIControlInfo
    struct CndAIControlInfoHeader final
    {
        CndResourceName aiFileName;
        int32_t         numPathFrames;
    };
    static_assert(sizeof(CndAIControlInfoHeader) == 68);
    static_assert(sizeof(Vector3f) == 12); // ai pathFrame


    // Thing info's variants
    using CndThingControlInfo = std::variant<std::monostate, CndAIControlInfo>;
    using CndThingMoveInfo    = std::variant<std::monostate, CndPhysicsInfo, PathInfo>;
    using CndThingInfo        = std::variant<
        std::monostate,
        CndActorInfo,
        CndWeaponInfo,
        CndExplosionInfo,
        CndItemInfo,
        CndHintUserVal,
        CndParticleInfo
    >;


    struct CndThing final : CndThingHeader {
        CndThingControlInfo controlInfo;
        CndThingMoveInfo    moveInfo;
        CndThingInfo        thingInfo;

        inline void reset() {
            *this = CndThing{};
        }

        void init()
        {
            switch (type)
            {
                case Thing::Actor:
                case Thing::Player:
                {
                    if (!std::holds_alternative<CndActorInfo>(thingInfo)) {
                        thingInfo = CndActorInfo{};
                    }

                    CndActorInfo& actorInfo = std::get<CndActorInfo>(thingInfo);
                    if (actorInfo.voiceColor.top.isZero()) {
                        actorInfo.voiceColor.top = {{-1.0f, -1.0f, -1.0f, -1.0f}}; // Same as in original engine. Init. like this will make sure the RGBA don't get clamped to 0.0f
                    }
                }
                break;
            default:
                break;
            }

            if (controlType == CndThingControlType::AI)
            {
                const auto* aici = std::get_if<CndAIControlInfo>(&controlInfo);
                if (!aici || aici->aiFileName.isEmpty()) {
                    controlType = CndThingControlType::Plot;
                }
            }
        }
    };


    // Array of sizes (number of elements) of each list
    // that is written after thing header list
    struct CndThingParamListSizes final
    {
        uint32_t sizePhysicsInfoList;
        uint32_t sizeNumPathFramesList;
        uint32_t sizePathFrameList;
        uint32_t sizeActorInfoList;
        uint32_t sizeWeaponInfoList;
        uint32_t sizeExplosionInfoList;
        uint32_t sizeItemInfoList;
        uint32_t sizeParticleInfoList;
        uint32_t sizeHintUserValueList;
        uint32_t sizeAIControlInfoList;
        uint32_t sizeAIPathFrameList;
    };

    static_assert(sizeof(CndThingParamListSizes) == 44);
}

#endif //LIBIM_CNDTHING_H
