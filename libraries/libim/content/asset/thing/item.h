#ifndef LIBIM_ITEM_H
#define LIBIM_ITEM_H
#include "thing.h"
#include <cstdint>

namespace libim::content::asset {
    class Item : Thing
    {
    public:
        enum Flag: uint32_t
        {
            None      = 0x00,
            RespawnMP = 0x01, // Respawn item when in multiplayer
            RespawnSP = 0x02, // Respawn item when in single player.
                              // Note:  Respawn interval is set through ndy level file by setting item param respawn=<msec> and set item type flag to RespawnSP
            Backpack  = 0x04
        };
    };
}

#endif // LIBIM_ITEM_H
