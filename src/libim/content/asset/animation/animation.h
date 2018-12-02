#ifndef LIBIM_ANIMATION_H
#define LIBIM_ANIMATION_H
#include "../asset.h"
#include "animation_node.h"
#include "key_marker.h"

#include "../../text/text_resource_reader.h"

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace libim::content::asset {
    class Animation final : public Asset
    {
    public:
        enum Type
        {
            Unknown  = 0x0000,
            Unknown2 = 0xFFFF,
        };

        enum class Flag : uint32_t
        {
            Loop                = 0x0, //*
            PausesOnFirstFrame  = 0x1,
            DoesNotLoop         = 0x2,
            PausesOnLastFrame   = 0x4,
            RestartIfPlaying    = 0x8,
            FinishInGivenTime   = 0x10,
            EndSmoothly         = 0x20,

            // Ref: http://www.jkhub.net/library/index.php?title=Reference:COG_Flag_-_Keyframe
            // *: Reference taken from https://www.massassi.net/jkspecs/
        };

        void setFlags(Flag flags)
        {
            flags_ = flags;
        }

        Flag flages() const
        {
            return flags_;
        }

        void setFrames(uint32_t frames)
        {
            frames_ = frames;
        }

        uint32_t frames() const
        {
            return frames_;
        }

        void setFps(float fps)
        {
            fps_ = fps;
        }

        float fps() const
        {
            return fps_;
        }

        void setJoints(uint32_t joints)
        {
            joints_ = joints;
        }

        uint32_t joints() const
        {
            return joints_;
        }

        void setMarkers(std::vector<KeyMarker> markers)
        {
            markers_ = std::move(markers);
        }

        const std::vector<KeyMarker>& markers() const
        {
            return markers_;
        }

        void setNodes(std::vector<AnimationNode>nodes)
        {
            nodes_ = std::move(nodes);
        }

        const std::vector<AnimationNode>& nodes() const
        {
            return nodes_;
        }

        void setType(Type type)
        {
            type_ = type;
        }

        Type type() const
        {
            return type_;
        }

        Animation& load(text::TextResourceReader& rr);

    private:
        Flag     flags_  = Flag::Loop;
        Type     type_   = Unknown;
        uint32_t frames_ = 0UL;
        float    fps_    = 0.0f;
        uint32_t joints_ = 0UL;

        std::vector<KeyMarker> markers_;
        std::vector<AnimationNode> nodes_;
    };
}
#endif // LIBIM_ANIMATION_H
