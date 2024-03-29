#ifndef LIBIM_NDY_THING_SER_COMMON_H
#define LIBIM_NDY_THING_SER_COMMON_H
#include "../../cnd/thing/cnd_thing.h"
#include <libim/content/asset/thing/thing.h>
#include <libim/types/string_map.h>

#include <string_view>
#include <type_traits>

namespace libim::content::asset {
    using namespace std::string_view_literals;

    static constexpr auto kNone = "none"sv;

    static const StringMap<const std::string_view, Thing::Type> kThingTypeNameMap {
        { "free"sv      , Thing::Free      },
        { "camera"sv    , Thing::Camera    },
        { "actor"sv     , Thing::Actor     },
        { "weapon"sv    , Thing::Weapon    },
        { "debris"sv    , Thing::Debris    },
        { "item"sv      , Thing::Item      },
        { "explosion"sv , Thing::Explosion },
        { "cog"sv       , Thing::Cog       },
        { "ghost"sv     , Thing::Ghost     },
        { "corpse"sv    , Thing::Corpse    },
        { "player"sv    , Thing::Player    },
        { "particle"sv  , Thing::Particle  },
        { "hint"sv      , Thing::Hint      },
        { "sprite"sv    , Thing::Sprite    },
        { "polyline"sv  , Thing::Polyline  }
    };

    static const std::map<Thing::Type, std::string_view> kThingTypeMap {
        { Thing::Free      , "free"sv      },
        { Thing::Camera    , "camera"sv    },
        { Thing::Actor     , "actor"sv     },
        { Thing::Weapon    , "weapon"sv    },
        { Thing::Debris    , "debris"sv    },
        { Thing::Item      , "item"sv      },
        { Thing::Explosion , "explosion"sv },
        { Thing::Cog       , "cog"sv       },
        { Thing::Ghost     , "ghost"sv     },
        { Thing::Corpse    , "corpse"sv    },
        { Thing::Player    , "player"sv    },
        { Thing::Particle  , "particle"sv  },
        { Thing::Hint      , "hint"sv      },
        { Thing::Sprite    , "sprite"sv    },
        { Thing::Polyline  , "polyline"sv  }
    };


    static const std::map<CndThingMoveType, std::string_view> kCndThingMoveTypeMap {
        { CndThingMoveType::None    , kNone       },
        { CndThingMoveType::Physics , "physics"sv },
        { CndThingMoveType::Path    , "path"sv    },
    };

    static const StringMap<const std::string_view, CndThingMoveType> kCndThingMoveNameMap {
        { kNone       , CndThingMoveType::None    },
        { "physics"sv , CndThingMoveType::Physics },
        { "path"sv    , CndThingMoveType::Path    },
    };


    enum class NdyThingParam
    {
        Type             = 1,
        Collide          = 2,
        Move             = 3,
        Size             = 4,
        ThingFlags       = 5,
        Timer            = 6,
        Light            = 7,
        Attach           = 8,  // not used by engine
        SoundClass       = 9,
        Model3d          = 10,
        Sprite           = 11,
        SurfDrag         = 12,
        AirDrag          = 13,
        StaticDrag       = 14,
        Mass             = 15,
        Height           = 16,
        PhysicsFlags     = 17,
        MaxRotVel        = 18,
        MaxVel           = 19,
        Vel              = 20,
        AngularVel       = 21,
        TypeFlags        = 22,
        Health           = 23,
        MaxThrust        = 24,
        MaxRotThrust     = 25,
        MaxHeadVel       = 26,
        MaxHeadYaw       = 27,
        JumpSpeed        = 28,
        Weapon           = 29,
        Damage           = 30,
        MinDamage        = 31,
        DamageClass      = 32,  // DamageType
        Explode          = 33,
        Frame            = 34, // ai path position frame or thing movement path position&orient frame
        NumFrames        = 35, // ai or thing path movement
        Puppet           = 36,
        BlastTime        = 37,
        BabyTime         = 38,  // explosion time before child explosion is created
        Force            = 39,
        MaxLight         = 40,
        Range            = 41,
        FlashRGB         = 42, // not used by engine
        ExpandTime       = 43,
        FadeTime         = 44,
        AIClass          = 45,
        Cog              = 46,
        Respawn          = 47,
        Material         = 48,
        Rate             = 49,
        Count            = 50,
        ElementSize      = 51,
        Particle         = 52,
        MaxHealth        = 53,
        MoveSize         = 54,
        OrientSpeed      = 55,
        Buoyancy         = 56,
        EyeOffset        = 57,
        MinHeadPitch     = 58,
        MaxHeadPitch     = 59,
        FireOffset       = 60,
        LightOffset      = 61,
        LightIntensity   = 62,
        Debris           = 63,
        CreateThing      = 64,
        MinSize          = 65,
        PitchRange       = 66,
        YawRange         = 67,
        Orient           = 68,
        UserVal          = 69,
        SpriteThing      = 70,
        SpriteStart      = 71,
        SpriteEnd        = 72,
        CollHeight       = 73,
        CollWidth        = 74,
        VoiceColor       = 75,
        PerformanceLevel = 76
    };

