#ifndef LIBIM_NDY_THING_ISER_H
#define LIBIM_NDY_THING_ISER_H
#include "ndy_thing_ser_common.h"

#include "../ndy.h"
#include "../../world_ser_common.h"

#include <libim/types/optref.h>
#include <libim/utils/traits.h>

#include <map>
#include <string_view>
#include <type_traits>

namespace libim::content::asset {

    template<typename VT>
    bool ndyParseMapValue(const StringMap<const std::string_view, VT>& map, const Token& val, VT& v)
    {
        auto it = map.find(val.value());
        if (it == map.end()) {
            return false;
        }
        v = it->second;
        return true;
    }

    void ndyParseLight(CndThing& thing, const Token& val)
    {
        LinearColor color;
        try{
            color = LinearColor(val.value(), /*strict=*/true);
        }
        catch (...) {
            try{
                color = makeLinearColor(LinearColorRgb(val.value(), /*strict=*/true), 0.0f);
            }
            catch (...) {
                LOG_WARNING("NDY: Bad light param: %s", val.value());
                throw SyntaxError("Bad thing light param"sv, val.location());
            }
        }

        thing.light.color = color;
        thing.light.emitColor = color;
        thing.flags |= Thing::Flag::EmitsLight;
    }

    bool ndyParseThingParam(NdyThingParam param, const Token& value, CndThing& thing)
    {
        switch ( param )
        {
            case NdyThingParam::Type:
                if (!ndyParseMapValue(kThingTypeNameMap, value, thing.type)) {
                    throw SyntaxError("Unknown thing type", value.location());
                }

                switch (thing.type)
                {
                    case Thing::Actor:
                        thing.controlType = CndThingControlType::AI;
                        break;
                    case Thing::Explosion:
                        thing.controlType = CndThingControlType::Explosion;
                        break;
                    case Thing::Player:
                        thing.controlType = CndThingControlType::Player;
                        break;
                    case Thing::Particle:
                        thing.controlType = CndThingControlType::Particle;
                        break;
                    case Thing::Sprite:
                    case Thing::Polyline:
                        thing.controlType = CndThingControlType::Plot;
                        break;
                }
                return true;

            case NdyThingParam::Collide:
                thing.collide.type = value.getFlags<decltype(thing.collide.type)>();
                return true;

            case NdyThingParam::Move:
                if (!ndyParseMapValue(kCndThingMoveNameMap, value, thing.moveType)) {
                    throw SyntaxError("Unknown thing move type", value.location());
                }
                return true;

            case NdyThingParam::Size:
            {
                auto size = value.getNumber<decltype(thing.collide.size)>();
                if (size < 0) {
                    throw SyntaxError("Bad size value"sv, value.location());
                }
                thing.collide.size     = size;
                thing.collide.movesize = size;
                return true;
            }

            case NdyThingParam::ThingFlags:
                thing.flags = value.getFlags<decltype(thing.flags)>();
                return true;

            case NdyThingParam::Timer:
            {
                auto secLifeLeft = value.getNumber<float>();
                if (secLifeLeft < 0) {
                    throw SyntaxError("Bad timer value"sv, value.location());
                }
                thing.msLifeLeft = static_cast<decltype(thing.msLifeLeft)>(secLifeLeft * 1000.0f);
                return true;
            }

            case NdyThingParam::Light:
                ndyParseLight(thing, value);
                return true;

            case NdyThingParam::SoundClass:
                thing.sndFileName = CndResourceName(value.value());
                return true;

            case NdyThingParam::Model3d:
                thing.rdThingType     = CndRdThingType::RdModel;
                thing.rdThingFileName = CndResourceName(value.value());

                if (thing.collide.unkWidth == 0.0 )
                {
                    thing.collide.collideWidth = thing.collide.movesize;
                    thing.collide.unkWidth     = thing.collide.movesize;
                }

                if (thing.collide.unkHeight == 0.0 )
                {
                    thing.collide.collideWidth = thing.collide.movesize;
                    thing.collide.unkHeight    = thing.collide.movesize;
                }

                return true;

            case NdyThingParam::Sprite:
                thing.rdThingType     = CndRdThingType::RdSprite;
                thing.rdThingFileName = CndResourceName(value.value());
                return true;

            case NdyThingParam::Puppet:
                thing.pupFileName = CndResourceName(value.value());
                return true;

            case NdyThingParam::AIClass:
            {
                thing.controlType = CndThingControlType::AI;

                // Init actorInfo in case it's not already
                if (!std::holds_alternative<CndAIControlInfo>(thing.controlInfo)) {
                    thing.controlInfo = CndAIControlInfo{};
                }
                std::get<CndAIControlInfo>(thing.controlInfo)
                    .aiFileName = CndResourceName(value.value());
                return true;
            }

            case NdyThingParam::Cog:
                thing.flags |= Thing::Flag::CogLinked;
                thing.cogScriptFileName = CndResourceName(value.value());
                return true;

            case NdyThingParam::Particle:
                thing.rdThingType     = CndRdThingType::RdParticle;
                thing.rdThingFileName = CndResourceName(value.value());
                return true;

            case NdyThingParam::MoveSize:
                if (thing.type != Thing::Actor && thing.type != Thing::Player)
                {
                    auto movesize = value.getNumber<decltype(thing.collide.movesize)>();
                    if (movesize < 0.0) {
                        throw SyntaxError("Bad movesize value"sv, value.location());
                    }
                    thing.collide.movesize = movesize;
                }
                return true;

            case NdyThingParam::CreateThing:
                thing.createThingTemplateName = CndResourceName(value.value());
                return true;

            case NdyThingParam::Orient:
                thing.pyrOrient = FRotator(value.value());
                return true;

            case NdyThingParam::UserVal:
            {
                auto userval = value.getNumber<CndHintUserVal>();
                if (userval < 0.0) {
                    throw SyntaxError("Bad user value"sv, value.location());
                }
                thing.thingInfo = userval;
                return true;
            }

            case NdyThingParam::CollHeight:
            {
                auto height = value.getNumber<decltype(thing.collide.collideHeight)>();
                if (height < 0.0) {
                    throw SyntaxError("Bad collheight value"sv, value.location());
                }
                thing.collide.collideHeight = height;
                thing.collide.unkHeight     = height;
                return true;
            }

            case NdyThingParam::CollWidth:
            {
                auto width = value.getNumber<decltype(thing.collide.collideWidth)>();
                if (width < 0.0) {
                    throw SyntaxError("Bad collwidth value"sv, value.location());
                }
                thing.collide.collideWidth = width;
                thing.collide.unkWidth     = width;
                return true;
            }

            case NdyThingParam::PerformanceLevel:
                thing.performanceLevel = value.getNumber<decltype(thing.performanceLevel)>();
                return true;

            default:
                break;
        }

        return false; // param not parsed
    }

