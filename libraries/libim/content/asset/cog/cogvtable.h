#ifndef COGVTABLE_H
#define COGVTABLE_H
#include <cstdint>
#include <map>
#include <stdexcept>

#include "cogsymbol_value.h"

namespace libim::content::asset {
    class CogVTable final : protected std::map<std::size_t, CogSymbolValue>
    {
        using base_ = std::map<std::size_t, CogSymbolValue>;
        constexpr static auto to_key = [](auto id) { return static_cast<std::size_t>(id); };
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
            return base_::at(static_cast<std::size_t>(id));
        }

        const CogSymbolValue& at(const Id& id) const
        {
            return base_::at(static_cast<std::size_t>(id));
        }

        CogSymbolValue& operator[](const Id& id)
        {
            if(id == defaultId && !contains(defaultId)) {
                throw std::runtime_error("Cannot insert into CogVTable default value via operator[]");
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
    };
}
#endif // COGVTABLE_H
