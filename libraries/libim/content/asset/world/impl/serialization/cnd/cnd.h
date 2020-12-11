#ifndef LIBIM_CND_H
#define LIBIM_CND_H
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "cnderror.h"
#include "thing/cnd_thing.h"

#include <libim/common.h>
#include <libim/content/asset/animation/animation.h>
#include <libim/content/asset/cog/cog.h>
#include <libim/content/asset/cog/cogscript.h>
#include <libim/content/asset/material/material.h>
#include <libim/content/asset/world/georesource.h>
#include <libim/content/audio/impl/sbtrack.h>
#include <libim/io/stream.h>
#include <libim/math/color.h>
#include <libim/math/vector2.h>
#include <libim/math/vector4.h>
#include <libim/types/hashmap.h>

namespace libim::content::asset {
    enum class CndWorldFlag : uint32_t
    {
        StaticWorld          = 0x1, // jones3dstatic.cnd
        WorldInitialized     = 0x2,
        UpdateFogRenderState = 0x4
    };

    struct CndHeader final
    {
        uint32_t fileSize;
        FixedString<1216> copyright;
        CndResourceName   filePath;
        CndWorldFlag flags; // World stored in cnd file usually has flags set to 0x0C. The static world has also flag 0x1 (StaticWorld) set and combined with other flags results in value 0x0D.
        uint32_t version;
        float    worldGravity;
        float    ceilingSky_Z;
        float    horizonDistance;
        Vector2f horizonSkyOffset; // x,y
        Vector2f ceilingSkyOffset; // x,y
        std::array<float, 4> lodDistances; // list of distances to change level of detail
        struct {
            int32_t enabled;
            LinearColor color;
            float startDepth;
            float endDepth;
        } fog;

        uint32_t numSounds;

        uint32_t numMaterials;
        uint32_t sizeMaterials;
        uint32_t aMaterials;          // 32-bit void*
        uint32_t apMatArray;          // 32-bit pointer to pointer

        uint32_t numVertices;
        uint32_t aVerticies;          // 32-bit pointer Vector3f*

        uint32_t unknown28;
        uint32_t unknown29;

        uint32_t numTexVertices;
        uint32_t aTexVerticies;       // 32-bit pointer Vector2f*

        uint32_t unknown32;
        uint32_t unknown33;

        uint32_t numAdjoins;
        uint32_t aAdjoins;            // 32-bit pointer

        uint32_t numSurfaces;
        uint32_t aSurfaces;           // 32-bit pointer

        uint32_t numSectors;
        uint32_t aSectors;            // 32-bit pointer

        uint32_t numAiClasses;
        uint32_t sizeAiClasses;
        uint32_t aAiClasses;          // 32-bit pointer

        uint32_t numModels;
        uint32_t sizeModels;
        uint32_t aModels;             // 32-bit pointer

        uint32_t numSprites;
        uint32_t sizeSprites;
        uint32_t aSprites;            // 32-bit pointer

        uint32_t numKeyframes;
        uint32_t sizeKeyframes;
        uint32_t aKeyframes;          // 32-bit pointer

        uint32_t numPuppets;
        uint32_t sizePuppets;
        uint32_t aPuppets;            // 32-bit pointer

        uint32_t numSoundClasses;
        uint32_t sizeSoundClasses;
        uint32_t aSoundClasses;       // 32-bit pointer

        uint32_t numCogScripts;
        uint32_t sizeCogScripts;
        uint32_t aCogScripts;         // 32-bit pointer

        uint32_t numCogs;
        uint32_t sizeCogs;
        uint32_t aCogs;               // 32-bit pointer

        uint32_t numThingTemplates;
        uint32_t sizeThingTemplates;
        uint32_t aThingTemplates;     // 32-bit pointer

        uint32_t numThings;
        uint32_t sizeThings;
        uint32_t aThings;             // 32-bit pointer

        uint32_t pvsSize;
        uint32_t aPvs;                // 32-bit pointer (struct of 16 * int)
    };

    static_assert(sizeof(CndHeader) == 1568);


    struct CND final
    {
        static CndHeader readHeader(const InputStream& istream);

        /**
        * Parses sounds section.
        * Offset of istream has to be at the beginning of sound section.
        *
        * @param istream - const reference to the InputStream
        * @param track   - const reference to the SbTrack
        * @return Returns last sound file ID nonce
        */
        static uint32_t parseSection_Sounds(const InputStream& istream, audio::impl::SbTrack& track);