    bool ndyParseActorParam(NdyThingParam param, const Token& value, CndThing& thing)
    {
        // Init actorInfo in case it's not already
        if (!std::holds_alternative<CndActorInfo>(thing.thingInfo)) {
            thing.thingInfo = CndActorInfo{};
        }

        CndActorInfo& actorInfo = std::get<CndActorInfo>(thing.thingInfo);

        switch (param)
        {
            case NdyThingParam::TypeFlags:
                actorInfo.flags = value.getFlags<decltype(actorInfo.flags)>();
                return true;

            case NdyThingParam::Health:
            {
                auto health = value.getNumber<decltype(actorInfo.health)>();
                if (health < 0.0) {
                    throw SyntaxError("Bad actor health value"sv, value.location());
                }

                actorInfo.health = health;
                if (actorInfo.maxHealth < health) {
                    actorInfo.maxHealth = health;
                }
                return true;
            }

            case NdyThingParam::MaxThrust:
            {
                auto maxThrust = value.getNumber<decltype(actorInfo.maxThrust)>();
                if (maxThrust < 0.0) {
                    throw SyntaxError("Bad actor maxthrust value"sv, value.location());
                }
                actorInfo.maxThrust = maxThrust;
                return true;
            }

            case NdyThingParam::MaxRotThrust:
            {
                auto maxRotThrust = value.getNumber<decltype(actorInfo.maxRotThrust)>();
                if (maxRotThrust < 0.0) {
                    throw SyntaxError("Bad actor maxrotthrust value"sv, value.location());
                }
                actorInfo.maxRotThrust = maxRotThrust;
                return true;
            }

            case NdyThingParam::MaxHeadVel:
            {
                auto maxHeadVelocity = value.getNumber<decltype(actorInfo.maxHeadVelocity)>();
                if (maxHeadVelocity < 0.0) {
                    throw SyntaxError("Bad actor maxheadvel value"sv, value.location());
                }
                actorInfo.maxHeadVelocity = maxHeadVelocity;
                return true;
            }

            case NdyThingParam::MaxHeadYaw:
            {
                auto maxHeadYaw = value.getNumber<decltype(actorInfo.maxHeadYaw)>();
                if (maxHeadYaw < 0.0) {
                    throw SyntaxError("Bad actor maxheadyaw value"sv, value.location());
                }
                actorInfo.maxHeadYaw = maxHeadYaw;
                return true;
            }

            case NdyThingParam::JumpSpeed:
            {
                auto jumpSpeed = value.getNumber<decltype(actorInfo.jumpSpeed)>();
                if (jumpSpeed < 0.0) {
                    throw SyntaxError("Bad actor jumpspeed value"sv, value.location());
                }
                actorInfo.jumpSpeed = jumpSpeed;
                return true;
            }

            case NdyThingParam::Weapon:
                actorInfo.weaponTemplateName = CndResourceName(value.value());
                return true;

            case NdyThingParam::Explode:
                actorInfo.explodeTemplateName = CndResourceName(value.value());
                return true;

            case NdyThingParam::MaxHealth:
            {
                auto maxHealth = value.getNumber<decltype(actorInfo.maxHealth)>();
                if (maxHealth < 0.0) {
                    throw SyntaxError("Bad actor maxhealth value"sv, value.location());
                }
                actorInfo.maxHealth = maxHealth;
                return true;
            }

            case NdyThingParam::EyeOffset:
                actorInfo.eyeOffset = Vector3f(value.value(), /*strict=*/true);
                return true;

            case NdyThingParam::MinHeadPitch:
                actorInfo.minHeadPitch  = value.getNumber<decltype(actorInfo.minHeadPitch)>();
                return true;

            case NdyThingParam::MaxHeadPitch:
                actorInfo.maxHeadPitch  = value.getNumber<decltype(actorInfo.maxHeadPitch)>();
                return true;

            case NdyThingParam::FireOffset:
                actorInfo.fireOffset = Vector3f(value.value(), /*strict=*/true);
                return true;

            case NdyThingParam::LightOffset: // param defines head light position.
                actorInfo.lightOffset = Vector3f(value.value(), /*strict=*/true);
                thing.flags |= Thing::Flag::EmitsLight;
                return true;

            case NdyThingParam::LightIntensity: // head light
                actorInfo.lightIntensity = makeLinearColor(LinearColorRgb(value.value(), /*strict=*/true), 0.0f); // Note: alpha aka light range only set for minecar.
                thing.flags |= Thing::Flag::EmitsLight;
                return true;

            case NdyThingParam::VoiceColor:
                actorInfo.voiceColor = GradientColor(value.value());
                return true;

            default:
                break;
        }

        return false; // param not parsed
    }

