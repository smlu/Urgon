#include "ndy_thing_iser.h"
#include "ndy_thing_oser.h"

#include "../ndy.h"
#include "../../world_ser_common.h"
#include <libim/types/optref.h>

#include <map>
#include <string_view>
#include <type_traits>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;
using namespace libim::utils;
using namespace std::string_view_literals;

static constexpr auto kWorldTemplates = "World templates"sv;
static constexpr auto kWorldThings    = "World things"sv;

IndexMap<CndThing> NDY::parseTemplateList(text::TextResourceReader& rr)
{
    return rr.readList<IndexMap<CndThing>, /*hasRowIdxs=*/false>(
        [](TextResourceReader& rr, const auto& templates, auto rowIdx, CndThing& t) {
            t = ndyParseTemplate(rr, templates);
        },
        [](auto& map, auto&& t) {
            map.emplaceBack(t.name, std::move(t));
    });
}

IndexMap<CndThing> NDY::parseSection_Templates(text::TextResourceReader& rr)
{
    const std::size_t sizeTemplates = rr.readKey<std::size_t>(kWorldTemplates);
    auto templates = parseTemplateList(rr);
    if (templates.size() != sizeTemplates) {
        LOG_WARNING("NDY::ParseSection_Templates(): Expected % templates, but found %", sizeTemplates, templates.size());
    }
    return templates;
}

void NDY::writeSection_Templates(TextResourceWriter& rw, const IndexMap<CndThing>& templates)
{
    rw.writeLine("##### Templates information ####"sv);
    rw.writeSection(kSectionTemplates, /*overline=*/ false);
    rw.writeEol();

    rw.writeKeyValue(kWorldTemplates, templates.size());
    rw.writeEol();

    writeTemplateList(rw, templates, /*writeHeader=*/true);

    rw.writeLine("################################"sv);
    rw.writeEol();
    rw.writeEol();
}

std::vector<CndThing> NDY::parseSection_Things(text::TextResourceReader& rr, const IndexMap<CndThing>& templates)
{
    const std::size_t sizeThings = rr.readKey<std::size_t>(kWorldThings);
    auto things = rr.readList<std::vector<CndThing>, /*hasRowIdxs*/false>( // Note, row idx are read by ndyParseThing
        [&templates](TextResourceReader& rr, auto rowIdx, CndThing& t) {
            t = ndyParseThing(rr, templates);
    });

    if (sizeThings < things.size()) {
        LOG_WARNING("NDY::parseSection_Things(): Expected thing array size % is smaller than actual size %", sizeThings, things.size());
    }

    if (sizeThings > things.size()) {
        // Resize buffer with free Things to match size
        things.resize(sizeThings);
    }

    // resize array to match size
    return things;
}

void NDY::writeSection_Things(TextResourceWriter& rw, const std::vector<CndThing>& things, const IndexMap<CndThing>& templates)
{
    rw.writeLine("##### Things information ####"sv);
    rw.writeSection(kSectionThings, /*overline=*/ false);
    rw.writeEol();

    rw.writeKeyValue(kWorldThings, things.size());
    rw.writeEol();

    rw.writeList(things, [&](auto& rw, auto idx, const CndThing& t) {
        if(t.type == Thing::Free) return;
        if(idx == 0) {
            rw.writeCommentLine("num template:        name:         	X:		Y:		Z:		Pitch:		Yaw:		Roll:		Sector:");
        }
        rw.writeRowIdx(idx, 3);
        rw.indent(1);
        writeThingNameAndBase(rw, t.name, t.baseName);

        AT_SCOPE_EXIT([&rw, ich = rw.indentChar()](){
            rw.setIndentChar(ich);
        });
        rw.setIndentChar('\t');

        rw.writeVector(t.position, /*width=*/1);
        rw.writeVector(t.pyrOrient, /*width=*/1);
        rw.indent(1);

        rw.writeNumber(t.sectorNum);
        rw.indent(1);

        rw.setIndentChar(' ');
        writeThingParams(rw, t, templates);
    });

    rw.writeLine("################################"sv);
    rw.writeEol();
    rw.writeEol();
}
