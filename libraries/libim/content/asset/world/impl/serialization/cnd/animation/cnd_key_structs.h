#ifndef LIBIM_CND_KEY_STRUCTS_H
#define LIBIM_CND_KEY_STRUCTS_H

#include "../cnd.h"

#include <libim/content/asset/animation/animation.h>
#include <libim/content/asset/animation/key_node_entry.h>
#include <libim/content/asset/animation/key_marker.h>
#include <libim/types/flags.h>

// Ref: https://www.massassi.net/jkspecs/key.htm

/* Defines animation structures in CND file
 *
 * Keyframes are grouped by section in list.
 * The number of all keyframe files is defined in cnd header.
 *
 * First is the list of header structurs for each keyframe file,
 * followed by the list of markers for each keyframe file.
 * Then is the list of nodes for each file and the last is list of node entries for each keyframe file.
 *
 * Header list
 * [
 *    keyframe_1_header,
 *    keyframe_2_header,
 *     ...
 *    keyframe_N_header
 * ]
 *
 * Marker list
 * [
 *    keyframe_1_markers { marker1, marker2 ...}
 *    keyframe_2_markers { marker1, marker2 ...},
 *     ...
 *    keyframe_N_markers { marker1, marker2 ...}
 * ]
 *
 * Node list
 * [
 *    keyframe_1_nodes { node1, node2 ...}
 *    keyframe_2_nodes { node1, node2 ...},
 *     ...
 *    keyframe_N_nodes { node1, node2 ...}
 * ]
 *
 * Node entry list
 * [
 *    keyframe_1_node_entries { entry1, entry2 ...}
 *    keyframe_2_node_entries { entry1, entry2 ...},
 *     ...
 *    keyframe_N_node_entries { entry1, entry2 ...}
 * ]
*/

namespace libim::content::asset
{
    constexpr static std::size_t kCndMaxKeyMarkers = 16;

    struct CndKeyHeader final
    {
        CndResourceName name;
        Flags<Animation::Flag> flags;
        Flags<Animation::Type> type;
        uint32_t frames;
        float    fps;
        uint32_t numMarkers;
        uint32_t numJoints;
        uint32_t numNodes;
    };
    static_assert(sizeof(CndKeyHeader) == 92);

    struct CndKeyNode final
    {
        CndResourceName meshName;
        uint32_t nodeNum;
        uint32_t numEntries;
    };
    static_assert(sizeof(CndKeyNode) == 72);
}

#endif // LIBIM_CND_KEY_STRUCTS_H