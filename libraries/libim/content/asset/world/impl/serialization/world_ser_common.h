#ifndef LIBIM_WORLD_SER_COMMON_H
#define LIBIM_WORLD_SER_COMMON_H
#include <array>
#include <string_view>
#include <string>
#include <optional>
#include <variant>

#include <libim/content/asset/cog/cog.h>
#include <libim/content/asset/cog/cogsymbol_value.h>
#include <libim/types/sharedref.h>
#include <libim/types/safe_cast.h>
#include <libim/utils/utils.h>


namespace libim::content::asset {
    static constexpr std::string_view kFileCopyright = {
        "................................" \
        "................@...@...@...@..." \
        ".............@...@..@..@...@...." \
        "................@.@.@.@.@.@....." \
        "@@@@@@@@......@...........@....." \
        "@@@@@@@@....@@......@@@....@...." \
        "@@.....@.....@......@@@.....@@.." \
        "@@.@@@@@......@.....@@@......@@." \
        "@@@@@@@@.......@....@@.....@@..." \
        "@@@@@@@@.........@@@@@@@@@@....." \
        "@@@@@@@@..........@@@@@@........" \
        "@@.....@..........@@@@@........." \
        "@@.@@@@@.........@@@@@@........." \
        "@@.....@.........@@@@@@........." \
        "@@@@@@@@.........@@@@@@........." \
        "@@@@@@@@.........@@@@@@@........" \
        "@@@...@@.........@@@@@@@........" \
        "@@.@@@.@.........@.....@........" \
        "@@..@..@........@.......@......." \
        "@@@@@@@@........@.......@......." \
        "@@@@@@@@.......@........@......." \
        "@@..@@@@.......@........@......." \
        "@@@@..@@......@.........@......." \
        "@@@@.@.@......@.........@......." \
        "@@....@@........................" \
        "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" \
        "@@@@@@@@@@@@@.@@@@@@@@@@@@@@@@@@" \
        "@@.@@..@@@@@..@@@@@@@@@@.@@@@@@@" \
        "@@.@.@.@@@@.@.@@@.@..@@...@@@..@" \
        "@@..@@@@@@....@@@..@@@@@.@@@@.@@" \
        "@@@@@@@@...@@.@@@.@@@@@..@@...@@" \
        "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" \
        "@.(c).lucasarts.entertainment..@" \
        "@.........company.llc..........@" \
        "@....(c).lucasfilm.ltd.&.tm....@" \
        "@.....all.rights.reserved......@" \
        "@...used.under.authorization...@" \
        "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
    };

    inline std::optional<std::size_t> make_optional_idx(int32_t idx)
    {
        return idx > -1 ? std::make_optional(safe_cast<std::size_t>(idx)) : std::nullopt;
    }

    inline int32_t from_optional_idx(std::optional<std::size_t> idx)
    {
        return idx.has_value() ? safe_cast<int32_t>(idx.value()) : -1;
    }

    template<typename Lambda>
    void cogvalue_visitor(Lambda&& consumer, const CogSymbolValue& value)
    {
        using namespace utils;
        std::visit(overloaded {
            [](std::monostate )                           {},
            [&consumer](int32_t v)                        { consumer(utils::to_string(v));       },
            [&consumer](float v)                          { consumer(utils::to_string(v));       },
            [&consumer](const std::string& v)             { consumer(v);                         },
            [&consumer](const Vector3f& v)                { consumer(v.toString());              },
            [&consumer](const CogFlexType& v)             { if(auto fv = std::get_if<float>(&v))
                                                                consumer(utils::to_string(*fv)); },
            [](const CogMessageType& )                    {},
            [&consumer](const SharedRef<audio::Sound>& v) { consumer(v->name());                 },
            [&consumer](const SharedRef<Animation>& v)    { consumer(v->name());                 },
            [&consumer](const SharedRef<Material>& v)     { consumer(v->name());                 },
            // TODO: for all types below that have mv 'id', remove it and find it's id in content table / world
            [&consumer](const SharedRef<Surface>& v)      { consumer(utils::to_string(v->id));   },
            [&consumer](const SharedRef<Sector>& v)       { consumer(utils::to_string(v->id));   },
            [&consumer](const SharedRef<Cog>& v)          { consumer(utils::to_string(v->id));   }
        }, value);
    }

}
#endif // LIBIM_WORLD_SER_COMMON_H
