#ifndef LIBIM_NDY_THING_SER_HELPERS_H
#define LIBIM_NDY_THING_SER_HELPERS_H
#include "ndy_thing_ser_common.h"
#include "../ndy.h"
#include "../../world_ser_common.h"
#include <libim/types/optref.h>
#include <libim/utils/traits.h>

#include <map>
#include <string_view>
#include <type_traits>


namespace libim::content::asset {

    // Macros for base and param prediction. To be used in NdyWriteThingParamIf.
    #define ndyWriteIfBase(x) [&](const auto& base){ return x; }
    #define ndyWriteIfPara(x) [&](){ return x; }

    // Macro defines new ndy serialization function to write thing parameter.
    // param: name            - name of function
    // param: basePred        - condition for the parm to be written if base template is present
    // param: paramPred       - condition for the parm to be written if base template is not present
    // param: paramType       - NdyThingParam parameter type
    // param: paramWriteValue - string parameter value to be written
    #define DefNdyWriteThingParamFunc(name, basePred, paramPred, paramType, paramWriteValue) \
        DefNdyWriteThingParamFuncEx(name, basePred, paramPred, [&](TextResourceWriter& rw) { \
            ndyWriteThingParam(rw, paramType, paramWriteValue); })

    /*void ndyWriteThing##name(TextResourceWriter& rw, const CndThing& t, OptionalRef<const CndThing> baseTemplate) { \
            NdyWriteThingParamIf(baseTemplate,   \
                ndyWriteIfBase(basePred),        \
                ndyWriteIfPara(paramPred), [&] { \
                ndyWriteThingParam(rw, paramType, paramWriteValue); \
            });}*/