    bool ndyParseWeaponParam(NdyThingParam param, const Token& value, CndThing& thing)
    {
        // Init weaponInfo in case it's not already
        if (!std::holds_alternative<CndWeaponInfo>(thing.thingInfo)) {
            thing.thingInfo = CndWeaponInfo{};
        }

        CndWeaponInfo& weaponInfo = std::get<CndWeaponInfo>(thing.thingInfo);

        switch (param)
        {
            case NdyThingParam::TypeFlags:
                weaponInfo.flags = value.getFlags<decltype(weaponInfo.flags)>();
                return true;

            case NdyThingParam::Damage:
                weaponInfo.damage = value.getNumber<decltype(weaponInfo.damage)>();
                return true;

            case NdyThingParam::MinDamage:
                weaponInfo.minDamage = value.getNumber<decltype(weaponInfo.minDamage)>();
                return true;

            case NdyThingParam::DamageClass:
                weaponInfo.damageType = value.getFlags<decltype(weaponInfo.damageType)>();
                return true;

            case NdyThingParam::Explode:
                weaponInfo.explosionTemplateName = CndResourceName(value.value());
                return true;

            case NdyThingParam::Force:
                weaponInfo.force = value.getNumber<decltype(weaponInfo.force)>();
                return true;

            case NdyThingParam::Range:
                weaponInfo.range = value.getNumber<decltype(weaponInfo.range)>();
                return true;

            case NdyThingParam::Rate:
                weaponInfo.rate = value.getNumber<decltype(weaponInfo.rate)>();
                return true;
        }
        return false; // param not parsed
    }

