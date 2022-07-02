#ifndef LIBIM_PHYSICSINFO_H
#define LIBIM_PHYSICSINFO_H
#include <cstdint>
#include <libim/math/vector3.h>
#include <libim/types/flags.h>

namespace libim::content::asset {
    struct PhysicsInfo final
    {
        enum Flag : uint32_t
        {
            None                         = 0x0,
            UseGravity                   = 0x1,
            UseThrust                    = 0x2,
            AlignToSurface               = 0x10,
            BounceOffSurface             = 0x20,
            SticksToFloor                = 0x40,
            SticksToWall                 = 0x80,
            Unknown_100                  = 0x100,     // used when controlling jeep
            UseRotationVelocity          = 0x200,
            BanksWhenTurning             = 0x400,
            Unknown_800                  = 0x800,     // maybe marks in the engine that the flags has been changed
            UseAngularThrustToRotate     = 0x1000,
            Fly                          = 0x2000,
            AffectedByBlastForce         = 0x4000,
            Unknown_8000                 = 0x8000,
            IsCrouching                  = 0x10000,
            DoNotRotateVelocity          = 0x20000,
            PartialGravity               = 0x40000,   // Half of world gravity
            Uknown_800000                = 0x800000,  // 11_pyr_kidvsindy_2.cog
            OnWaterSurface               = 0x100000,  // raft, swimming
            NotAffectedByThrust          = 0x400000,
            MineCar                      = 0x1000000, // Thing moves on the track surface as mine car. bab killtruck, sol mineplayer - minecar
            Raft                         = 0x2000000, // Thing moves on the water surface as raft. mentioned in pru_lagoon.cog, 02_riv - raftplayer
            Jeep                         = 0x4000000, // Thing moves as jeep/car.
            Unknown_40000000             = 0x40000000
        };

        Flags<Flag> flags          = None;
        double mass                = 0;
        double height              = 0;
        double airDrag             = 0;
        double surfaceDrag         = 0;
        double staticDrag          = 0;
        double maxRotationVelocity = 0;
        double maxVelocity         = 0;
        double orientSpeed         = 0;
        double buoyancy            = 0;
        Vector3f angularVelocity;
        Vector3f velocity;
    };
}

#endif //LIBIM_PHYSICSINFO_H
