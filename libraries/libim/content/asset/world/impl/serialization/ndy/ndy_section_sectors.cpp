#include "ndy.h"
#include <string_view>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;
using namespace libim::utils;
using namespace std::string_view_literals;


static constexpr auto kValDelim  = '\t';

static constexpr auto kWorldSectors    = "World sectors"sv;

static constexpr auto kSector          = "SECTOR"sv;
static constexpr auto kFlags           = "FLAGS"sv;
static constexpr auto kAmbientLight    = "AMBIENT LIGHT"sv;
static constexpr auto kExtraLight      = "EXTRA LIGHT"sv;
static constexpr auto kTint            = "TINT"sv;
static constexpr auto kAvgLightInt     = "AVERAGE LIGHT INTENSITY"sv;
static constexpr auto kAvgLightPos     = "AVERAGE LIGHT POSITION"sv;
static constexpr auto kAvgLightFalloff = "AVERAGE LIGHT FALLOFF"sv;
static constexpr auto kBoundBox        = "BOUNDBOX"sv;
static constexpr auto kCollideBox      = "COLLIDEBOX"sv;
static constexpr auto kSound           = "SOUND"sv;
static constexpr auto kThrust          = "THRUST"sv;
static constexpr auto kCenter          = "CENTER"sv;
static constexpr auto kRadius          = "RADIUS"sv;
static constexpr auto kVertices        = "VERTICES"sv;
static constexpr auto kSurfaces        = "SURFACES"sv;



std::vector<Sector> NDY::parseSection_Sectors(TextResourceReader& rr)
{
    return rr.readList<std::vector<Sector>, /*hasRowIdxs=*/false>(kWorldSectors, [](TextResourceReader& rr, auto rowIdx, Sector& s){
        s.id = rowIdx;
        [[maybe_unused]] auto sidx = rr.readKey<std::size_t>(kSector); // discard sector number
        assert(sidx == s.id && "ParseSection_Sectors: sidx == s.id");

        s.flags = rr.readKey<decltype(s.flags)>(kFlags);
        s.ambientLight = makeLinearColor(rr.readKey<LinearColorRgb>(kAmbientLight), 1.0f);
        s.extraLight   = makeLinearColor(rr.readKey<LinearColorRgb>(kExtraLight), 1.0f);

        const Token& keytkn = rr.currentToken();
        auto readNextKey = [&rr]() {
            rr.getDelimitedString([](auto c){  return c == kValDelim; });
        };

        // Optional tint
        readNextKey();
        if (keytkn.value() == kTint)
        {
            s.tint = rr.readVector<decltype(s.tint)>();
            readNextKey();
        }

        // Optional light param
        if (keytkn.value() == kAvgLightInt)
        {
            s.avgLight.color = makeLinearColor(rr.readVector<LinearColorRgb>(), 1.0f);
            readNextKey();
        }

        if (keytkn.value() == kAvgLightPos)
        {
            s.avgLight.position = rr.readVector<decltype(s.avgLight.position)>();
            readNextKey();
        }

        if (keytkn.value() == kAvgLightFalloff)
        {
            s.avgLight.falloffMin= rr.getNumber<float>();
            s.avgLight.falloffMax= rr.getNumber<float>();
            readNextKey();
        }

        // Read bound box
        if (keytkn.value() != kBoundBox)
        {
            LOG_DEBUG("NDY::ParseSection_Sectors: expected key '%', found '%'", kBoundBox, keytkn.value());
            throw SyntaxError("invalid value"sv, keytkn.location());
        }
        s.boundBox = rr.readBox<decltype(s.boundBox)>();

        // Optional collide box
        readNextKey();
        if (keytkn.value() == kCollideBox)
        {
            s.collideBox = rr.readBox<decltype(s.collideBox)>();
            readNextKey();
        }

        // Optional sound
        if (keytkn.value() == kSound)
        {
            auto sound  = rr.getSpaceDelimitedString();
            auto volume = rr.getNumber<decltype(std::declval<Sector::AmbientSound>().volume)>();
            s.ambientSound = { std::move(sound), volume };
            readNextKey();
        }

        // Optional thrust
        if (keytkn.value() == kThrust)
        {
            s.thrust= rr.readVector<decltype(s.thrust)>();
            readNextKey();
        }

        // Center & radius
        if(keytkn.value() != kCenter)
        {
            LOG_DEBUG("NDY::ParseSection_Sectors: expected '%', found '%'", kCenter, keytkn.value());
            throw SyntaxError("Invalid sector property value for center"sv, keytkn.location());
        }

        s.center = rr.readVector<decltype(s.center)>();
        s.radius = rr.readKey<decltype(s.radius)>(kRadius);

        // Vertex idxs
        s.vertIdxs = rr.readList<decltype(s.vertIdxs)>(kVertices, [](auto& rr, auto /*rowIdx*/, auto& vidx) {
            vidx = rr.template getNumber<decltype(vidx)>();
        });

        // Surfaces
        rr.assertKey(kSurfaces);
        s.surfaces.firstIdx = rr.getNumber<decltype(s.surfaces.firstIdx)>();
        s.surfaces.count    = rr.getNumber<decltype(s.surfaces.count)>();
    });
}

