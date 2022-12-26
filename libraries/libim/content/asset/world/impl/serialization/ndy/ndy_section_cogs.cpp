#include "ndy.h"
#include "../world_ser_common.h"

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;
using namespace libim::utils;
using namespace std::string_view_literals;

static constexpr auto kWorldCogs = "World cogs"sv;

std::pair<std::size_t, std::vector<SharedRef<Cog>>> NDY::parseSection_Cogs(TextResourceReader& rr, const IndexMap<SharedRef<CogScript>>& scripts)
{
    std::size_t maxCogs = rr.readKey<std::size_t>(kWorldCogs);

    auto resources = rr.readList<std::vector<SharedRef<Cog>>>(
    [&scripts](TextResourceReader& rr, auto rowIdx, SharedRef<Cog>& c) {
        AT_SCOPE_EXIT([&rr, reol = rr.reportEol()](){
            rr.setReportEol(reol);
        });

        rr.setReportEol(true); // Require EOL after each row. This will make sure any unset symbols are initialized with default values

        /* Get script */
        auto scriptName = rr.getSpaceDelimitedString();
        auto sit = scripts.find(scriptName);
        if(sit == scripts.end())
        {
            LOG_ERROR("CND::ParseSection_Cogs(): Can't find cog script '%'", scriptName);
            throw StreamError("Can't make COG, CogScript not found");
        }

        /* Parse COG symbol values */
        c->setName(std::string(scriptName));
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

            std::string sval{ rr.getSpaceDelimitedString(/*throwIfEmpty=*/false) };
            if(sval.empty())
            {
                if(!s.hasDefaultValue())
                {
                    const auto& loc = rr.currentToken().location();
                    LOG_WARNING("%:%:%: Reached end of line while trying to parse value for symbol:'%' of a script:'%'",
                        loc.filename, loc.lastLine, loc.firstColumn, s.name, scriptName);
                }

               break;
            }

            s.vtable[c->vtid] = std::move(sval);
            // TODO: initialize value with valid type
        }
    });

    return { maxCogs, std::move(resources) };
}

void NDY::writeSection_Cogs(TextResourceWriter& rw, std::size_t maxCogs, const std::vector<SharedRef<Cog>>& cogs)
{
    std::vector<std::string> scogs;
    scogs.reserve(cogs.size());
    for(const auto& c : cogs)
    {
        std::string cogvals;
        cogvals.reserve(c->name().size() + c->script->symbols.size() * 64);
        for(const auto& s : c->script->symbols)
        {
            if (s.isLocal
            || s.type == CogSymbol::Message) {
                continue;
            }

            const CogSymbolValue& svalue = s.valueOrDefault(c->vtid);
            cogvalue_visitor([&](const std::string_view s) {
                if(s.empty()) return;

                const char delim = cogvals.empty() ? '\t' : ' ';
                std::string sval(s.size() + 1, delim);
                std::copy(s.begin(), s.end(), sval.begin() + 1);
                cogvals += std::move(sval);
            }, svalue);
        }

        scogs.push_back(c->name() + cogvals);
    }

    writeResourceSection<true>(rw,
        "######### COG placement ########"sv,
        kSectionCogs,
        kWorldCogs,
        maxCogs,
        scogs,
        [](const auto& v) { return v; },
        "Num\tScript          Symbol values"sv
    );
}
