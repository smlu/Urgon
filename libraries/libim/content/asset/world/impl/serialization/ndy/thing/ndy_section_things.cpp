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


void writeThingNameAndBase(TextResourceWriter& rw, std::string_view name, std::string_view base)
{
    rw.write(name, /*fieldWidth=*/18);
    rw.write(base.empty() ? kNone : base, /*fieldWidth=*/18);
}


void writeThingParams(TextResourceWriter& rw, const CndThing& t, const HashMap<CndThing>& templates, bool bTemplateThing = false)
{
    OptionalRef<const CndThing> baseTemplate;
    if(templates.contains(t.baseName)) {
        baseTemplate = templates.at(t.baseName);
    }
    if(bTemplateThing && !t.pyrOrient.isZero()) {
        ndyWriteThingOrient(rw, t, baseTemplate);
    }

    ndyWriteThingType(rw, t, baseTemplate);
    ndyWriteThingCollideType(rw, t, baseTemplate);
    ndyWriteThingMoveType(rw, t, baseTemplate);
    ndyWriteThingFlags(rw, t, baseTemplate);

    ndyWriteThingLight(rw, t, baseTemplate);
    ndyWriteThingTimer(rw, t, baseTemplate);
    ndyWriteThingPerfLevel(rw, t, baseTemplate);

    ndyWriteThingRdFileName(rw, t, baseTemplate);
    ndyWriteThingSize(rw, t, baseTemplate);
    ndyWriteThingMoveSize(rw, t, baseTemplate);
    ndyWriteThingCollWidth(rw, t, baseTemplate);
    ndyWriteThingCollHeight(rw, t, baseTemplate);

    ndyWriteThingPuppet(rw, t, baseTemplate);
    ndyWriteThingSoundClass(rw, t, baseTemplate);
    ndyWriteThingCreateThing(rw, t, baseTemplate);
    ndyWriteThingCog(rw, t, baseTemplate);

    ndyWriteThingMoveInfo(rw, t, baseTemplate);
    ndyWriteThingInfo(rw, t, baseTemplate);
    ndyWriteThingControlInfo(rw, t, baseTemplate);
}

void writeTemplateParams(TextResourceWriter& rw, const CndThing& t, const HashMap<CndThing>& templates)
{
    writeThingParams(rw, t, templates, true);
}



void NDY::writeSection_Templates(TextResourceWriter& rw, const HashMap<CndThing>& templates)
{
    rw.writeLine("##### Templates information ####"sv);
    rw.writeSection(kSectionTemplates, /*overline=*/ false);
    rw.writeEol();

    rw.writeKeyValue(kWorldTemplates, templates.size());
    rw.writeEol();

    rw.writeList(templates, [&](auto& rw, auto idx, const CndThing& t) {
        if(idx == 0) {
            rw.writeCommentLine("Name:           Based On:        Params:");
        }

        writeThingNameAndBase(rw, t.name, t.baseName);
        writeTemplateParams(rw, t, templates);
    });

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