    bool ndyParseItemParam(NdyThingParam param, const Token& value, CndThing& thing)
    {
        // Init itemInfo in case it's not already
        if (!std::holds_alternative<CndItemInfo>(thing.thingInfo)) {
            thing.thingInfo = CndItemInfo{};
        }

        CndItemInfo& itemInfo = std::get<CndItemInfo>(thing.thingInfo);

        switch (param)
        {
            case NdyThingParam::TypeFlags:
                itemInfo.flags = value.getFlags<decltype(itemInfo.flags)>();
                return true;

            case NdyThingParam::Respawn:
                itemInfo.secRespawnInterval = value.getNumber<decltype(itemInfo.secRespawnInterval)>();
                return true;
        }
        return false; // param not parsed
    }

    bool ndyParseExplosionParam(NdyThingParam param, const Token& value, CndThing& thing)
    {
        // Init explosionInfo in case it's not already
        if (!std::holds_alternative<CndExplosionInfo>(thing.thingInfo)) {
            thing.thingInfo = CndExplosionInfo{};
        }

        CndExplosionInfo& explosionInfo = std::get<CndExplosionInfo>(thing.thingInfo);

        switch (param)
        {
            case NdyThingParam::TypeFlags:
                explosionInfo.flags = value.getFlags<decltype(explosionInfo.flags)>();
                return true;

            case NdyThingParam::Damage:
                explosionInfo.damage = value.getNumber<decltype(explosionInfo.damage)>();
                return true;

            case NdyThingParam::DamageClass:
                explosionInfo.damageType = value.getFlags<decltype(explosionInfo.damageType)>();
                return true;

            case NdyThingParam::BlastTime:
                using BlastTimeT = decltype(explosionInfo.msBlastTime);
                explosionInfo.msBlastTime = static_cast<BlastTimeT>(value.getNumber<float>() * 1000.0f);
                explosionInfo.flags |= ExplosionThing::HasBlastPhase;
                return true;

            case NdyThingParam::BabyTime:
                using BabyTimeT = decltype(explosionInfo.msBabyTime);
                explosionInfo.msBabyTime = static_cast<BabyTimeT>(value.getNumber<float>() * 1000.0f);
                explosionInfo.flags |= ExplosionThing::HasChildExplosion;
                return true;

            case NdyThingParam::Force:
                explosionInfo.force = value.getNumber<decltype(explosionInfo.force)>();
                explosionInfo.flags |= ExplosionThing::HasBlastPhase;
                return true;

            case NdyThingParam::MaxLight:
                explosionInfo.maxLight = value.getNumber<decltype(explosionInfo.maxLight)>();
                explosionInfo.flags |= ExplosionThing::VariableLight;
                return true;

            case NdyThingParam::Range:
                explosionInfo.range = value.getNumber<decltype(explosionInfo.range)>();
                explosionInfo.flags |= ExplosionThing::HasBlastPhase;
                return true;

            case NdyThingParam::FlashRGB:
                // Skipped since it's not used by the engine
                return true;

            case NdyThingParam::ExpandTime:
                using ExpandTimeT = decltype(explosionInfo.msExpandTime);
                explosionInfo.msExpandTime = static_cast<ExpandTimeT>(value.getNumber<float>() * 1000.0f);
                explosionInfo.flags |= ExplosionThing::ExpandTimeSet;
                return true;

            case NdyThingParam::FadeTime:
                using FadeTimeT = decltype(explosionInfo.msFadeTime);
                explosionInfo.msFadeTime = static_cast<FadeTimeT>(value.getNumber<float>() * 1000.0f);
                explosionInfo.flags |= ExplosionThing::FadeTimeSet;
                return true;

            case NdyThingParam::Debris:
                /*Note max 16 sprites can be used */
                for (auto& debris : explosionInfo.aDebrisTemplateNames)
                {
                    if (debris.isEmpty())
                    {
                        debris = CndResourceName(value.value());
                        break;
                    }

                }
                return true;

            case NdyThingParam::SpriteThing:
                explosionInfo.spriteTemplateName = CndResourceName(value.value());
                return true;

            case NdyThingParam::SpriteStart:
                explosionInfo.spriteStartPos = Vector3f(value.value(), /*strict=*/true);
                return true;

            case NdyThingParam::SpriteEnd:
                explosionInfo.spriteEndPos = Vector3f(value.value(), /*strict=*/true);
                return true;
        }

        return false; // param not parsed
    }

