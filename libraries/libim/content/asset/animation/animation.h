#ifndef LIBIM_ANIMATION_H
#define LIBIM_ANIMATION_H
#include "../asset.h"
#include "key_node.h"
#include "key_marker.h"

#include <libim/content/text/text_resource_reader.h>
#include <libim/content/text/text_resource_writer.h>
#include <libim/types/flags.h>

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
            Unknown   = 0x0000,
            Unknown1  = 0x0001,   // tu_hit_shoulderl.key
            Unknown2  = 0x0004,   // tu_attack_ready.key
            Unknown3  = 0x0008,   // tu_hit_headl
            Unknown4  = 0x000F,   // 0so_armsmid_3_3.key
            Unknown5  = 0x0070,   // vo_rotate_left.key
            Unknown6  = 0x0104,   // in_attack_put_gun.key
            Unknown7  = 0x010D,   // in_attack_put_whip.key, in_attack_put_machete.key
            Unknown8  = 0x0186,   // 0vo_bothup_3_3.key
            Unknown9  = 0x018F,   // in_attack_unaim_rifle.key, in_attack_unaim_shotgun.key, in_attack_put_imp.key
            Unknown10 = 0x04FB,   // in_activate_medium_left.key
            Unknown11 = 0x0904,   // in_attack_pull_whip.key, in_attack_pull_satchel.key, in_attack_pull_imp.key
            Unknown12 = 0x0986,   // in_attack_pull_rifle.key
            Unknown13 = 0x090D,   // in_attack_pull_machete.key
            Unknown14 = 0x098F,   // in_attack_pull_fists.key
            Unknown15 = 0xFFFF,
        };

        enum class Flag : uint32_t
        {
            Loop                = 0x0,
            PausesOnFirstFrame  = 0x1,
            DoesNotLoop         = 0x2,
            PausesOnLastFrame   = 0x4,
            RestartIfPlaying    = 0x8,
            FinishInGivenTime   = 0x10,
            EndSmoothly         = 0x20,

            // Ref: http://www.jkhub.net/library/index.php?title=Reference:COG_Flag_-_Keyframe
            // *: Reference taken from https://www.massassi.net/jkspecs/
        };

        using Asset::Asset;

        void setFlags(Flags<Flag> flags)
        {
            flags_ = flags;
        }

        Flags<Flag> flags() const
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

        void setNodes(std::vector<KeyNode>nodes)
        {
            nodes_ = std::move(nodes);
        }

        const std::vector<KeyNode>& nodes() const
        {
            return nodes_;
        }

        std::vector<KeyNode>& nodes()
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

    private:
        Flags<Flag> flags_  = Flag::Loop;
        Type        type_   = Unknown;
        uint32_t    frames_ = 0UL;
        float       fps_    = 0.0f;
        uint32_t    joints_ = 0UL;

        std::vector<KeyMarker> markers_;
        std::vector<KeyNode> nodes_;
    };

    /**
     * Loads Animation from KEY text format from TextResourceReader
     * @param rr - text resource reader to read Animation from
    */
    Animation keyLoad(text::TextResourceReader& rr);
    Animation keyLoad(text::TextResourceReader&& rr);

    /**
     * Writes Animation to TextResourceWriter as KEY text format
     * @param anim - Animation to write
     * @param rw   - text resource writer to write anim to
     * @param headerComments - (optional) additional header comments to write to KEY file
     *
    */
    void keyWrite(const Animation& anim,
        text::TextResourceWriter& rw,
        const std::vector<std::string>& headerComments = {}
    );

    void keyWrite(const Animation& anim,
        text::TextResourceWriter&& rw,
        const std::vector<std::string>& headerComments = {}
    );
}
#endif // LIBIM_ANIMATION_H
