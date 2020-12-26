#include "ndy.h"
#include "../world_ser_common.h"

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;
using namespace libim::utils;
using namespace std::string_view_literals;

static constexpr auto kWorldCogs = "World cogs"sv;

std::pair<std::size_t, std::vector<SharedRef<Cog>>> NDY::parseSection_Cogs(TextResourceReader& rr, const HashMap<SharedRef<CogScript>>& scripts)
{
    std::size_t maxCogs = rr.readKey<std::size_t>(kWorldCogs);
    auto resources = rr.readList<std::vector<SharedRef<Cog>>>(
    [&scripts](TextResourceReader& rr, auto rowIdx, SharedRef<Cog>& c) {
        AT_SCOPE_EXIT([&rr, reol = rr.reportEol()](){
            rr.setReportEol(reol);
        });

        auto scrname = rr.getSpaceDelimitedString();

        auto sit = scripts.find(scrname);
        if(sit == scripts.end())
        {
            LOG_ERROR("CND::ParseSection_Cogs(): Can't find cog script '%'", scrname);
            throw StreamError("Can't make Cog, CogScript not found");
        }

        c->id     = rowIdx;
        c->script = *sit;
        c->flags  = c->script->flags;
        c->vtid   = c->script->getNextVTableId();

        for(auto& s : c->script->symbols)
        {
            if(s.isLocal ||
               s.type == CogSymbol::Message) {
                continue;
            }

            std::string sval = rr.getSpaceDelimitedString(/*throwIfEmpty=*/false);
            if(sval.empty())
            {
                if(!s.hasDefaultValue())
                {
                    const auto& loc = rr.currentToken().location();
                    LOG_WARNING("NDY::ParseSection_Cogs(): Reached end of line while parsing file:'%', script:'%', symbol:'%' [LOC %:%]",
                        loc.filename, scrname, s.name, loc.last_line, loc.first_col);
                }

               break;
            }

            s.vtable[c->vtid] = std::move(sval);
            // TODO: initialize value with valid type
        }
    });

    return { maxCogs, std::move(resources) };
}


void NDY::writeSection_Cogs(TextResourceWriter& rw, std::size_t maxWorldCogs, const std::vector<SharedRef<Cog>>& cogs)
{
    std::vector<std::string> scogs;
    scogs.reserve(cogs.size());
    for(const auto& c : cogs)
    {
        std::string cogvals;
        cogvals.reserve(c->script->name().size() + c->script->symbols.size() * 64);
        for(const auto& s : c->script->symbols)
        {
            if(s.isLocal ||
               !s.vtable.contains(c->vtid)) {
                continue;
            }

            cogvalue_visitor([&](const std::string_view& s) {
                if(s.empty()) return;

                const char delim = cogvals.empty() ? '\t' : ' ';
                std::string sval(s.size() + 1, delim);
                std::copy(s.begin(), s.end(), sval.begin() + 1);
                cogvals += std::move(sval);
            }, s.vtable.at(c->vtid));
        }

        scogs.push_back(c->script->name() + cogvals);
    }

    writeResourceSection<true>(rw,
        "######### COG placement ########"sv,
        kSectionCogs,
        kWorldCogs,
        maxWorldCogs,
        scogs,
        [](const auto& v) { return v; },
        "Num\tScript          Symbol values"sv
    );
}