    static const std::map<NdyThingParam, std::string_view> kNdyThingParamMap {
        { NdyThingParam::Type             , "type"sv           },
        { NdyThingParam::Collide          , "collide"sv        },
        { NdyThingParam::Move             , "move"sv           },
        { NdyThingParam::Size             , "size"sv           },
        { NdyThingParam::ThingFlags       , "thingflags"sv     },
        { NdyThingParam::Timer            , "timer"sv          },
        { NdyThingParam::Light            , "light"sv          },
        { NdyThingParam::Attach           , "attach"sv         },
        { NdyThingParam::SoundClass       , "soundclass"sv     },
        { NdyThingParam::Model3d          , "model3d"sv        },
        { NdyThingParam::Sprite           , "sprite"sv         },
        { NdyThingParam::SurfDrag         , "surfdrag"sv       },
        { NdyThingParam::AirDrag          , "airdrag"sv        },
        { NdyThingParam::StaticDrag       , "staticdrag"sv     },
        { NdyThingParam::Mass             , "mass"sv           },
        { NdyThingParam::Height           , "height"sv         },
        { NdyThingParam::PhysicsFlags     , "physflags"sv      },
        { NdyThingParam::MaxRotVel        , "maxrotvel"sv      },
        { NdyThingParam::MaxVel           , "maxvel"sv         },
        { NdyThingParam::Vel              , "vel"sv            },
        { NdyThingParam::AngularVel       , "angvel"sv         },
        { NdyThingParam::TypeFlags        , "typeflags"sv      },
        { NdyThingParam::Health           , "health"sv         },
        { NdyThingParam::MaxThrust        , "maxthrust"sv      },
        { NdyThingParam::MaxRotThrust     , "maxrotthrust"sv   },
        { NdyThingParam::MaxHeadVel       , "maxheadvel"sv     },
        { NdyThingParam::MaxHeadYaw       , "maxheadyaw"sv     },
        { NdyThingParam::JumpSpeed        , "jumpspeed"sv      },
        { NdyThingParam::Weapon           , "weapon"sv         },
        { NdyThingParam::Damage           , "damage"sv         },
        { NdyThingParam::MinDamage        , "mindamage"sv      },
        { NdyThingParam::DamageClass      , "damageclass"sv    },
        { NdyThingParam::Explode          , "explode"sv        },
        { NdyThingParam::Frame            , "frame"sv          },
        { NdyThingParam::NumFrames        , "numframes"sv      },
        { NdyThingParam::Puppet           , "puppet"sv         },
        { NdyThingParam::BlastTime        , "blasttime"sv      },
        { NdyThingParam::BabyTime         , "babytime"sv       },
        { NdyThingParam::Force            , "force"sv          },
        { NdyThingParam::MaxLight         , "maxlight"sv       },
        { NdyThingParam::Range            , "range"sv          },
        { NdyThingParam::FlashRGB         , "flashrgb"sv       },
        { NdyThingParam::ExpandTime       , "expandtime"sv     },
        { NdyThingParam::FadeTime         , "fadetime"sv       },
        { NdyThingParam::AIClass          , "aiclass"sv        },
        { NdyThingParam::Cog              , "cog"sv            },
        { NdyThingParam::Respawn          , "respawn"sv        },
        { NdyThingParam::Material         , "material"sv       },
        { NdyThingParam::Rate             , "rate"sv           },
        { NdyThingParam::Count            , "count"sv          },
        { NdyThingParam::ElementSize      , "elementsize"sv    },
        { NdyThingParam::Particle         , "particle"sv       },
        { NdyThingParam::MaxHealth        , "maxhealth"sv      },
        { NdyThingParam::MoveSize         , "movesize"sv       },
        { NdyThingParam::OrientSpeed      , "orientspeed"sv    },
        { NdyThingParam::Buoyancy         , "buoyancy"sv       },
        { NdyThingParam::EyeOffset        , "eyeoffset"sv      },
        { NdyThingParam::MinHeadPitch     , "minheadpitch"sv   },
        { NdyThingParam::MaxHeadPitch     , "maxheadpitch"sv   },
        { NdyThingParam::FireOffset       , "fireoffset"sv     },
        { NdyThingParam::LightOffset      , "lightoffset"sv    },
        { NdyThingParam::LightIntensity   , "lightintensity"sv },
        { NdyThingParam::Debris           , "debris"sv         },
        { NdyThingParam::CreateThing      , "creatething"sv    },
        { NdyThingParam::MinSize          , "minsize"sv        },
        { NdyThingParam::PitchRange       , "pitchrange"sv     },
        { NdyThingParam::YawRange         , "yawrange"sv       },
        { NdyThingParam::Orient           , "orient"sv         },
        { NdyThingParam::UserVal          , "userval"sv        },
        { NdyThingParam::SpriteThing      , "spritething"sv    },
        { NdyThingParam::SpriteStart      , "spritestart"sv    },
        { NdyThingParam::SpriteEnd        , "spriteend"sv      },
        { NdyThingParam::CollHeight       , "collheight"sv     },
        { NdyThingParam::CollWidth        , "collwidth"sv      },
        { NdyThingParam::VoiceColor       , "voicecolor"sv     },
        { NdyThingParam::PerformanceLevel , "perflevel"sv      }
    };

