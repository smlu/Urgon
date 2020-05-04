#ifndef IM_COGSCRIPT_FIXES_H
#define IM_COGSCRIPT_FIXES_H
#include <functional>
#include <map>
#include <string>

#include <libim/content/asset/cog/cogscript.h>
#include <libim/utils/utils.h>

namespace imfixes {
    namespace details {
        using namespace libim::content::asset;
        using namespace libim::utils;

        // TODO: Verify and make fix if necessary for cog script 'lag_talk_looksector.cog'.
        //       It has duplicated symbol 'player' with different type

        // TODO: Make fix for cog sripts 'lag_rustycrank.cog', 'PRU_finale.cog' and remove duplicated symbol 'player'.

        // TODO: Make fix for cog script 'lag_bronzedoor1.cog' and remove duplicated symbol 'doorsector'.

        void shs_btladder_fix(CogScript& cs)
        {
            // Cog Script: shs_BTladder.cog
            // What's fixed: symbol 'int in_rotrate' is marked as non-local variable.
            // Description:
            //      The script contains int symbol 'in_rotrate' with assignment to default value '0'.
            //      The end part of assignment is not delimited with whitespace character resulting in
            //      attribute 'local' being combined with the assignment value e.g.: int in_rotrate=0local
            //      The Jones3D engine interpreters this symbol as non-local symbol and assigns value
            //      when COG is created in level. Particularly, this script is used in level '03_shs'.
            if(iequal(cs.name(), "shs_btladder.cog"))
            {
                auto it = cs.symbols.find("in_rotrate");
                if(it != cs.symbols.end() && it->type == CogSymbol::Int) {
                    it->isLocal = false;
                }
            }
        }

        static const std::map<std::string, std::function<void(CogScript&)>> kCogScriptFixes = {
            { "shs_btladder.cog", shs_btladder_fix },
        };

        decltype(kCogScriptFixes)::const_iterator get_cog_script_fix(std::string sname)
        {
            to_lower(sname);
            return kCogScriptFixes.find(sname);
        }

    }

    bool isMalformedCogScript(const libim::content::asset::CogScript& s)
    {
        using namespace  details;
        return get_cog_script_fix(s.name()) != kCogScriptFixes.end();
    }

    void fixCogScript(libim::content::asset::CogScript& s)
    {
        using namespace  details;
        auto it = get_cog_script_fix(s.name());
        if( it != kCogScriptFixes.end() ) {
            it->second(s);
        }
    }
}

#endif // IM_COGSCRIPT_FIXES_H
