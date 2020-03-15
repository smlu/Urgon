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
            None     = 0x00,
            Backpack = 0x04
        };
    };
}

#endif // LIBIM_ITEM_H