    static const StringMap<const std::string_view, NdyThingParam> kNdyThingParamNameMap {
        { "type"sv           , NdyThingParam::Type             },
        { "collide"sv        , NdyThingParam::Collide          },
        { "move"sv           , NdyThingParam::Move             },
        { "size"sv           , NdyThingParam::Size             },
        { "thingflags"sv     , NdyThingParam::ThingFlags       },
        { "timer"sv          , NdyThingParam::Timer            },
        { "light"sv          , NdyThingParam::Light            },
        { "attach"sv         , NdyThingParam::Attach           },
        { "soundclass"sv     , NdyThingParam::SoundClass       },
        { "model3d"sv        , NdyThingParam::Model3d          },
        { "sprite"sv         , NdyThingParam::Sprite           },
        { "surfdrag"sv       , NdyThingParam::SurfDrag         },
        { "airdrag"sv        , NdyThingParam::AirDrag          },
        { "staticdrag"sv     , NdyThingParam::StaticDrag       },
        { "mass"sv           , NdyThingParam::Mass             },
        { "height"sv         , NdyThingParam::Height           },
        { "physflags"sv      , NdyThingParam::PhysicsFlags     },
        { "maxrotvel"sv      , NdyThingParam::MaxRotVel        },
        { "maxvel"sv         , NdyThingParam::MaxVel           },
        { "vel"sv            , NdyThingParam::Vel              },
        { "angvel"sv         , NdyThingParam::AngularVel       },
        { "typeflags"sv      , NdyThingParam::TypeFlags        },
        { "health"sv         , NdyThingParam::Health           },
        { "maxthrust"sv      , NdyThingParam::MaxThrust        },
        { "maxrotthrust"sv   , NdyThingParam::MaxRotThrust     },
        { "maxheadvel"sv     , NdyThingParam::MaxHeadVel       },
        { "maxheadyaw"sv     , NdyThingParam::MaxHeadYaw       },
        { "jumpspeed"sv      , NdyThingParam::JumpSpeed        },
        { "weapon"sv         , NdyThingParam::Weapon           },
        { "damage"sv         , NdyThingParam::Damage           },
        { "mindamage"sv      , NdyThingParam::MinDamage        },
        { "damageclass"sv    , NdyThingParam::DamageClass      },
        { "explode"sv        , NdyThingParam::Explode          },
        { "frame"sv          , NdyThingParam::Frame            },
        { "numframes"sv      , NdyThingParam::NumFrames        },
        { "puppet"sv         , NdyThingParam::Puppet           },
        { "blasttime"sv      , NdyThingParam::BlastTime        },
        { "babytime"sv       , NdyThingParam::BabyTime         },
        { "force"sv          , NdyThingParam::Force            },
        { "maxlight"sv       , NdyThingParam::MaxLight         },
        { "range"sv          , NdyThingParam::Range            },
        { "flashrgb"sv       , NdyThingParam::FlashRGB         },
        { "expandtime"sv     , NdyThingParam::ExpandTime       },
        { "fadetime"sv       , NdyThingParam::FadeTime         },
        { "aiclass"sv        , NdyThingParam::AIClass          },
        { "cog"sv            , NdyThingParam::Cog              },
        { "respawn"sv        , NdyThingParam::Respawn          },
        { "material"sv       , NdyThingParam::Material         },
        { "rate"sv           , NdyThingParam::Rate             },
        { "count"sv          , NdyThingParam::Count            },
        { "elementsize"sv    , NdyThingParam::ElementSize      },
        { "particle"sv       , NdyThingParam::Particle         },
        { "maxhealth"sv      , NdyThingParam::MaxHealth        },
        { "movesize"sv       , NdyThingParam::MoveSize         },
        { "orientspeed"sv    , NdyThingParam::OrientSpeed      },
        { "buoyancy"sv       , NdyThingParam::Buoyancy         },
        { "eyeoffset"sv      , NdyThingParam::EyeOffset        },
        { "minheadpitch"sv   , NdyThingParam::MinHeadPitch     },
        { "maxheadpitch"sv   , NdyThingParam::MaxHeadPitch     },
        { "fireoffset"sv     , NdyThingParam::FireOffset       },
        { "lightoffset"sv    , NdyThingParam::LightOffset      },
        { "lightintensity"sv , NdyThingParam::LightIntensity   },
        { "debris"sv         , NdyThingParam::Debris           },
        { "creatething"sv    , NdyThingParam::CreateThing      },
        { "minsize"sv        , NdyThingParam::MinSize          },
        { "pitchrange"sv     , NdyThingParam::PitchRange       },
        { "yawrange"sv       , NdyThingParam::YawRange         },
        { "orient"sv         , NdyThingParam::Orient           },
        { "userval"sv        , NdyThingParam::UserVal          },
        { "spritething"sv    , NdyThingParam::SpriteThing      },
        { "spritestart"sv    , NdyThingParam::SpriteStart      },
        { "spriteend"sv      , NdyThingParam::SpriteEnd        },
        { "collheight"sv     , NdyThingParam::CollHeight       },
        { "collwidth"sv      , NdyThingParam::CollWidth        },
        { "voicecolor"sv     , NdyThingParam::VoiceColor       },
        { "perflevel"sv      , NdyThingParam::PerformanceLevel }
    };
}
#endif // LIBIM_NDY_THING_SER_COMMON_H