void NDY::writeSection_Sectors(TextResourceWriter& rw, const std::vector<Sector>& sectors)
{
    rw.writeLine("###### Sector information ######"sv);
    rw.writeSection(kSectionSectors, /*overline=*/ false);
    rw.writeEol();

    AT_SCOPE_EXIT([&rw, ich = rw.indentChar()](){
        rw.setIndentChar(ich);
    });

    rw.setIndentChar(kValDelim);
    rw.writeList(kWorldSectors, sectors, [](TextResourceWriter& rw, auto idx, const Sector& s) {
        rw.writeEol();

        rw.writeKeyValue(kSector, idx);
        rw.writeKeyValue</*precision=*/1>(kFlags, s.flags);
        rw.writeKeyValue(kAmbientLight, makeLinearColorRgb(s.ambientLight));
        rw.writeKeyValue(kExtraLight, makeLinearColorRgb(s.extraLight));

        // Tint
        if (!s.tint.isZero()) {
            rw.writeKeyValue(kTint, s.tint);
        }

        // Optional light params
        if (!s.avgLight.color.isZero()) {
            rw.writeKeyValue(kAvgLightInt, makeLinearColorRgb(s.avgLight.color));
        }

        if (!s.avgLight.position.isZero()) {
            rw.writeKeyValue(kAvgLightPos, s.avgLight.position);
        }

        if (s.avgLight.falloffMin != 0.0f || s.avgLight.falloffMax != 0.0f) {
            rw.writeKeyValue(kAvgLightFalloff, Vector2f{ s.avgLight.falloffMin, s.avgLight.falloffMax });
        }

        // Bound box and collide box
        rw.writeKeyValue(kBoundBox, s.boundBox);
        if (!s.collideBox.isZero()) {
            rw.writeKeyValue(kCollideBox, s.collideBox);
        }

        // Ambient sound
        if (s.ambientSound)
        {
            rw.write(kSound);

            rw.indent(1);
            rw.write(s.ambientSound->sound);

            rw.indent(1);
            rw.writeNumber<10, 6>(s.ambientSound->volume);
            rw.writeEol();
        }

        // Thrust
        if (!s.thrust.isZero()) {
            rw.writeKeyValue(kThrust, s.thrust);
        }

        // Center & radius
        rw.writeKeyValue(kCenter, s.center);
        rw.writeKeyValue</*precision=*/8>(kRadius, s.radius);

        // Vertex idxs
        rw.writeList</*lbAfterSize=*/false>(kVertices, s.vertIdxs,
        [](auto& rw, auto ridx, auto vidx) {
            rw.writeRowIdx(ridx, 0);
            rw.indent(1);
            rw.writeNumber(vidx);
        });

        // Surfaces
        rw.write(kSurfaces);
        rw.indent(1);

        rw.writeNumber(s.surfaces.firstIdx);
        rw.indent(1);

        rw.writeNumber(s.surfaces.count);
        rw.writeEol();
    });
}
