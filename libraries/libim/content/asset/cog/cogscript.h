#ifndef LIBIM_COGSCRIPT_H
#define LIBIM_COGSCRIPT_H
#include "cogsymbol.h"
#include "cogvtable.h"
#include "../asset.h"

#include <libim/io/stream.h>
#include <libim/types/flags.h>
#include <libim/types/indexmap.h>
#include <string>

namespace libim::content::asset {
    struct CogScript : public Asset
    {
        using Asset::Asset;

        enum Flag
        {
            None        = 0x00,
            Debug       = 0x01,
            Disabled    = 0x02,
            PulseSet    = 0x04,
            TimerSet    = 0x08,
            Paused      = 0x10,
            ThingLinked = 0x20,    // Cog is linked to a thing, i.e it is a thing cog script
            Local       = 0x40,    // Cog runs on the client and server
            Server      = 0x80,    // Cog runs only on the server, client messages are forwarded to the server
            Global      = 0x100,   // Cog runs locally on all machines
            NoSync      = 0x200,   // Cog results are not broadcast to the other machines.
        };

        Flags<Flag> flags;
        IndexMap<CogSymbol> symbols;

        CogVTable::Id getNextVTableId() const
        {
            auto uvtid = utils::to_underlying(vtid_);
            vtid_ = static_cast<CogVTable::Id>( ++uvtid );
            return vtid_;
        }

    private:
        mutable CogVTable::Id vtid_ = CogVTable::defaultId;
    };

    /**
     * Loads COG script from the given input stream.
     * @param istream             - Input stream to load script from.
     * @param parseSymDescription - If true, the symbol descriptions will be parsed.
     * @return Loaded COG script.
     *
     * @throw StreamError - On encountering stream IO errors.
     * @throw SyntaxError - If the script contains syntax errors.
    */
    CogScript loadCogScript(const InputStream& istream, bool parseSymDescription = false);
}
#endif // LIBIM_COGSCRIPT_H