    // Macro defines new ndy serialization function to write thing parameter.
    // param: name            - name of function
    // param: basePred        - condition for the parm to be written if base template is present
    // param: paramPred       - condition for the parm to be written if base template is not present
    // param: paramWriteFun   - Function which writes parameter. Function should have 1 parameter of type TextResourceWriter&
    #define DefNdyWriteThingParamFuncEx(name, basePred, paramPred, paramWriteFun) \
        void ndyWriteThing##name(TextResourceWriter& rw, const CndThing& t, OptionalRef<const CndThing> baseTemplate) { \
            ndyWriteThingParamIf(baseTemplate,   \
                ndyWriteIfBase(basePred),        \
                ndyWriteIfPara(paramPred), [&](){ paramWriteFun(rw); });}

    // Returns True if
    // - base (template) value is available and it's value satisfies basePred
    // - or base value is not present and paramPred is satisfied. Otherwise returns false;
    template<typename BaseT, typename LambdaBasePred, typename LambdaParamPred>
    bool ndyCanWriteThingParam(OptionalRef<const BaseT> baseVal, LambdaBasePred&& basePred, LambdaParamPred&& paramPred) {
       return (baseVal.hasValue() && basePred(baseVal.value())) || (!baseVal && paramPred());
    }

    // Function serializes thing param using writeParam function.
    // Param is written only if basePred or paramPred (depending if baseVal is available) satisfy ndyCanWriteThingParam function.
    template<typename BaseT, typename LambdaBasePred, typename LambdaParamPred, typename ParamWriter>
    void ndyWriteThingParamIf(OptionalRef<const BaseT> baseVal, LambdaBasePred&& basePred, LambdaParamPred&& paramPred, ParamWriter&& writeParam) {
       if(ndyCanWriteThingParam(baseVal,
          std::forward<LambdaBasePred>(basePred),
          std::forward<LambdaParamPred>(paramPred))) {
           writeParam();
       }
    }

    // Function serializes thing param and it's value to rw stream.
    void ndyWriteThingParam(TextResourceWriter& rw, NdyThingParam param, std::string_view value)
    {
        rw.write(kNdyThingParamMap.at(param));
        rw.write("=");
        rw.write(value.empty() ? kNone : value);
        rw.indent(1);
    }

    // Overload for numbers and enums
    template<std::size_t base = 10, typename T,
        typename std::enable_if_t<utils::isEnum<T> || std::is_arithmetic_v<T>>* = nullptr>
    void ndyWriteThingParam(TextResourceWriter& rw, NdyThingParam param, T value)
    {
        using namespace libim::utils;
        if constexpr(utils::isEnum<T>) {
            ndyWriteThingParam<base>(rw, param, to_underlying(value));
        }
        else
        {
            constexpr std::size_t precision = std::is_floating_point_v<T> ? 6 : 0;
            ndyWriteThingParam(rw, param, to_string<base, precision>(value));
        }
    }

    // Helper function for DefNdyWriteThingParamFunc macro, when enum has to be written as HEX string.
    template<typename T,
        typename std::enable_if_t<utils::isEnum<T>>* = nullptr>
    std::string ndyWriteAsFlag(T f)
    {
        using namespace utils;
        return to_string<16>(to_underlying(f));
    }


    //-----------------------------------------------------------------//
    // Helper functions for serializing thing params and their values. //
    //-----------------------------------------------------------------//

    DefNdyWriteThingParamFunc(Flags,
        t.flags != base.flags,
        t.flags != Thing::Flag::None,
        NdyThingParam::ThingFlags,
        ndyWriteAsFlag(t.flags)
    )

    DefNdyWriteThingParamFunc(Cog,
        t.cogScriptFileName != base.cogScriptFileName && !t.cogScriptFileName.isEmpty(),
        !t.cogScriptFileName.isEmpty(),
        NdyThingParam::Cog,
        t.cogScriptFileName
    )

    DefNdyWriteThingParamFunc(CollideType,
        t.collide.type != base.collide.type,
        t.collide.type != Collide::None,
        NdyThingParam::Collide,
        t.collide.type
    )

    DefNdyWriteThingParamFunc(CollHeight,
        t.collide.collideHeight != base.collide.collideHeight,
        t.collide.collideHeight != 0,
        NdyThingParam::CollHeight,
        t.collide.collideHeight
    )

    DefNdyWriteThingParamFunc(CollWidth,
        t.collide.collideWidth != base.collide.collideWidth,
        t.collide.collideWidth != 0,
        NdyThingParam::CollWidth,
        t.collide.collideWidth
    )

    DefNdyWriteThingParamFunc(CreateThing,
        t.createThingName != base.createThingName && !t.createThingName.isEmpty(),
        !t.createThingName.isEmpty(),
        NdyThingParam::CreateThing,
        t.createThingName
    )

    DefNdyWriteThingParamFuncEx(Light,
        t.light.color != base.light.color,
        !t.light.color.isZero(),
        [&](TextResourceWriter&) {
            // Writes: light=(r/g/b/a) light=(r/g/b)
            ndyWriteThingParam(rw, NdyThingParam::Light, t.light.color.toString());
            ndyWriteThingParam(rw, NdyThingParam::Light, makeLinearColorRgb(t.light.emitColor).toString());
        }
    )

    DefNdyWriteThingParamFunc(MoveSize,
        t.collide.movesize != base.collide.movesize,
        t.collide.movesize != 0,
        NdyThingParam::MoveSize,
        t.collide.movesize
    )

    DefNdyWriteThingParamFunc(MoveType,
        t.moveType != base.moveType,
        t.moveType != CndThingMoveType::None,
        NdyThingParam::Move,
        kCndThingMoveTypeMap.at(t.moveType)
    )

    DefNdyWriteThingParamFunc(Orient,
        t.pyrOrient != base.pyrOrient,
        !t.pyrOrient.isZero(),
        NdyThingParam::Orient,
        t.pyrOrient.toString()
    )

    DefNdyWriteThingParamFunc(PerfLevel,
        t.perfLevel != base.perfLevel,
        t.perfLevel != 0,
        NdyThingParam::Perflevel,
        t.perfLevel
    )

    DefNdyWriteThingParamFunc(Puppet,
        t.pupFileName != base.pupFileName && !t.pupFileName.isEmpty(),
        !t.pupFileName.isEmpty(),
        NdyThingParam::Puppet,
        t.pupFileName
    )

    DefNdyWriteThingParamFuncEx(RdFileName,
        t.rdThingFileName != base.rdThingFileName && !t.rdThingFileName.isEmpty(),
        !t.rdThingFileName.isEmpty(),
        [&](TextResourceWriter&) {
            // Writes render file name e.g. model3d=*.3do or sprite=*.spr or particle=*.par
            if(t.rdThingType == CndRdThingType::RdModel) {
                ndyWriteThingParam(rw, NdyThingParam::Model3d, t.rdThingFileName);
            }
            else if(t.rdThingType == CndRdThingType::RdSprite) {
                ndyWriteThingParam(rw, NdyThingParam::Sprite, t.rdThingFileName);
            }
            else if(t.rdThingType == CndRdThingType::RdParticle) {
                ndyWriteThingParam(rw, NdyThingParam::Particle, t.rdThingFileName);
            }
        }
    )

    DefNdyWriteThingParamFunc(Size,
        t.collide.size != base.collide.size,
        t.collide.size != 0,
        NdyThingParam::Size,
        t.collide.size
    )

    DefNdyWriteThingParamFunc(SoundClass,
        t.sndFileName != base.sndFileName && !t.sndFileName.isEmpty(),
        !t.sndFileName.isEmpty(),
        NdyThingParam::SoundClass,
        t.sndFileName
    )

    DefNdyWriteThingParamFunc(Timer,
        t.msLifeLeft != base.msLifeLeft,
        t.msLifeLeft != 0,
        NdyThingParam::Timer,
        t.msLifeLeft * 0.001 // in sec
    )

    DefNdyWriteThingParamFunc(Type,
        t.type != base.type,
        t.type > 0,
        NdyThingParam::Type,
        kThingTypeMap.at(t.type)
    )

    void ndyWriteThingControlInfo(TextResourceWriter& rw, const CndThing& t, OptionalRef<const CndThing> baseTemplate)
    {
        using namespace utils;
        std::visit(overloaded {
            [](std::monostate ){},
            [&](const CndAiControlInfo& aiInfo)
            {
                OptionalRef<const CndAiControlInfo> baseAiInfo;
                if(baseTemplate && std::holds_alternative<CndAiControlInfo>(baseTemplate->controlInfo)) {
                   baseAiInfo = std::get<CndAiControlInfo>(baseTemplate->controlInfo);
                }

                ndyWriteThingParamIf(baseAiInfo,
                   ndyWriteIfBase(aiInfo.aiFileName != base.aiFileName),
                   ndyWriteIfPara(!aiInfo.aiFileName.isEmpty()), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::AiClass, aiInfo.aiFileName);
                });

                // Writes: numframes=x frame=(f/f/f) frame=(f/f/f) ...
                if(!aiInfo.pathFrames.empty()) {
                    ndyWriteThingParam(rw, NdyThingParam::NumFrames, aiInfo.pathFrames.size());
                    for( const auto& p : aiInfo.pathFrames) {
                        ndyWriteThingParam(rw, NdyThingParam::Frame, p.toString());
                    }
                }
           }
        }, t.controlInfo);
    }

    void ndyWriteThingMoveInfo(TextResourceWriter& rw, const CndThing& t, OptionalRef<const CndThing> baseTemplate)
    {
        using namespace libim::utils;
        std::visit(overloaded {
            [](std::monostate ){},
            [&](const PathInfo& pi)
            {
                OptionalRef<const PathInfo> pathInfo;
                if(baseTemplate && std::holds_alternative<PathInfo>(baseTemplate->moveInfo)) {
                    pathInfo = std::get<PathInfo>(baseTemplate->moveInfo);
                }

                // Writes: numframes=x frame=(f/f/f:f/f/f) frame=(f/f/f:f/f/f) ...
                if(!pi.pathFrames.empty()) {
                    ndyWriteThingParam(rw, NdyThingParam::NumFrames, pi.pathFrames.size());
                    for( const auto& p : pi.pathFrames) {
                        ndyWriteThingParam(rw, NdyThingParam::Frame, p.toString());
                    }
                }
            },
            [&](const CndPhysicsInfo& pi)
            {
                OptionalRef<const CndPhysicsInfo> physInfo;
                if(baseTemplate && std::holds_alternative<CndPhysicsInfo>(baseTemplate->moveInfo)) {
                    physInfo = std::get<CndPhysicsInfo>(baseTemplate->moveInfo);
                }

                // Param SurfDrag
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.surfaceDrag != base.surfaceDrag),
                    ndyWriteIfPara(pi.surfaceDrag != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::SurfDrag, pi.surfaceDrag);
                });

                // Param AirDrag
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.airDrag != base.airDrag),
                    ndyWriteIfPara(pi.airDrag != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::AirDrag, pi.airDrag);
                });

                // Param StaticDrag
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.staticDrag != base.staticDrag),
                    ndyWriteIfPara(pi.staticDrag != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::StaticDrag, pi.staticDrag);
                });

                // Param Mass
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.mass != base.mass),
                    ndyWriteIfPara(pi.mass != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Mass, pi.mass);
                });

                // Param Height
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.height != base.height),
                    ndyWriteIfPara(pi.height != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Height, pi.height);
                });

                // Param PhysFlags
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.flags != base.flags),
                    ndyWriteIfPara(pi.flags != PhysicsInfo::Flag::None), [&] {
                    ndyWriteThingParam<16>(rw, NdyThingParam::Physflags, pi.flags);
                });

                // Param MaxRotVel
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.maxRotationVelocity != base.maxRotationVelocity),
                    ndyWriteIfPara(pi.maxRotationVelocity != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::MaxRotVel, pi.maxRotationVelocity);
                });

                // Param Vel
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.velocity != base.velocity),
                    ndyWriteIfPara(!pi.velocity.isZero()), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Vel, pi.velocity.toString());
                });

                // Param MaxVel
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.maxVelocity != base.maxVelocity),
                    ndyWriteIfPara(pi.maxVelocity != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::MaxVel, pi.maxVelocity);
                });

                // Param AngVel
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.angularVelocity != base.angularVelocity),
                    ndyWriteIfPara(!pi.angularVelocity.isZero()), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Angvel, pi.angularVelocity.toString());
                });

                // Param OrientSpeed
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.orientSpeed != base.orientSpeed),
                    ndyWriteIfPara(pi.orientSpeed != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::OrientSpeed, pi.orientSpeed);
                });

                // Param Buoyancy
                ndyWriteThingParamIf(physInfo,
                    ndyWriteIfBase(pi.buoyancy != base.buoyancy),
                    ndyWriteIfPara(pi.buoyancy != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Buoyancy, pi.buoyancy);
                });
            }
        },
        t.moveInfo);
    }

    void ndyWriteThingInfo(TextResourceWriter& rw, const CndThing& t, OptionalRef<const CndThing> baseTemplate)
    {
        using namespace libim::utils;
        std::visit(overloaded {
            [](std::monostate){},
            // ActorInfo
            [&](const CndActorInfo& ai)
            {
                OptionalRef<const CndActorInfo> baseActorInfo;
                if(baseTemplate && std::holds_alternative<CndActorInfo>(baseTemplate->thingInfo)) {
                   baseActorInfo = std::get<CndActorInfo>(baseTemplate->thingInfo);
                }

               // Param Weapon
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.weaponTemplateName != base.weaponTemplateName && !ai.weaponTemplateName.isEmpty()),
                   ndyWriteIfPara(!ai.weaponTemplateName.isEmpty()), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::Weapon, ai.weaponTemplateName);
               });

               // Param Health
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.health != base.health),
                   ndyWriteIfPara(ai.health != 0), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::Health, ai.health);
               });

               // Param MaxHealth
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.maxHealth != base.maxHealth),
                   ndyWriteIfPara(ai.maxHealth != 0), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::MaxHealth, ai.maxHealth);
               });

               // Param MaxThrust
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.maxThrust != base.maxThrust),
                   ndyWriteIfPara(ai.maxThrust != 0), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::MaxThrust, ai.maxThrust);
               });

               // Param MaxRotThrust
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.maxRotThrust != base.maxRotThrust),
                   ndyWriteIfPara(ai.maxRotThrust != 0), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::MaxRotThrust, ai.maxRotThrust);
               });

               // Param MaxHeadVel
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.maxHeadVelocity != base.maxHeadVelocity),
                   ndyWriteIfPara(ai.maxHeadVelocity != 0), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::MaxHeadVel, ai.maxHeadVelocity);
               });

               // Param MaxHeadYaw
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.maxHeadYaw != base.maxHeadYaw),
                   ndyWriteIfPara(ai.maxHeadYaw != 0), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::MaxHeadYaw, ai.maxHeadYaw);
               });

               // Param JumpSpeed
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.jumpSpeed != base.jumpSpeed),
                   ndyWriteIfPara(ai.jumpSpeed != 0), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::JumpSpeed, ai.jumpSpeed);
               });

               // Param TypeFlags
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.flags != base.flags),
                   ndyWriteIfPara(ai.flags != 0), [&] {
                   ndyWriteThingParam<16>(rw, NdyThingParam::Typeflags, ai.flags);
               });

               // Param EyeOffset
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.eyeOffset != base.eyeOffset),
                   ndyWriteIfPara(!ai.eyeOffset.isZero()), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::EyeOffset, ai.eyeOffset.toString());
               });

               // Param MinHeadPitch
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.minHeadPitch != base.minHeadPitch),
                   ndyWriteIfPara(ai.minHeadPitch != 0), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::MinHeadPitch, ai.minHeadPitch);
               });

               // Param MaxHeadPitch
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.maxHeadPitch != base.maxHeadPitch),
                   ndyWriteIfPara(ai.maxHeadPitch != 0), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::MaxHeadPitch, ai.maxHeadPitch);
               });

               // Param FireOffset
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.fireOffset != base.fireOffset),
                   ndyWriteIfPara(!ai.fireOffset.isZero()), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::FireOffset, ai.fireOffset.toString());
               });

               // Param LightOffset
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.lightOffset != base.lightOffset),
                   ndyWriteIfPara(!ai.lightOffset.isZero()), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::LightOffset, ai.lightOffset.toString());
               });

               // Param LightIntensity
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.lightIntensity != base.lightIntensity),
                   ndyWriteIfPara(!ai.lightIntensity.isZero()), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::LightIntensity, ai.lightIntensity.toString());
               });

               // Param Explode
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.explodeTemplateName != base.explodeTemplateName && !ai.explodeTemplateName.isEmpty()),
                   ndyWriteIfPara(!ai.explodeTemplateName.isEmpty()), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::Explode, ai.explodeTemplateName);
               });

               // Param VoiceColor
               ndyWriteThingParamIf(baseActorInfo,
                   ndyWriteIfBase(ai.voiceColor != base.voiceColor),
                   ndyWriteIfPara(ai.voiceColor.isValid()), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::VoiceColor, ai.voiceColor.toString());
               });
            },
            // WeaponInfo
            [&](const CndWeaponInfo& wi) {
                OptionalRef<const CndWeaponInfo> baseWeaponInfo;
                if(baseTemplate && std::holds_alternative<CndWeaponInfo>(baseTemplate->thingInfo)) {
                    baseWeaponInfo = std::get<CndWeaponInfo>(baseTemplate->thingInfo);
                }

                // Param Explode
                ndyWriteThingParamIf(baseWeaponInfo,
                    ndyWriteIfBase(wi.explosionTemplateName != base.explosionTemplateName && !wi.explosionTemplateName.isEmpty()),
                    ndyWriteIfPara(!wi.explosionTemplateName.isEmpty()), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Explode, wi.explosionTemplateName);
                });

                // Param Damage
                ndyWriteThingParamIf(baseWeaponInfo,
                    ndyWriteIfBase(wi.damage != base.damage),
                    ndyWriteIfPara(wi.damage != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Damage, wi.damage);
                });

                // Param Damage
                ndyWriteThingParamIf(baseWeaponInfo,
                    ndyWriteIfBase(wi.minDamage != base.minDamage),
                    ndyWriteIfPara(wi.minDamage != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::MinDamage, wi.minDamage);
                });

                // Param TypeFlags
                ndyWriteThingParamIf(baseWeaponInfo,
                    ndyWriteIfBase(wi.flags != base.flags),
                    ndyWriteIfPara(wi.flags != 0), [&] {
                    ndyWriteThingParam<16>(rw, NdyThingParam::Typeflags, wi.flags);
                });

                // Param DamageClass
                ndyWriteThingParamIf(baseWeaponInfo,
                    ndyWriteIfBase(wi.damageType != base.damageType),
                    ndyWriteIfPara(wi.damageType != DamageType::None), [&] {
                    ndyWriteThingParam<16>(rw, NdyThingParam::DamageClass, wi.damageType);
                });

                // Param Range
                ndyWriteThingParamIf(baseWeaponInfo,
                    ndyWriteIfBase(wi.range != base.range),
                    ndyWriteIfPara(wi.range != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Range, wi.range);
                });

                // Param Rate
                ndyWriteThingParamIf(baseWeaponInfo,
                    ndyWriteIfBase(wi.rate != base.rate),
                    ndyWriteIfPara(wi.rate != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Rate, wi.rate);
                });

                // Param Force
                ndyWriteThingParamIf(baseWeaponInfo,
                   ndyWriteIfBase(wi.force != base.force),
                   ndyWriteIfPara(wi.force != 0), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::Force, wi.force);
                });
            },
            // Explosion Info
            [&](const CndExplosionInfo& ei) {
                OptionalRef<const CndExplosionInfo> baseExpInfo;
                if(baseTemplate && std::holds_alternative<CndExplosionInfo>(baseTemplate->thingInfo)) {
                    baseExpInfo = std::get<CndExplosionInfo>(baseTemplate->thingInfo);
                }

                // Param TypeFlags
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.flags != base.flags),
                    ndyWriteIfPara(ei.flags != 0), [&] {
                    ndyWriteThingParam<16>(rw, NdyThingParam::Typeflags, ei.flags);
                });

                // Param Damage
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.damage != base.damage),
                    ndyWriteIfPara(ei.damage != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Damage, ei.damage);
                });

                // Param DamageClass
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.damageType != base.damageType),
                    ndyWriteIfPara(ei.damageType != DamageType::None), [&] {
                    ndyWriteThingParam<16>(rw, NdyThingParam::DamageClass, ei.damageType);
                });

                // Param ExpandTime
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.msExpandTime != base.msExpandTime),
                    ndyWriteIfPara(ei.msExpandTime != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::ExpandTime, ei.msExpandTime * 0.001); // in sec
                });

                // Param FadeTime
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.msFadeTime != base.msFadeTime),
                    ndyWriteIfPara(ei.msFadeTime != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::FadeTime, ei.msFadeTime * 0.001); // in sec
                });

                // Param BlastTime
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.msBlastTime != base.msBlastTime),
                    ndyWriteIfPara(ei.msBlastTime != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::BlastTime, ei.msBlastTime * 0.001); // in sec
                });

                // Param BabyTime
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.msBabyTime != base.msBabyTime),
                    ndyWriteIfPara(ei.msBabyTime != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::BabyTime, ei.msBabyTime * 0.001); // in sec
                });

                // Param Force
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.force != base.force),
                    ndyWriteIfPara(ei.force != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Force, ei.force);
                });

                // Param MaxLight
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.maxLight != base.maxLight),
                    ndyWriteIfPara(ei.maxLight != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::MaxLight, ei.maxLight);
                });

                // Param Range
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.range != base.range),
                    ndyWriteIfPara(ei.range != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Range, ei.range);
                });

                // Param Debries
                for (const auto& d : ei.aDebrisTemplateNames)
                {
                    if (!d.isEmpty()) {
                        ndyWriteThingParam(rw, NdyThingParam::Debris, d);
                    }
                }

                // Param SpriteThing
                ndyWriteThingParamIf(baseExpInfo,
                   ndyWriteIfBase(ei.spriteThingName != base.spriteThingName && !ei.spriteThingName.isEmpty()),
                   ndyWriteIfPara(!ei.spriteThingName.isEmpty()), [&] {
                   ndyWriteThingParam(rw, NdyThingParam::SpriteThing, ei.spriteThingName);
                });

                // Param SpriteStart
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.posSpriteStart != base.posSpriteStart),
                    ndyWriteIfPara(!ei.posSpriteStart.isZero()), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::SpriteStart, ei.posSpriteStart.toString());
                });

                // Param SpriteEnd
                ndyWriteThingParamIf(baseExpInfo,
                    ndyWriteIfBase(ei.posSpriteEnd != base.posSpriteEnd),
                    ndyWriteIfPara(!ei.posSpriteEnd.isZero()), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::SpriteEnd, ei.posSpriteEnd.toString());
                });
            },
            // ItemInfo
            [&](const CndItemInfo& ii) {
                OptionalRef<const CndItemInfo> baseItemInfo;
                if (baseTemplate && std::holds_alternative<CndItemInfo>(baseTemplate->thingInfo)) {
                    baseItemInfo = std::get<CndItemInfo>(baseTemplate->thingInfo);
                }

                // Param TypeFlags
                ndyWriteThingParamIf(baseItemInfo,
                    ndyWriteIfBase(ii.flags != base.flags),
                    ndyWriteIfPara(ii.flags != 0), [&] {
                    ndyWriteThingParam<16>(rw, NdyThingParam::Typeflags, ii.flags);
                });

                // Param Respawn
                ndyWriteThingParamIf(baseItemInfo,
                    ndyWriteIfBase(ii.secRespawnInterval != base.secRespawnInterval),
                    ndyWriteIfPara(ii.secRespawnInterval != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Respawn, ii.secRespawnInterval);
                });
            },
            // Hint UserVal
            [&](const CndHintUserVal& userVal) {
                OptionalRef<const CndHintUserVal> baseUserVal;
                if (baseTemplate && std::holds_alternative<CndHintUserVal>(baseTemplate->thingInfo)) {
                    baseUserVal = std::get<CndHintUserVal>(baseTemplate->thingInfo);
                }

                ndyWriteThingParamIf(baseUserVal,
                    ndyWriteIfBase(userVal != base),
                    ndyWriteIfPara(userVal != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::UserVal, userVal);
                });
            },
            // ParticleInfo
            [&](const CndParticleInfo& pi) {
                OptionalRef<const CndParticleInfo> basePartInfo;
                if (baseTemplate && std::holds_alternative<CndParticleInfo>(baseTemplate->thingInfo)) {
                    basePartInfo = std::get<CndParticleInfo>(baseTemplate->thingInfo);
                }

                // Param TypeFlags
                ndyWriteThingParamIf(basePartInfo,
                    ndyWriteIfBase(pi.flags != base.flags),
                    ndyWriteIfPara(pi.flags != 0), [&] {
                    ndyWriteThingParam<16>(rw, NdyThingParam::Typeflags, pi.flags);
                });

                // Param Material
                ndyWriteThingParamIf(basePartInfo,
                    ndyWriteIfBase(pi.materialFileName != base.materialFileName && !pi.materialFileName.isEmpty()),
                    ndyWriteIfPara(!pi.materialFileName.isEmpty()), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Material, pi.materialFileName);
                });

                // Param Range
                ndyWriteThingParamIf(basePartInfo,
                    ndyWriteIfBase(pi.maxRadius != base.maxRadius),
                    ndyWriteIfPara(pi.maxRadius != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Range, pi.maxRadius);
                });

                // Param MinSize
                ndyWriteThingParamIf(basePartInfo,
                    ndyWriteIfBase(pi.minRadius != base.minRadius),
                    ndyWriteIfPara(pi.minRadius != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::MinSize, pi.minRadius);
                });

                // Param Rate
                ndyWriteThingParamIf(basePartInfo,
                    ndyWriteIfBase(pi.timeoutRate != base.timeoutRate),
                    ndyWriteIfPara(pi.timeoutRate != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Rate, pi.timeoutRate); // in msec
                });

                // Param MaxThrust
                ndyWriteThingParamIf(basePartInfo,
                    ndyWriteIfBase(pi.growthSpeed != base.growthSpeed),
                    ndyWriteIfPara(pi.growthSpeed != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::MaxThrust, pi.growthSpeed);
                });

                // Param PitchRange
                ndyWriteThingParamIf(basePartInfo,
                    ndyWriteIfBase(pi.pitchRange != base.pitchRange),
                    ndyWriteIfPara(pi.pitchRange != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::PitchRange, pi.pitchRange);
                });

                // Param YawRange
                ndyWriteThingParamIf(basePartInfo,
                    ndyWriteIfBase(pi.yawRange != base.yawRange),
                    ndyWriteIfPara(pi.yawRange != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::YawRange, pi.yawRange);
                });

                // Param ElementSize
                ndyWriteThingParamIf(basePartInfo,
                    ndyWriteIfBase(pi.size != base.size),
                    ndyWriteIfPara(pi.size != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::ElementSize, pi.size);
                });

                // Param Count
                ndyWriteThingParamIf(basePartInfo,
                    ndyWriteIfBase(pi.numParticles != base.numParticles),
                    ndyWriteIfPara(pi.numParticles != 0), [&] {
                    ndyWriteThingParam(rw, NdyThingParam::Count, pi.numParticles);
                });
            },
        }, t.thingInfo);
    }
}
#endif // LIBIM_NDY_THING_SER_HELPERS_H
