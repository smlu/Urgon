#include "ndy.h"
#include "../world_ser_common.h"
#include <libim/content/text/impl/text_resource_literals.h>
#include <libim/types/safe_cast.h>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;
using namespace libim::utils;

static constexpr auto kWorldVertices   = "World vertices"sv;
static constexpr auto kWorldTexVerts   = "World texture vertices"sv;
static constexpr auto kWorldAdjoints   = "World adjoins"sv;
static constexpr auto kWorldSurfaces   = "World surfaces"sv;

Georesource NDY::parseSection_Georesource(TextResourceReader& rr)
{
    Georesource res;

    // Read world vertices
    res.vertices = rr.readList<decltype(res.vertices)>(kWorldVertices, [](auto& rr, auto /*rowIdx*/, auto& v){
        v = rr.template readVector<decltype(v)>();
    });

    // Read world tex vertices
    res.texVertices = rr.readList<decltype(res.texVertices)>(kWorldTexVerts, [](auto& rr, auto /*rowIdx*/, auto& tv){
        tv = rr.template readVector<decltype(tv)>();
    });

    // Read world adjoins
    res.adjoins = rr.readList<decltype(res.adjoins)>(kWorldAdjoints, [](auto& rr, auto /*rowIdx*/, SurfaceAdjoin& a){
        a.flags     = rr.template readFlags<decltype(a.flags)>();
        auto mirror = rr.template getNumber<int32_t>();
        a.distance  = rr.template getNumber<decltype(a.distance)>();

        a.mirrorIdx = makeOptionalIdx(mirror);
    });

    // Read world surfaces
    res.surfaces = rr.readList<decltype(res.surfaces)>(kWorldSurfaces, [](auto& rr, auto rowIdx, Surface& s){
        s.id        = rowIdx;
        auto matIdx = rr.template getNumber<int32_t>();
        s.matIdx    = makeOptionalIdx(matIdx);

        s.surflags  = rr.template readFlags<decltype(s.surflags)>();
        s.flags     = rr.template readFlags<decltype(s.flags)>();

        s.geoMode   = rr.template readFlags<decltype(s.geoMode)>();
        s.lightMode = rr.template readFlags<decltype(s.lightMode)>();
        rr.template getNumber<uint32_t>(); // unused tex mode

        auto adjoinIdx = rr.template getNumber<int32_t>();
        s.adjoinIdx    = makeOptionalIdx(adjoinIdx);
        s.extraLight   = rr.template readVector<decltype(s.extraLight)>();

        auto numVerts = static_cast<std::size_t>(
            rr.template getNumber<int32_t>()
        );

        s.vertices.resize(numVerts);
        for(auto& v : s.vertices)
        {
             v.vertIdx = rr.template getNumber<decltype(v.vertIdx)>();
             rr.assertPunctuator(",");

             auto uvIdx = rr.template getNumber<int32_t>();
             v.uvIdx    = makeOptionalIdx(uvIdx);
        }

        s.vecIntensities.resize(numVerts);
        for(auto& i : s.vecIntensities)
        {
            i = makeLinearColor(
                rr.template readVector<LinearColorRgb>(),
                s.extraLight.alpha()
            );
        }
    });

    // Read surface normals
    for(auto[idx, s] : enumerate(res.surfaces))
    {
        [[maybe_unused]] const auto ridx = rr.readRowIdx();
        assert(ridx == idx);
        s.normal = rr.readVector<decltype(s.normal)>();
    }

    return res;
}

