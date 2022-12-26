#ifndef LIBIM_COGVTABLE_H
#define LIBIM_COGVTABLE_H
#include <cstdint>
#include <map>
#include <stdexcept>

#include "cogsymbol_value.h"
#include <libim/utils/utils.h>

namespace libim::content::asset {
    class CogVTable final : protected std::map<std::size_t, CogSymbolValue>
    {
        using base_ = std::map<std::size_t, CogSymbolValue>;
        constexpr static auto to_key = [](auto id) { return utils::to_underlying(id); };
    public:
        enum class Id : std::size_t {};
        constexpr static CogVTable::Id defaultId = Id{0};

        void setDefault(CogSymbolValue v)
        {
            base_::operator[](to_key(defaultId)) = std::move(v);
        }

        CogSymbolValue& getDefault()
        {
            return at(defaultId);
        }

        const CogSymbolValue& getDefault() const
        {
            return at(defaultId);
        }

        bool hasDefault() const
        {
            return contains(defaultId);
        }

        CogSymbolValue& at(const Id& id)
        {
            return base_::at(to_key(id));
        }

        const CogSymbolValue& at(const Id& id) const
        {
            return base_::at(to_key(id));
        }

        CogSymbolValue& operator[](const Id& id)
        {
            if(id == defaultId && !contains(defaultId)) {
                throw std::invalid_argument("Cannot insert default value into CogVTable via operator[]");
            }
            return base_::operator[](to_key(id));
        }

        std::size_t size() const
        {
            return base_::size();
        }

        bool contains(Id id) const
        {
            return count(to_key(id)) > 0;
        }

    private:
        friend struct CogSymbol;
        base_::iterator find(Id vtid)
        {
            return base_::find(to_key(vtid));
        }

        base_::const_iterator find(Id vtid) const
        {
            return base_::find(to_key(vtid));
        }
    };
}
#endif // LIBIM_COGVTABLE_H