    bool ndyParseParticleParam(NdyThingParam param, const Token& value, CndThing& thing)
    {
        // Init particleInfo in case it's not already
        if (!std::holds_alternative<CndParticleInfo>(thing.thingInfo)) {
            thing.thingInfo = CndParticleInfo{};
        }

        CndParticleInfo& particleInfo = std::get<CndParticleInfo>(thing.thingInfo);

        switch (param)
        {
            case NdyThingParam::TypeFlags:
                particleInfo.flags = value.getFlags<decltype(particleInfo.flags)>();
                return true;

            case NdyThingParam::MaxThrust:
                particleInfo.growthSpeed = value.getNumber<decltype(particleInfo.growthSpeed)>();
                if (particleInfo.growthSpeed < 0.0f) {
                    throw SyntaxError("Bad particle maxthrust value"sv, value.location());
                }
                return true;

            case NdyThingParam::Range:
                particleInfo.maxRadius = value.getNumber<decltype(particleInfo.maxRadius)>();
                if (particleInfo.maxRadius < 0.0f) {
                    throw SyntaxError("Bad particle range value"sv, value.location());
                }
                return true;

            case NdyThingParam::Material:
                particleInfo.materialFilename = CndResourceName(value.value());
                return true;

            case NdyThingParam::Rate:
                particleInfo.timeoutRate = value.getNumber<decltype(particleInfo.timeoutRate)>();
                if (particleInfo.timeoutRate < 0.0f) {
                    throw SyntaxError("Bad particle rate value"sv, value.location());
                }
                return true;

            case NdyThingParam::Count:
                particleInfo.numParticles = value.getNumber<decltype(particleInfo.numParticles)>();
                if (particleInfo.numParticles > 256) {
                    throw SyntaxError("Bad particle count value"sv, value.location());
                }
                return true;

            case NdyThingParam::ElementSize:
                particleInfo.size = value.getNumber<decltype(particleInfo.size)>();
                if (particleInfo.size < 0.0f) {
                    throw SyntaxError("Bad particle elementsize value"sv, value.location());
                }
                return true;

            case NdyThingParam::MinSize:
                particleInfo.minRadius = value.getNumber<decltype(particleInfo.minRadius)>();
                if (particleInfo.minRadius < 0.0f) {
                    throw SyntaxError("Bad particle minsize value"sv, value.location());
                }
                return true;

            case NdyThingParam::PitchRange:
                particleInfo.pitchRange = value.getNumber<decltype(particleInfo.pitchRange)>();
                if (particleInfo.pitchRange < 0.0f) {
                    throw SyntaxError("Bad particle pitchrange value"sv, value.location());
                }
                return true;

            case NdyThingParam::YawRange:
                particleInfo.yawRange = value.getNumber<decltype(particleInfo.yawRange)>();
                if (particleInfo.yawRange < 0.0f) {
                    throw SyntaxError("Bad particle yawrange value"sv, value.location());
                }
                return true;
        }

        return false; // param not parsed
    }

