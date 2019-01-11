#ifndef LIBIM_CND_KEY_STRUCTS_H
#define LIBIM_CND_KEY_STRUCTS_H

#include "../../../../../animation/animation.h"
#include "../../../../../animation/key_node_entry.h"
#include "../../../../../animation/key_marker.h"

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
    struct CndKeyHeader final
    {
        char  name[64];
        Animation::Flag flags;
        Animation::Type  type;
        uint32_t frames;
        float    fps;
        uint32_t numMarkers;
        uint32_t numJoints;
        uint32_t numNodes;
    };


    struct CndKeyNode final
    {
        char meshName[64];
        uint32_t nodeNum;
        uint32_t numEntries;
    };
}




#endif // LIBIM_CND_KEY_STRUCTS_H

