#ifndef LIBIM_RESOURCE_LITERALS_H
#define LIBIM_RESOURCE_LITERALS_H
#include <libim/text/impl/schars.h>
#include <string_view>

namespace libim::content::text {
    using namespace std::string_view_literals;

    static constexpr auto kResLabelDelim     = ":"sv;
    static constexpr auto kResSectionHeader  = "###############"sv;
    static constexpr auto kResName_Entries   = "ENTRIES"sv;
    static constexpr auto kResName_Flags     = "FLAGS"sv;
    static constexpr auto kResName_Frames    = "FRAMES"sv;
    static constexpr auto kResName_Fps       = "FPS"sv;
    static constexpr auto kResName_Header    = "HEADER"sv;
    static constexpr auto kResName_Joints    = "JOINTS"sv;
    static constexpr auto kResName_KfNodes   = "KEYFRAME NODES"sv;
    static constexpr auto kResName_Markers   = "MARKERS"sv;
    static constexpr auto kResName_MeshName  = "MESH NAME"sv;
    static constexpr auto kResName_Node      = "NODE"sv;
    static constexpr auto kResName_Nodes     = "NODES"sv;
    static constexpr auto kResName_Section   = "SECTION"sv;
    static constexpr auto kResName_Type      = "TYPE"sv;
}

#endif // LIBIM_RESOURCE_LITERALS_H