    bool ndyParsePhysicsParam(NdyThingParam param, const Token& value, CndThing& thing)
    {
        // Init physicsInfo in case it's not already
        if (!std::holds_alternative<CndPhysicsInfo>(thing.moveInfo)) {
            thing.moveInfo = CndPhysicsInfo{};
        }

        CndPhysicsInfo& physicsInfo = std::get<CndPhysicsInfo>(thing.moveInfo);

        switch (param)
        {
            case NdyThingParam::SurfDrag:
            {
                auto surfDrag = value.getNumber<decltype(physicsInfo.surfaceDrag)>();
                if (surfDrag < 0.0f) {
                    throw SyntaxError("Bad physics surfdrag value"sv, value.location());
                }
                physicsInfo.surfaceDrag = surfDrag;
                return true;
            }

            case NdyThingParam::AirDrag:
            {
                auto airDrag = value.getNumber<decltype(physicsInfo.airDrag)>();
                if (airDrag < 0.0f) {
                    throw SyntaxError("Bad physics airdrag value"sv, value.location());
                }
                physicsInfo.airDrag = airDrag;
                return true;
            }

            case NdyThingParam::StaticDrag:
            {
                auto staticDrag = value.getNumber<decltype(physicsInfo.staticDrag)>();
                if (staticDrag < 0.0f) {
                    throw SyntaxError("Bad physics staticdrag value"sv, value.location());
                }
                physicsInfo.staticDrag = staticDrag;
                return true;
            }

            case NdyThingParam::Mass:
            {
                auto mass = value.getNumber<decltype(physicsInfo.mass)>();
                if (mass < 0.0f) {
                    throw SyntaxError("Bad physics mass value"sv, value.location());
                }
                physicsInfo.mass = mass;
                return true;
            }

            case NdyThingParam::Height:
            {
                auto height = value.getNumber<decltype(physicsInfo.height)>();
                if (height < 0.0f) {
                    throw SyntaxError("Bad physics height value"sv, value.location());
                }
                physicsInfo.height = height;
                return true;
            }

            case NdyThingParam::PhysicsFlags:
                physicsInfo.flags = value.getFlags<decltype(physicsInfo.flags)>();
                return true;

            case NdyThingParam::MaxRotVel:
            {
                auto maxRotVel = value.getNumber<decltype(physicsInfo.maxRotationVelocity)>();
                if (maxRotVel < 0.0f) {
                    throw SyntaxError("Bad physics maxrotvel value"sv, value.location());
                }
                physicsInfo.maxRotationVelocity = maxRotVel;
                return true;
            }

            case NdyThingParam::MaxVel:
            {
                auto maxVel = value.getNumber<decltype(physicsInfo.maxVelocity)>();
                if (maxVel < 0.0f) {
                    throw SyntaxError("Bad physics maxvel value"sv, value.location());
                }
                physicsInfo.maxVelocity = maxVel;
                return true;
            }

            case NdyThingParam::Vel:
                physicsInfo.velocity = Vector3f(value.value(), /*strict=*/true);
                return true;

            case NdyThingParam::AngularVel:
                physicsInfo.angularVelocity = Vector3f(value.value(), /*strict=*/true);
                return true;

            case NdyThingParam::OrientSpeed:
            {
                auto orientSpeed = value.getNumber<decltype(physicsInfo.orientSpeed)>();
                if (orientSpeed < 0.0f) {
                    throw SyntaxError("Bad physics orientspeed value"sv, value.location());
                }
                physicsInfo.orientSpeed = orientSpeed;
                return true;
            }
            case NdyThingParam::Buoyancy:
                physicsInfo.buoyancy = value.getNumber<decltype(physicsInfo.buoyancy)>();
                return true;

        }

        return false; // param not parsed
    }