void NDY::writeSection_Georesource(TextResourceWriter& rw, const Georesource& geores)
{
    AT_SCOPE_EXIT([&rw, ich = rw.indentChar()](){
        rw.setIndentChar(ich);
    });

    rw.setIndentChar(' ');
    rw.writeLine("#### Geometry Resources Info ####"sv);
    rw.writeSection(kSectionGeoresource, /*overline=*/ false);

    rw.writeCommentLine("----- Vertices Subsection -----"sv);
    rw.writeList(kWorldVertices, geores.vertices, [](auto& rw, auto idx, auto& v){
        if (idx == 0)
        {
            rw.setIndentChar('\t');
            rw.writeCommentLine("num:	vertex:"sv);
        }

        rw.writeRowIdx(idx, 0);
        rw.writeVector(v, /*indent=*/ 1);
    });

    rw.writeEol();
    rw.writeEol();

    rw.setIndentChar(' ');
    rw.writeCommentLine("-- Texture Verts Subsection ---"sv);
    rw.writeList(kWorldTexVerts, geores.texVertices, [](auto& rw, auto idx, auto& v){
        if (idx == 0)
        {
            rw.setIndentChar('\t');
            rw.writeCommentLine("num:	u:	v:"sv);
        }

        rw.writeRowIdx(idx, 0);
        rw.writeVector(v, /*width=*/ 1);
    });

    rw.writeEol();
    rw.writeEol();

    rw.setIndentChar(' ');
    rw.writeCommentLine("------ Adjoins Subsection -----"sv);
    rw.writeList(kWorldAdjoints, geores.adjoins, [](auto& rw, auto idx, const SurfaceAdjoin& a) {
        if (idx == 0)
        {
            rw.setIndentChar('\t');
            rw.writeCommentLine("num:	flags:	mirror:	dist:"sv);
        }

        rw.writeRowIdx(idx, 0);
        rw.indent(1);

        rw.template writeFlags<1>(a.flags);
        rw.indent(1);

        rw.writeNumber(a.mirrorIdx ? *a.mirrorIdx : 0); // Note: must not be -1
        rw.indent(1);

        rw.template writeNumber</*base=*/10, /*precision=*/8>(a.distance);
    });

    rw.writeEol();
    rw.setIndentChar(' ');

    rw.writeCommentLine("----- Surfaces Subsection -----"sv);
    rw.writeList(kWorldSurfaces, geores.surfaces, [](auto& rw, auto idx, const Surface& s){
        if(idx == 0)
        {
            rw.setIndentChar('\t');
            rw.writeCommentLine("num:	mat:	surfflags:	faceflags:	geo:	light:	tex:	adjoin:	extralight:	nverts:	vertices:			intensities:"sv);
        }

        rw.writeRowIdx(idx, 0);
        rw.indent(1);

        rw.writeNumber(fromOptionalIdx(s.matIdx));
        rw.indent(1);

        rw.template writeFlags<1>(s.surflags);
        rw.indent(1);

        rw.template writeFlags<1>(s.flags);
        rw.indent(1);

        rw.writeNumber(utils::to_underlying(s.geoMode));
        rw.indent(1);

        rw.writeNumber(utils::to_underlying(s.lightMode));
        rw.indent(1);

        rw.writeNumber(uint32_t(3)); // tex mode, unused by the engine
        rw.indent(1);

        rw.writeNumber(fromOptionalIdx(s.adjoinIdx));
        rw.indent(1);

        rw.writeVector(s.extraLight, /*width=*/1);
        rw.indent(2);

        // Surface verts, tex verts and verts color
        rw.writeNumber(safe_cast<uint32_t>(s.vertices.size()));
        rw.indent(1);
        for(const auto& v : s.vertices)
        {
            rw.writeNumber(v.vertIdx);
            rw.write(",");
            rw.writeNumber(fromOptionalIdx(v.uvIdx));
            rw.indent(1);
        }

        for(const auto& i : s.vecIntensities) {
            rw.writeVector(makeLinearColorRgb(i), /*width=*/1); // Note: The engine takes alpha from field alpha of member var extraLight
        }
    });

    rw.writeEol();
    rw.setIndentChar('\t');

    rw.writeCommentLine("--- Surface normals ---"sv);
    for(auto[idx, s] : enumerate(geores.surfaces))
    {
        rw.writeRowIdx(idx, 0);
        rw.writeVector(s.normal, /*indent=*/1);
        rw.writeEol();
    }

    rw.writeLine("################################");
    rw.writeEol();
    rw.writeEol();
}