        static std::size_t getOffset_Materials(const InputStream& istream);
        static HashMap<Material> parseSection_Materials(const InputStream& istream, const CndHeader& header); // Reads materials section. Offset of istream hast to be at beginning of material section.
        static HashMap<Material> readMaterials(const InputStream& istream);
        static void writeSection_Materials(OutputStream& ostream, const HashMap<Material>& materials);

        static std::size_t getOffset_Georesource(const InputStream& istream, const CndHeader& header);
        static Georesource parseSection_Georesource(const InputStream& istream, const CndHeader& cndHeader);
        static Georesource readGeoresource(const InputStream& istream);
        static void writeSection_Georesource(OutputStream& ostream, const Georesource& geores);

        static std::size_t getOffset_Sectors(const InputStream& istream, const CndHeader& header);
        static std::vector<Sector> parseSection_Sectors(const InputStream& istream, const CndHeader& header);
        static std::vector<Sector> readSectors(const InputStream& istream);
        static void writeSection_Sectors(OutputStream& ostream, const std::vector<Sector>& sectors);

        static std::size_t getOffset_AiClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> parseSection_AiClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> readAiClass(const InputStream& istream);
        static void writeSection_AiClass(OutputStream& ostream, const std::vector<std::string>& aiclasses);

        static std::size_t getOffset_Models(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> parseSection_Models(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> readModels(const InputStream& istream);
        static void writeSection_Models(OutputStream& ostream, const std::vector<std::string>& models);

        static std::size_t getOffset_Sprites(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> parseSection_Sprites(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> readSprites(const InputStream& istream);
        static void WriteSection_Sprites(OutputStream& ostream, const std::vector<std::string>& sprites);

        static std::size_t getOffset_Keyframes(const InputStream& istream, const CndHeader& header);
        static HashMap<Animation> parseSection_Keyframes(const InputStream& istream, const CndHeader& header); // Reads keyframes section. Offset of istream hast to be at beginning of keyframe section.
        static HashMap<Animation> readKeyframes(const InputStream& istream);
        static void writeSection_Keyframes(OutputStream& ostream, const HashMap<Animation>& animations);

        static std::size_t getOffset_AnimClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> parseSection_AnimClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> readAnimClass(const InputStream& istream);
        static void writeSection_AnimClass(OutputStream& ostream, const std::vector<std::string>& animclasses);

        static std::size_t getOffset_SoundClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> parseSection_SoundClass(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> readSoundClass(const InputStream& istream);
        static void writeSection_SoundClass(OutputStream& ostream, const std::vector<std::string>& sndclasses);

        static std::size_t getOffset_CogScripts(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> parseSection_CogScripts(const InputStream& istream, const CndHeader& header);
        static std::vector<std::string> readCogScripts(const InputStream& istream);
        static void writeSection_CogScripts(OutputStream& ostream, const std::vector<std::string>& scripts);

        static std::size_t getOffset_Cogs(const InputStream& istream, const CndHeader& header);
        static std::vector<SharedRef<Cog>> parseSection_Cogs(const InputStream& istream, const CndHeader& header, const HashMap<SharedRef<CogScript>>& scripts);
        static std::vector<SharedRef<Cog>> readCogs(const InputStream& istream, const HashMap<SharedRef<CogScript>>& scripts);
        static void writeSection_Cogs(OutputStream& ostream, const std::vector<SharedRef<Cog>>& cogs);

        static std::size_t getOffset_Templates(const InputStream& istream, const CndHeader& header);
        static HashMap<CndThing> parseSection_Templates(const InputStream& istream, const CndHeader& header);
        static HashMap<CndThing> readTemplates(const InputStream& istream);
        static void writeSection_Templates(OutputStream& ostream, const HashMap<CndThing>& templates);

        static std::size_t getOffset_Things(const InputStream& istream, const CndHeader& header);
        static std::vector<CndThing> parseSection_Things(const InputStream& istream, const CndHeader& header);
        static std::vector<CndThing> readThings(const InputStream& istream);
        static void writeSection_Things(OutputStream& ostream, const std::vector<CndThing>& things);

        // Note: Section PVS is optional and it doesn't need to be written but performance will be degraded.
        //       Also if this section is not written, the sectors in sector section should have pvsIdx member set to -1
        static std::size_t getOffset_PVS(const InputStream& istream, const CndHeader& header);
        static ByteArray parseSection_PVS(const InputStream& istream);
        static ByteArray readPVS(const InputStream& istream);
        static void writeSection_PVS(OutputStream& ostream, const ByteArray& pvs);
    };
}
#endif // LIBIM_CND_H