    bool ndyParsePathParam(NdyThingParam param, const Token& value, CndThing& thing)
    {
        // Init pathInfo in case it's not already
        if (!std::holds_alternative<PathInfo>(thing.moveInfo)) {
            thing.moveInfo = PathInfo{};
        }

        PathInfo& pathInfo = std::get<PathInfo>(thing.moveInfo);

        switch (param)
        {
            case NdyThingParam::NumFrames:
                if (pathInfo.pathFrames.size() > 0) {
                    throw SyntaxError("Multiple numframes arguments found."sv, value.location());
                }
                pathInfo.pathFrames.reserve(value.getNumber<int>());
                return true;

            case NdyThingParam::Frame:
                // Note, since it's not needed this scope doesn't check if path array was allocated
                // or if there is enough space like the original code does.
                if (pathInfo.pathFrames.capacity() <= (pathInfo.pathFrames.size())) {
                    LOG_WARNING("Path frame param found either before numframes or the numframes param allocated too small buffer!"sv);
                }
                pathInfo.pathFrames.push_back(PathFrame(value.value()));
                return true;
        }

        return false; // param not parsed
    }

    bool ndyParseAIParam(NdyThingParam param, const Token& value, CndThing& thing)
    {
        // Init aiInfo in case it's not already
        if (!std::holds_alternative<CndAIControlInfo>(thing.controlInfo)) {
            thing.controlInfo = CndAIControlInfo{};
        }

        CndAIControlInfo& aiInfo = std::get<CndAIControlInfo>(thing.controlInfo);

        switch (param)
        {
            case NdyThingParam::NumFrames:
                if (aiInfo.pathFrames.size() > 0) {
                    throw SyntaxError("Multiple AI numframes arguments found."sv, value.location());
                }
                aiInfo.pathFrames.reserve(value.getNumber<int>());
                return true;

            case NdyThingParam::Frame:
                // Note, since it's not needed this scope doesn't check if path array was allocated
                // or if there is enough space like the original code does.
                if (aiInfo.pathFrames.capacity() <= (aiInfo.pathFrames.size())) {
                    LOG_WARNING("AI path frame param found either before numframes or the numframes param allocated too small buffer!"sv);
                }
                aiInfo.pathFrames.push_back(Vector3f(value.value(), /*strict=*/true));
                return true;
        }

        return false; // param not parsed
    }

    void ndyParseThingParams(TextResourceReader& rr, CndThing& thing)
    {
        AT_SCOPE_EXIT([&rr, reol = rr.reportEol()](){
            rr.setReportEol(reol);
        });
        rr.setReportEol(true);

        Token kt, vt;
        while (rr.readKeyValue(kt, vt))
        {
            auto it = kNdyThingParamNameMap.find(kt.value());
            if (it == kNdyThingParamNameMap.end())
            {
                const auto& loc = kt.location();
                LOG_WARNING("%:%:%: Unrecognized thing param '%'", loc.filename, loc.firstLine, loc.firstColumn, kt.value());
                continue;
            }

            try
            {
                bool parsed = ndyParseThingParam(it->second, vt, thing);
                if (!parsed)
                {
                    switch (thing.type)
                    {
                        case Thing::Actor:
                        case Thing::Player:
                            parsed = ndyParseActorParam(it->second, vt, thing);
                            break;
                        case Thing::Weapon:
                            parsed = ndyParseWeaponParam(it->second, vt, thing);
                            break;
                        case Thing::Item:
                            parsed = ndyParseItemParam(it->second, vt, thing);
                            break;
                        case Thing::Explosion:
                            parsed = ndyParseExplosionParam(it->second, vt, thing);
                            break;
                        case Thing::Particle:
                            parsed = ndyParseParticleParam(it->second, vt, thing);
                            break;
                    }
                }

                if (!parsed)
                {
                    if (thing.moveType == CndThingMoveType::Physics) {
                        parsed = ndyParsePhysicsParam(it->second, vt, thing);
                    }
                    else if (thing.moveType == CndThingMoveType::Path) {
                        parsed = ndyParsePathParam(it->second, vt, thing);
                    }

                    if (!parsed && thing.controlType == CndThingControlType::AI) {
                        parsed = ndyParseAIParam(it->second, vt, thing);
                    }

                    if (!parsed)
                    {
                        const auto& loc = kt.location();
                        LOG_WARNING("%:%:%: Unused thing param '%'", loc.filename, loc.firstLine, loc.firstColumn, kt.value());
                    }
                }
            }
            catch (const SyntaxError& e)
            {
                auto loc = vt.location();
                LOG_ERROR("%:%:%: Syntax error encountered while parsing thing param '%': Bad value format '%'", loc.filename, loc.firstLine, loc.firstColumn, kt.value(), vt.value());
                throw SyntaxError(e.what(), loc); // re-throw with new location because e can have local location to binary stream. (e.g parsing vector as Vector3f(vt.value()))
            }
            catch (const std::exception& e)
            {
                auto loc = vt.location();
                LOG_ERROR("%:%:%: Exception encountered while parsing thing param '%': %", loc.filename, loc.firstLine, loc.firstColumn, kt.value(), e.what());
                throw SyntaxError(e.what(), loc);
            }
        }
    }

