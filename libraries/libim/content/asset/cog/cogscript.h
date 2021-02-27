#ifndef LIBIM_COGSCRIPT_H
#define LIBIM_COGSCRIPT_H
#include "cogsymbol.h"
#include "cogvtable.h"
#include "../asset.h"

#include <libim/io/stream.h>
#include <libim/types/flags.h>
#include <libim/types/hashmap.h>
#include <string>

namespace libim::content::asset {
    struct CogScript : public Asset
    {
        enum Flag
        {
            None      = 0x00,
            Debug     = 0x01,
            Disabled  = 0x02,
            PulseSet  = 0x04,
            TimerSet  = 0x08,
            Paused    = 0x10,
            Unknown20 = 0x20,    // Set when thing/template assigns cog
            Local     = 0x40,    // Cog runs on the client and server
            Server    = 0x80,    // Cog runs only on the server, client messages are forwarded to the server
            Global    = 0x100,   // Cog runs locally on all machines
            NoSync    = 0x200,   // Cog results are not broadcast to the other machines.
        };

        Flags<Flag> flags;
        HashMap<CogSymbol> symbols;

        using Asset::Asset;
        CogScript(const InputStream& istream, bool parseSymDescription = false);

        CogVTable::Id getNextVTableId() const
        {
            auto uvtid = utils::to_underlying(vtid_);
            vtid_ = static_cast<CogVTable::Id>( ++uvtid );
            return vtid_;
        }

    private:
        mutable CogVTable::Id vtid_ = CogVTable::defaultId;
    };
}
#endif // LIBIM_COGSCRIPT_H
