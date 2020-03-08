#ifndef LOG_COGSYMBOL_H
#define LOG_COGSYMBOL_H
#include <map>
#include <string>
#include <variant>

#include "cogvtable.h"
#include "../thing/thing.h"
#include <libim/types/typemask.h>

namespace libim::content::asset {

    struct CogSymbol
    {
        enum Type
        {
            None      =  0,
            Int       =  1,
            Flex      =  2,
            Thing     =  3,
            Template  =  4,
            Sector    =  5,
            Surface   =  6,
            Keyframe  =  7,
            Sound     =  8,
            Cog       =  9,
            Material  = 10,
            Vector    = 11,
            Model     = 12,
            Ai        = 13,
            Message   = 14
        };

        Type type;
        std::string name;
        CogVTable vtable;

        bool isLocal = false;
        int linkId   = 0;    // Note: LinkId get be accessed via cog function GetSenderId(). TODO: Verify if default should be -1.
        TypeMask<Thing::Type> mask = Thing::Free;
        std::string description;

        void setDefaultValue(CogSymbolValue v)
        {
            vtable.setDefault(std::move(v));
        }

        CogSymbolValue& defaultValue()
        {
            return vtable.getDefault();
        }

        const CogSymbolValue& defaultValue() const
        {
            return vtable.getDefault();
        }

        bool hasDefaultValue() const
        {
            return vtable.hasDefault();
        }
    };
}
#endif // LOG_COGSYMBOL_H