    CndThing ndyParseTemplate(TextResourceReader& rr, const IndexMap<CndThing>& templates)
    {
        try
        {
            auto name = CndResourceName(rr.getSpaceDelimitedString(/*throwIfEmpty=*/true)); // Has to construct a new string or basetkn will overwrite it.
            if (auto tit = templates.find(name); tit != templates.end()) {
                return *tit;
            }

            Token basetkn;
            if (!rr.getSpaceDelimitedString(basetkn, /*throwIfEmpty=*/true)) {
                throw SyntaxError("Expected base template name"sv, basetkn.location());
            }

            CndThing thing{};
            if (auto tit = templates.find(basetkn.value()); tit != templates.end()) {
                thing = *tit;
            }
            else if( !basetkn.value().empty() && !utils::iequal(basetkn.value(), "none"))
            {
                auto loc = basetkn.location();
                LOG_WARNING("%:%:%: Based-on template '%' not found, using blank!", loc.filename, loc.firstLine, loc.firstColumn, basetkn.value());
                basetkn.setValue("");
            }

            thing.name     = name;
            thing.baseName = CndResourceName(basetkn.value());
            ndyParseThingParams(rr, thing);
            return thing;
        }
        catch (const SyntaxError& e)
        {
            auto loc = e.location();
            LOG_ERROR("%:%:%: Syntax error encountered while parsing template: %", loc.filename, loc.firstLine, loc.firstColumn, e.what());
            throw;
        }
        catch(const std::exception& e)
        {
            auto loc = rr.currentLocation();
            LOG_ERROR("%:%:%: Exception encountered while parsing template: %", loc.filename, loc.firstLine, loc.firstColumn, e.what());
            throw SyntaxError(e.what(), loc);
        }
    }

    CndThing ndyParseThing(TextResourceReader& rr, const IndexMap<CndThing>& templates)
    {
        try
        {
            [[maybe_unused]] auto thingNum = rr.readRowIdx();
            auto templateName = rr.getSpaceDelimitedString(/*throwIfEmpty=*/true);

            auto tit = templates.find(templateName);
            if (tit == templates.end()) {
                throw SyntaxError("Unknown template name!"sv, rr.currentLocation());
            }

            CndThing thing{ *tit };
            thing.baseName = CndResourceName(templateName);
            thing.name     = CndResourceName(rr.getSpaceDelimitedString(/*throwIfEmpty=*/true));

            // Read position, orientation and sector
            thing.position  = rr.readVector<Vector3f>(/*strict=*/false);
            thing.pyrOrient = rr.readVector<FRotator>(/*strict=*/false);
            thing.sectorNum = rr.getNumber<decltype(thing.sectorNum)>();

            ndyParseThingParams(rr, thing);
            return thing;
        }
        catch (const SyntaxError& e)
        {
            auto loc = e.location();
            LOG_ERROR("%:%:%: Syntax error encountered while parsing thing: %", loc.filename, loc.firstLine, loc.firstColumn, e.what());
            throw;
        }
        catch(const std::exception& e)
        {
            auto loc = rr.currentLocation();
            LOG_ERROR("%:%:%: Exception encountered while parsing thing: %", loc.filename, loc.firstLine, loc.firstColumn, e.what());
            throw SyntaxError(e.what(), loc);
        }
    }
}
#endif // LIBIM_NDY_THING_ISER_H