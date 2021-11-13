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
        Ai        = 0x2,
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
        float unknown4;  // probably center x
        float unknown5;  // probably center y
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
        Vector3f pos;
        FRotator pyrOrient;
        int unknown;                        // Possible: padding
        int sectorNum;
        Thing::Type type;
        Flags<Thing::Flag> flags;
        CndThingMoveType moveType;
        CndThingControlType controlType;
        CndThingLight light;
        int32_t msLifeLeft;
        CndRdThingType rdThingType;
        CndResourceName rdThingFileName;    // [*.3do, *.spr, *.par]
        CndResourceName pupFileName;
        CndResourceName sndFileName;
        CndResourceName createThingName;    // The name of thing template that will be created when this thing is created
        CndResourceName cogScriptFileName;
        CndCollide collide;
        int perfLevel;  // performance level. Note: If greater then what the game is configured the thing won't be created
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
        int32_t msBlastTime;
        int32_t msBabyTime;
        int32_t msExpandTime;
        int32_t msFadeTime;
        float maxLight;
        std::array<CndResourceName, 16> aDebrisTemplateNames;
        Vector3f posSpriteStart;
        Vector3f posSpriteEnd;
        CndResourceName spriteThingName;
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
        CndResourceName materialFileName;
    };
    static_assert (sizeof(CndParticleInfo) == 100);


    // Thing control info
    struct CndAiControlInfo final // Used if controlType == Ai
    {
        CndResourceName aiFileName;
        std::vector<Vector3f> pathFrames;
    };

    // This struct is written in the cnd file and represents part of CndAiControlInfo
    struct CndAiControlInfoHeader final
    {
        CndResourceName aiFileName;
        int32_t         numPathFrames;
    };
    static_assert(sizeof(CndAiControlInfoHeader) == 68);
    static_assert(sizeof(Vector3f) == 12); // ai pathFrame


    // Thing info's variants
    using CndThingControlInfo = std::variant<std::monostate, CndAiControlInfo>;
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
        uint32_t sizeAiControlInfoList;
        uint32_t sizeAiPathFrameList;
    };

    static_assert(sizeof(CndThingParamListSizes) == 44);
}

#endif //LIBIM_CNDTHING_H
