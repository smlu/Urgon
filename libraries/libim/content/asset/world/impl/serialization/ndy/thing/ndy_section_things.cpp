#include "ndy_thing_wser_helpers.h"
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

void NDY::writeSection_Templates(TextResourceWriter& rw, const HashMap<CndThing>& templates)
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

void NDY::writeSection_Things(TextResourceWriter& rw, const std::vector<CndThing>& things, const HashMap<CndThing>& templates)
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

        AT_SCOPE_EXIT([&rw, ich = rw.indentCh()](){
            rw.setIndentCh(ich);
        });
        rw.setIndentCh('\t');

        rw.writeVector(t.pos, /*indent=*/1);
        rw.writeVector(t.pyrOrient, /*indent=*/1);
        rw.indent(1);

        rw.writeNumber(t.sectorNum);
        rw.indent(1);

        rw.setIndentCh(' ');
        writeThingParams(rw, t, templates);
    });

    rw.writeLine("################################"sv);
    rw.writeEol();
    rw.writeEol();
}
