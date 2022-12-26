#ifndef LIBIM_COG_H
#define LIBIM_COG_H
#include "cogscript.h"
#include "cogvtable.h"
#include "../asset.h"
#include <libim/types/flags.h>
#include <libim/types/sharedref.h>

namespace libim::content::asset {
    struct Cog : public Asset
    {
        using Id = std::size_t;
        Id id;
        Flags<CogScript::Flag> flags;
        SharedRef<CogScript> script;
        CogVTable::Id vtid;
    };
}
#endif // LIBIM_COG_H
