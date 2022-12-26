#ifndef LIBIM_KEY_NODE_H
#define LIBIM_KEY_NODE_H
#include "key_node_entry.h"

#include <cstdint>
#include <vector>
#include <string>

namespace libim::content::asset {
    struct KeyNode final
    {
        uint32_t num;
        std::string meshName;
        std::vector<KeyNodeEntry> entries;
    };

    inline constexpr bool operator == (const KeyNode& n1, const KeyNode& n2)
    {
        return n1.num == n2.num && n1.meshName == n2.meshName && n1.entries == n2.entries;
    }
}
#endif // LIBIM_KEY_NODE_H
